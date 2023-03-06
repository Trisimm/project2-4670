#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace std;

#define SHM_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <num_forks> <num_processes>" << endl;
        exit(1);
    }

    int num_forks = atoi(argv[1]);
    int num_processes = atoi(argv[2]);

    int shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        cerr << "shmget failed." << endl;
        exit(1);
    }

    int *shared_num = (int *) shmat(shmid, NULL, 0);
    *shared_num = 0;

    for (int i = 0; i < num_forks; i++) {
        pid_t pid;
        pid = fork();

        if (pid == -1) {
            cerr << "Fork failed." << endl;
            exit(1);
        }
        else if (pid == 0) {
            // Child process
            char args[] = {"./worker", NULL};
            execvp(args[0], args);
            exit(0);
        }

        if ((i + 1) % num_processes == 0) {
            // Parent process
            while (*shared_num < i) {
                *shared_num += 1;
            }
            cout << "Parent added " << i << " to shared memory." << endl;
            for (int j = 0; j < num_processes; j++) {
                wait(NULL);
            }
        }
    }

    shmdt(shared_num);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
