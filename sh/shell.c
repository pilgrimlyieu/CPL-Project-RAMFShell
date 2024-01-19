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
                PATH = realloc(PATH, (strlen(line + 17) + strlen(PATH) + 1)); // 17 = strlen("export PATH=$PATH")
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

void access_error(const char* cmd, const char* custom, const char* pathname) {
    switch (FIND_LEVEL) {
        case (ENOTDIR):
            printf("%s: %s '%s': Not a directory\n", cmd, custom, pathname);
            break;
        case (ENOENT):
            printf("%s: %s '%s': No such file or directory\n", cmd, custom, pathname);
            break;
    }
}

// STATUS
// 0: If OK.
// 1: If minor problems (e.g., cannot access subdirectory).
stat sls(const char* pathname) { // List directory contents.
    print("ls %s\n", pathname);
    Node *node = find(pathname);
    if (node == NULL) {
        access_error("ls", "cannot access", pathname);
        return PROBLEM;
    }
    else {
        if (node->type == D) {
            for (int i = 0; i < node->size; i++) {
                printf("%s ", node->childs[i]->name);
            }
            putchar('\n');
        }
        else {
            printf("%s\n", node->name);
        }
        return SUCCESS;
    }
}

stat scat(const char* pathname) { // Concatenate files and print on the standard output.
    print("cat %s\n", pathname);
    Node *node = find(pathname);
    if (node == NULL) {
        access_error("cat", "cannot access", pathname);
        return PROBLEM;
    }
    else {
        if (node->type == D) {
            printf("cat: %s: Is a directory\n", pathname);
            return PROBLEM;
        }
        else {
            for (int i = 0; i < node->size; i++) {
                putchar(((char*) node->content)[i]);
            }
            return SUCCESS;
        }
    }
}

stat smkdir(const char* pathname) { // Make directories.
    print("mkdir %s\n", pathname);
    Node *parent = find_parent(pathname);
    if (parent == NULL) {
        access_error("mkdir", "cannot create directory", pathname);
        return PROBLEM;
    }
    else {
        char *basename = get_basename(pathname);
        if (create_dir(parent, basename) == NULL) {
            printf("mkdir: cannot create directory '%s': File exists\n", pathname);
            return PROBLEM;
        }
        else {
            return SUCCESS;
        }
    }
}

stat stouch(const char* pathname) { // Change file timestamps. If file doesn't exist, create it.
    print("touch %s\n", pathname);

}

stat secho(const char* content) { // Equivalent to `echo <content>`. No need to support escape sequences. Have to support environment variables.
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
    free(PATH);
}