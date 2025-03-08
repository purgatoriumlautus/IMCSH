# IMCSH: A Custom Linux Shell Implementation

## Authors

Alisa Beztsinna & Arsenii Kobelianskyi

## Introduction

IMCSH (IMC Shell) is a custom Linux shell written in C. The shell provides basic functionality such as command execution, background processes, output redirection, and internal commands. It operates independently of standard Linux shells and interacts with users through a `user@host>` prompt.

### Key Features

- **Command Execution (`exec <command>`)**: Runs programs with arguments.
- **Background Execution (`&`)**: Executes commands without waiting for completion.
- **Output Redirection (`>` )**: Redirects command output to a file.
- **Internal Commands:**
  - `globalusage`: Displays shell version and author details.
  - `quit`: Exits the shell, optionally terminating running processes.

## Code Structure

The shell consists of several key components:

### 1. Process Management

- Utilizes a **linked list** to track background processes.
- Functions:
  - `add_process(pid_t id)`: Adds a process to the list.
  - `remove_process(pid_t id)`: Removes a process from the list when it exits.
  - `finish_child(int sig)`: Handles `SIGCHLD` signals to detect terminated processes and update the list.

### 2. Command Execution

- The `exec` function:
  - Tokenizes the user input.
  - Uses `fork()` to create a child process.
  - Executes commands via `execvp()`.
  - Handles output redirection with `dup2()`.

### 3. Prompt and Input Handling

- The `prompt()` function displays `user@host>` format using `getpwuid()` and `gethostname()`.
- The main loop:
  - Reads user input.
  - Parses and dispatches commands to `exec`, `globalusage`, or `quit`.

### 4. Internal Commands

- `globalusage`:
  - Writes version information to stdout or a file.
- `quit`:
  - Lists running processes and prompts user confirmation before termination using `SIGKILL`.

## How Modules Interact

1. The main loop reads user input and determines the command type.
2. For `exec` commands:
   - If executed in the background, the process is added to the linked list.
   - If output redirection is used, the output is written to the specified file.
3. The `SIGCHLD` signal handler updates the process list upon termination.
4. The `quit` function checks for running processes and exits the shell safely.

## Compilation & Execution

### Build the Shell

Run the following command to compile the shell:

```sh
make
```

This generates an executable named `imcsh`.

### Run the Shell

Execute the shell using:

```sh
./imcsh
```

## Usage Examples

```sh
exec ls -l > output.txt   # Writes directory listing to output.txt
exec sleep 10 &           # Runs sleep in the background
globalusage > version.txt # Saves version info to version.txt
quit                      # Exits the shell
```

## Summary

IMCSH provides core shell functionalities while focusing on simplicity and educational value. The project helped deepen our understanding of process management, Unix system calls, and C programming.

