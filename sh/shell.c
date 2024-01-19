#include "ramfs.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef ONLINE_JUDGE
    #define print(...) printf("\033[31m");printf(__VA_ARGS__);printf("\033[0m");
#else
    #define print(...) 
#endif

char *PATH;

void read_path(void) {
    Node* rc = find("/home/ubuntu/.bashrc");
    char *run_commands = malloc(rc->size);
    strcpy(run_commands, rc->content);
    char *line = strtok(run_commands, "\n");
    while (line != NULL) {
        if (strncmp(line, "export PATH=", 12) == 0) { // 12 = strlen("export PATH=")
            char *path = strstr(line, "$PATH");
            if (path == NULL) {
                PATH = realloc(PATH, (strlen(line + 12) + 1));
                strcpy(PATH, line + 12);
            }
            else if (path == line + 12) { // "export PATH=$PATH:..."
                PATH = realloc(PATH, (strlen(line + 17) + strlen(PATH) + 1)); // 17 = strlen("export PATH=$PATH:")
                strcat(PATH, line + 17);
            }
            else { // "export PATH=...:$PATH". I guess `$PATH` will not appear in the middle of the line, LOL.
                *path = '\0';
                PATH = realloc(PATH, (strlen(line + 12) + strlen(PATH) + 1)); // `PATH` is promised initialized, so don't worry about `strlen(NULL)`.
                char *temp = malloc(strlen(PATH) + 1);
                strcpy(temp, PATH);
                strcpy(PATH, line + 12);
                strcat(PATH, temp);
                free(temp);
            }
        }
        line = strtok(NULL, "\n");
    }
    free(run_commands);
}

// STATUS
// 0: If OK.
// 1: If minor problems (e.g., cannot access subdirectory).
stat sls(const char* pathname) { // List directory contents.
    print("ls %s\n", pathname);
}

stat scat(const char* pathname) { // Concatenate files and print on the standard output.
    print("cat %s\n", pathname);

}

stat smkdir(const char* pathname) { // Make directories.
    print("mkdir %s\n", pathname);

}

stat stouch(const char* pathname) { // Change file timestamps. If file doesn't exist, create it.
    print("touch %s\n", pathname);

}

stat secho(const char* content) { // Equivalent to `echo "<content>"`. No need to support escape sequences. Have to support environment variables.
    print("echo %s\n", content);

}

// STATUS
// 0: If the specified command is found.
// 1: If the specified command is nonexistent.
stat swhich(const char* cmd) { // Locate a command.
    print("which %s\n", cmd);

}

void init_shell() {
    PATH = NULL;
    read_path();
}

void close_shell() {

}