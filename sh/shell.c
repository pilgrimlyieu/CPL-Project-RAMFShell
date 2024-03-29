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

char *PATH = NULL;

void read_path(void) {
    Node* rc = find("/home/ubuntu/.bashrc");
    if (rc == NULL) {
        return;
    }
    char *run_commands = malloc(rc->size + 1);
    memcpy(run_commands, rc->content, rc->size);
    run_commands[rc->size] = '\0';
    char *line = strtok(run_commands, "\n");
    while (line != NULL) {
        if (strncmp(line, "export PATH=", 12) == 0) { // 12 = strlen("export PATH=")
            char *path = strstr(line, "$PATH");
            if (path == NULL) {
                free(PATH);
                PATH = malloc(strlen(line + 12) + 1);
                strcpy(PATH, line + 12);
            } else {
                PATH = realloc(PATH, strlen(line + 12) + strlen(PATH) - 4); // `PATH` is promised initialized, so don't worry about `strlen(NULL)`.
                *path = '\0';
                char *temp = malloc(strlen(PATH) + 1);
                strcpy(temp, PATH);
                strcpy(PATH, line + 12);
                strcat(PATH, temp);
                strcat(PATH, path + 5);
                free(temp);
            }
        }
        line = strtok(NULL, "\n");
    }
    free(run_commands);
}

void print_error(const char* cmd, const char* custom, const char* pathname) {
    static char *string1 = "No such file or directory";
    static char *string2 = "Not a directory";
    static char *string3 = "File exists";
    printf("%s: %s '%s': %s\n", cmd, custom, pathname, (FIND_LEVEL == ENOENT) ? string1 : (FIND_LEVEL == ENOTDIR || FIND_LEVEL == EISFILE) ? string2 : string3);
}

// STATUS
// 0: If OK.
// 1: If minor problems (e.g., cannot access subdirectory).
stat sls(const char* pathname) { // List directory contents.
    print("ls %s\n", pathname);
    if (*pathname == '\0') {
        for (int i = 0; i < ROOT->size; i++) {
            printf("%s ", ROOT->childs[i]->name);
        }
        putchar('\n');
        return SUCCESS;
    }
    Node *node = find(pathname);
    if (node == NULL) {
        print_error("ls", "cannot access", pathname);
        return PROBLEM;
    } else {
        if (node->type == D) {
            for (int i = 0; i < node->size; i++) {
                printf("%s ", node->childs[i]->name);
            }
            putchar('\n');
        } else {
            puts(pathname);
        }
        return SUCCESS;
    }
}

stat scat(const char* pathname) { // Concatenate files and print on the standard output.
    print("cat %s\n", pathname);
    if (!is_valid_path(pathname)) {
        printf("cat: %s: Invalid argument\n", pathname);
        return PROBLEM;
    }
    Node *node = find(pathname);
    if (node == NULL) {
        switch (FIND_LEVEL) {
            case (EISFILE):
            case (ENOTDIR):
                printf("cat: %s: Not a directory\n", pathname);
                break;
            case (ENOENT):
                printf("cat: %s: No such file or directory\n", pathname);
                break;
        }
        return PROBLEM;
    } else if (node->type == D) {
        printf("cat: %s: Is a directory\n", pathname);
        return PROBLEM;
    } else {
        fwrite(node->content, sizeof(char), node->size, stdout);
        putchar('\n');
        return SUCCESS;
    }
}

stat smkdir(const char* pathname) { // Make directories.
    print("mkdir %s\n", pathname);
    if (!is_valid_path(pathname)) {
        printf("mkdir: cannot create directory '%s': Invalid argument\n", pathname);
        return PROBLEM;
    }
    Node *parent = find_parent(pathname);
    if (parent == NULL) {
        print_error("mkdir", "cannot create directory", pathname);
        return PROBLEM;
    } else {
        char *basename = get_basename(pathname);
        if (create_dir(parent, basename) == NULL) {
            printf("mkdir: cannot create directory '%s': File exists\n", pathname);
            free(basename);
            return PROBLEM;
        } else {
            free(basename);
            return SUCCESS;
        }
    }
}

stat stouch(const char* pathname) { // Change file timestamps. If file doesn't exist, create it.
    print("touch %s\n", pathname);
    if (!is_valid_path(pathname)) {
        printf("touch: cannot touch '%s': Invalid argument\n", pathname);
        return PROBLEM;
    }
    Node *parent = find_parent(pathname);
    if (parent == NULL && FIND_LEVEL != SUCCESS) {
        print_error("touch", "cannot touch", pathname);
        return PROBLEM;
    } else {
        Node *node = find(pathname);
        if (node == NULL && FIND_LEVEL == EISFILE) {
            printf("touch: cannot touch '%s': Not a directory\n", pathname);
            return PROBLEM;
        } else if (node == NULL && pathname[strlen(pathname) - 1] == '/') {
            printf("touch: cannot touch '%s': Not such file or directory\n", pathname);
            return PROBLEM;
        }
        char *basename = get_basename(pathname);
        create_file(parent, basename);
        free(basename);
        return SUCCESS;
    }
}

stat secho(const char* content) { // Equivalent to `echo <content>`. No need to support escape sequences. Have to support environment variables.
    print("echo %s\n", content);
    char *output = malloc(strlen(content) + 1);
    bool escape = false;
    int j = 0;
    for (int i = 0; content[i] != '\0'; ) {
        if (escape) {
            output[j++] = content[i++];
            escape = false;
        } else if (strncmp(content + i, "$PATH", 5) == 0) {
            int len = strlen(PATH);
            output = realloc(output, j + len + 1);
            memcpy(output + j, PATH, len);
            output[j + len] = '\0';
            i += 5;
            j += len;
        } else if (content[i] == '\\') {
            escape = true;
            i++;
        } else {
            output[j++] = content[i++];
        }
    }
    output[j] = '\0';
    puts(output);
    free(output);
    return SUCCESS;
}

// STATUS
// 0: If the specified command is found.
// 1: If the specified command is nonexistent.
stat swhich(const char* cmd) { // Locate a command.
    print("which %s\n", cmd);
    if (PATH == NULL) {
        return PROBLEM;
    }
    int start = -1;
    int end = -1;
    int len = strlen(PATH);
    while (1) { // `strtok` doesn't work in "/bin:/usr/bin:/usr/" case. To be fixed.
        start = end;
        while (start < len && PATH[++start] == ':');
        end = start;
        while (end < len && PATH[++end] != ':');
        if (start == len) {
            break;
        }
        char *element = malloc(end - start + 1);
        strncpy(element, PATH + start, end - start);
        element[end - start] = '\0';
        Node *dir = find(element);
        int index = existed_index(dir, cmd);
        if (index != FAILURE && dir->childs[index]->type == F) {
            printf("%s/%s\n", element, cmd);
            free(element);
            return SUCCESS;
        }
        free(element);
    }
    return PROBLEM;
}

void init_shell() {
    read_path();
}

void close_shell() {
    free(PATH);
    PATH = NULL;
}