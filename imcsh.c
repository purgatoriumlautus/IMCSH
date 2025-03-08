#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>

// Structure to represent processes
typedef struct Process {
    pid_t id;                 // Process ID
    struct Process* next;     // Pointer to the next process in the list
} Process;

Process* all_processes = NULL;  // Head of the process linked list

// Function to add a process to the process list
void add_process(pid_t id) {
    Process* new = malloc(sizeof(Process));  // Allocate memory for new process
    new->id = id;                            // Set process ID
    new->next = all_processes;               // Add to the front of the list
    all_processes = new;                     // Update the head of the list
}

// Function to remove a process from the process list by its pid
void remove_process(pid_t id) {
    Process** current = &all_processes;  // Pointer to pointer of the head node
    while (*current) {
        if ((*current)->id == id) {     // If the process is found
            Process* release = *current; // Store the node to free
            *current = (*current)->next; // Update the pointer to the next process
            free(release);               // Free the node memory
            break;
        }
        current = &((*current)->next);  // Move to the next process
    }
}

// Function to print the shell prompt (username@hostname>)
void prompt() {
    char hostname[256];
    struct passwd* pw = getpwuid(getuid());     // Get current user info
    char* username = pw ? pw->pw_name : "unknown";  // Get username or use "unknown" if unavailable

    gethostname(hostname, sizeof(hostname));   // Get the hostname of the system
    printf("%s@%s> ", username, hostname);     // Print the prompt
    fflush(stdout);                            // Ensure prompt is displayed immediately
}

// Signal handler for SIGCHLD to handle child processes that have finished
void finish_child(int sig) {
    (void)sig;  // Unused parameter

    pid_t id;
    // Wait for any child process that has finished, do not block
    while ((id = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("\n[Process %d finished]\n", id);  // Notify the process finished
        remove_process(id);                      // Remove the process from the process list
        prompt();                                // Print the prompt after the process has finished
    }
}

// Function to execute a command with optional background execution and output redirection
void exec(char* command, int running, char* file) {
    char* args[128];
    char* token = strtok(command, " ");
    int i = 0;

    // Tokenize the command string into individual arguments
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Null-terminate the argument list

    pid_t id = fork();  // Create a child process

    if (id == 0) {  // Child process
        if (file) {  // If there's an output file, redirect output
            int f = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (f < 0) {
                perror("open");  // Handle error if file cannot be opened
                exit(1);
            }
            dup2(f, STDOUT_FILENO);  // Redirect stdout to the file
            close(f);               // Close the file descriptor
        }
        execvp(args[0], args);  // Execute the command using execvp
        perror("execvp");       // If execvp fails, print an error message
        exit(1);
    } else if (id > 0) {  // Parent process
        if (running) {    // If the process is running in the background
            add_process(id);  // Add it to the process list
            printf("[Process %d started in background]\n", id);  // Notify the user
        } else {
            waitpid(id, NULL, 0);  // Wait for the process to finish in the foreground
            printf("[Process %d finished]\n", id);  // Notify the user when it finishes
        }
    } else {
        perror("fork");  // Handle error if fork fails
    }
}

// Function to quit the shell, with an option to kill running processes
int quit() {
    if (all_processes) {  // If there are processes running
        printf("The following processes are running, are you sure you want to quit? [Y/n]\n");
        Process* current = all_processes;
        while (current) {  // List all running processes
            printf("Process %d\n", current->id);
            current = current->next;
        }

        char choice;
        if (scanf(" %c", &choice) != 1) {  // Read user input
            choice = 'n';  // Default to 'n' if input fails
        }

        if (choice == 'Y' || choice == 'y') {  // If user confirms
            current = all_processes;
            while (current) {
                kill(current->id, SIGKILL);  // Kill each process
                current = current->next;
            }
            exit(0);  // Exit the shell
        } else if (choice == 'n' || choice == 'N') {
            return 1;  // Return to the shell without quitting
        }

    } 
        exit(0);  // Exit if no processes are running

}

// Main function to run the shell loop
int main() {
    signal(SIGCHLD, finish_child);  // Set up signal handler for child processes
    char input[1024];               // Buffer to store user input

    while (1) {  // Infinite loop to keep the shell running
        prompt();  // Print the shell prompt
        if (!fgets(input, sizeof(input), stdin)) {  // Read user input
            break;  // Exit if input is EOF
        }

        input[strcspn(input, "\n")] = 0;  // Remove the newline character

        // Check for "exec" command and handle execution
        if (strncmp(input, "exec ", 5) == 0) {
            char* command = input + 5;
            int running = 0;
            char* output_file = NULL;

            if (strchr(command, '&')) {  // If there's a background operator '&'
                running = 1;
                *strchr(command, '&') = '\0';  // Remove the '&' from the command
            }
            if (strchr(command, '>')) {  // If there's output redirection
                output_file = strchr(command, '>') + 1;
                *strchr(command, '>') = '\0';  // Remove the '>' from the command
                while (*output_file == ' ') output_file++;  // Skip spaces after '>'
            }
            exec(command, running, output_file);  // Execute the command

        // Handle "globalusage" command
        } else if (strncmp(input, "globalusage", 11) == 0) {
            char* output_file = strchr(input, '>');  // Check for redirection operator
            if (output_file) {  // If there's a redirection
                output_file++;  // Skip the '>'
                while (*output_file == ' ') output_file++;  // Skip spaces after '>'
                FILE* file = fopen(output_file, "w");  // Open the file for writing
                if (file) {
                    fprintf(file, "IMCSH Version 1.1 created by Alisa Beztsinna and Arsenii Kobelianskyi\n");
                    fclose(file);  // Close the file after writing
                }
            } else {
                printf("IMCSH Version 1.1 created by Alisa Beztsinna and Arsenii Kobelianskyi\n");  // If no redirection, print to stdout
            }
        // Handle "quit" command
        } else if (strcmp(input, "quit") == 0) {
            if (quit() == 1) {
                continue;  // Skip processing if quit was canceled
            }
        } else if (input[0] != '\0') {  // Check if input is not empty
            printf("Unknown command: %s\n", input);  // Inform the user if the command is not recognized
        }
    }

    return 0;  // Exit the program (should never reach here unless the loop ends)
}
