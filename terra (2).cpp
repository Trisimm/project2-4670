#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <num_processes> <max_concurrent>" << endl;
        return 1;
    }

    int num_processes = atoi(argv[1]);
    int max_concurrent = atoi(argv[2]);

    // Allocate shared memory for the counter
    int* counter = static_cast<int*>(mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    *counter = 0;

    // Create child processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            cerr << "Error: fork() failed" << endl;
            return 1;
        } else if (pid == 0) {
            // Child process
            execlp("./worker", "worker", (char*)NULL);
            cerr << "Error: execlp() failed" << endl;
            return 1;
        }

        // Parent process
        if ((i + 1) % max_concurrent == 0 || i == num_processes - 1) {
            // Wait for all child processes to finish
            while (true) {
                int status;
                pid_t done_pid = wait(&status);
                if (done_pid == -1) {
                    if (errno == ECHILD) {
                        // No more child processes
                        break;
                    } else {
                        cerr << "Error: wait() failed" << endl;
                        return 1;
                    }
                }
            }
        }
    }

    // Parent process - increment the counter and print its value
    for (int i = 0; i < num_processes; i++) {
        (*counter)++;
        cout << "Counter = " << *counter << endl;
        sleep(1);
    }

    // Free the shared memory
    munmap(counter, sizeof(int));

    return 0;
}
