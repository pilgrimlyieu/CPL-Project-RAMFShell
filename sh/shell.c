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
                if (PATH != NULL) {
                    free(PATH);
                }
                PATH = malloc(strlen(line + 12) + 1);
                strcpy(PATH, line + 12);
            }
            else {
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

void access_error(const char* cmd, const char* custom, const char* pathname) {
    static char *string1 = "No such file or directory";
    static char *string2 = "Not a directory";
    static char *string3 = "File exists";
    printf("%s: %s '%s': %s\n", cmd, custom, pathname, (FIND_LEVEL == ENOENT) ? string1 : (FIND_LEVEL == ENOTDIR) ? string2 : string3);
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
            for (int i = 0; i < node->nchilds; i++) {
                printf("%s", node->childs[i]->name);
                if (i != node->nchilds - 1) {
                    putchar(' ');
                }
            }
            putchar('\n');
        }
        else {
            puts(node->name);
        }
        return SUCCESS;
    }
}

stat scat(const char* pathname) { // Concatenate files and print on the standard output.
    print("cat %s\n", pathname);
    Node *node = find(pathname);
    if (node == NULL) {
        switch (FIND_LEVEL) {
            case (ENOTDIR):
                printf("cat: %s: Not a directory\n", pathname);
                break;
            case (ENOENT):
                printf("cat: %s: No such file or directory\n", pathname);
                break;
        }
        return PROBLEM;
    }
    else if (node->type == D) {
        printf("cat: %s: Is a directory\n", pathname);
        return PROBLEM;
    }
    else {
        fwrite(node->content, sizeof(char), node->size, stdout);
        putchar('\n');
        return SUCCESS;
    }
}

stat smkdir(const char* pathname) { // Make directories.
    print("mkdir %s\n", pathname);
    Node *parent = find_parent(pathname);
    if (parent == NULL) {
        access_error("mkdir", "cannot create directory", pathname);
        return PROBLEM;
    }
    else if (parent->type == F) {
        printf("mkdir: cannot create directory '%s': Not a directory\n", pathname);
        return PROBLEM;
    }
    else {
        char *basename = get_basename(pathname);
        if (create_dir(parent, basename) == NULL) {
            printf("mkdir: cannot create directory '%s': File exists\n", pathname);
            free(basename);
            return PROBLEM;
        }
        else {
            free(basename);
            return SUCCESS;
        }
    }
}

stat stouch(const char* pathname) { // Change file timestamps. If file doesn't exist, create it.
    print("touch %s\n", pathname);
    Node *parent = find_parent(pathname);
    if (parent == NULL && FIND_LEVEL) {
        access_error("touch", "cannot touch", pathname);
        return PROBLEM;
    }
    else {
        char *basename = get_basename(pathname);
        create_file(parent, basename);
        free(basename);
        return SUCCESS;
    }
}

stat secho(const char* content) { // Equivalent to `echo <content>`. No need to support escape sequences. Have to support environment variables.
    print("echo %s\n", content);
    char *output = calloc(strlen(content) + 1, sizeof(char));
    bool escape = false;
    int j = 0;
    for (int i = 0; content[i] != '\0'; ) {
        if (escape) {
            output[j++] = content[i++];
            escape = false;
        }
        else if (strncmp(content + i, "$PATH", 5) == 0) {
            output = realloc(output, j + strlen(PATH) + 1);
            memcpy(output + j, PATH, strlen(PATH));
            output[j + strlen(PATH)] = '\0';
            i += 5;
            j += strlen(PATH);
        }
        else if (content[i] == '\\') {
            escape = true;
            i++;
        }
        else {
            output[j++] = content[i++];
        }
    }
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
    char *start = PATH;
    char *end;
    while ((end = strchr(start, ':')) != NULL) {
        char *element = malloc(end - start + 1);
        strncpy(element, start, end - start);
        element[end - start] = '\0';
        Node *dir = find(element);
        int index = existed_index(dir, cmd);
        if (index != FAILURE && dir->childs[index]->type == F) {
            int path_name = strlen(element);
            printf((element[path_name - 1] == '/') ? "%s%s\n" : "%s/%s\n", element, cmd);
            free(element);
            return SUCCESS;
        }
        free(element);
        start = end + 1;
    }
    return PROBLEM;
}

void init_shell() {
    PATH = NULL;
    read_path();
}

void close_shell() {
    if (PATH != NULL) {
        free(PATH);
    }
}