#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

Node *root = NULL;

#define NRFD 4096
Handle Handles[NRFD];

Node* find(const char* pathname) {
    Node *current = root;
    char *path = malloc(strlen(pathname) + 1);
    strcpy(path, pathname);
    char *element = strtok(path, "/");
    while (element != NULL) {
        int index = existed_index(current, element);
        if (index == FAILURE) {
            free(path);
            return NULL;
        }
        current = current->childs[index];
        element = strtok(NULL, "/");
    }
    free(path);
    return current;
}

Node* find_parent(const char* pathname) {
    Node *current = root;
    Node *parent = NULL;
    char *path = malloc(strlen(pathname) + 1);
    strcpy(path, pathname);
    char *element = strtok(path, "/");
    while (element != NULL) {
        parent = current;
        int index = existed_index(current, element);
        if (index == FAILURE) {
            break;
        }
        current = current->childs[index];
        element = strtok(NULL, "/");
    }
    free(path);
    return parent;
}

char* get_basename(const char* pathname) {
    char *start = (char*)pathname + strlen(pathname) - 1;
    int len = 0;
    while (start >= pathname && *start == '/') {
        start--;
    }
    while (start >= pathname && *start != '/') {
        len++;
        start--;
    }
    char *basename = malloc(len + 1);
    strncpy(basename, start + 1, len);
    basename[len] = '\0';
    return basename;
}

bool is_valid_name(const char* name) {
    size_t len = strlen(name);
    for (int i = 0; i < len; i++) {
        if (!isalnum(name[i]) && name[i] != '.') {
            return false;
        }
    }
    return true;
}

bool is_valid_path(const char* pathname) {
    const char *valid_char = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    size_t valid = strcspn(pathname, valid_char);
    return valid == strlen(pathname);
}

int existed_index(const Node *dir, const char* name) {
    if (dir->type == D) {
        for (int i = 0; i < dir->nchilds; i++) {
            if (strcmp(dir->childs[i]->name, name) == 0) {
                return i;
            }
        }
    }
    return FAILURE;
}

stat create_dir(Node* parent, const char* name) {
    if (existed_index(parent, name) != FAILURE) {
        return FAILURE;
    }
    Node *dir = malloc(sizeof(Node));
    dir->type = D;
    dir->name = malloc(strlen(name) + 1);
    strcpy(dir->name, name);
    dir->nchilds = 0;
    dir->childs = NULL;
    parent->nchilds++;
    parent->childs = realloc(parent->childs, parent->nchilds * sizeof(Node *));
    parent->childs[parent->nchilds - 1] = dir;
    return SUCCESS;
}


// ERRORS
// EINVAL:  The final component(basename) of `pathname` is invalid.
// ENONENT: O_CREAT is not set and the named file does not exist.
stat ropen(const char* pathname, flags_t flags) { // Open and possibly create a file.

}

// ERRORS
// EBADF: `fd` isn't a valid open file descriptor.
stat rclose(fd_t fd) { // Close a file descriptor.

}

// ERRORS
// EBADF:  `fd` is not a valid file descriptor or is not open for writing.
// EISDIR: `fd` refers to a directory.
ssize_t rwrite(fd_t fd, const void *buf, size_t count) { // Write to a file descriptor.

}

// ERRORS
// EBADF:  `fd` is not a valid file descriptor or is not open for reading.
// EISDIR: `fd` refers to a directory.
ssize_t rread(fd_t fd, void *buf, size_t count) { // Read from a file descriptor.

}

// `whence`
// SEEK_SET (0): The file offset is set to `offset` bytes.
// SEEK_CUR (1): The file offset is set to its current location plus `offset` bytes.
// SEEK_END (2): The file offset is set to the size of the file plus `offset` bytes.
// ERRORS
// EINVAL: `whence` is not valid.
off_t rseek(fd_t fd, off_t offset, whence_t whence) { // Reposition read/write file offset.

}

// ERRORS
// EEXIST:  `pathname` already exists (not necessarily as a directory).
// EINVAL:  The final component(basename) of the new directory's pathname is invalid.
// ENOENT:  A directory component in `pathname` does not exist.
// ENOTDIR: A component used as a directory in `pathname` is not a directory.
stat rmkdir(const char* pathname) { // Create a directory.
    Node *parent = find_parent(pathname);
    char *name = get_basename(pathname);
    if (parent == NULL || !is_valid_name(name) || create_dir(parent, name) != SUCCESS) { // ENOENT, ENOTDIR; EINVAL; EEXIST
        return FAILURE;
    }
    return SUCCESS;
}

// ERRORS
// ENONENT:   A directory component in `pathname` does not exist.
// ENOTDIR:   `pathname`, or a component used as a directory in `pathname`, is not a directory.
// ENOTEMPTY: `pathname` is not empty.
// EACCESS:   Write access to the directory containing `pathname` was not allowed, or one of the directories in the path prefix of `pathname` did not allow search permission.
stat rrmdir(const char* pathname) { // Delete a directory.

}

// ERRORS
// EISDIR: `pathname` refers to a directory.
// ENOENT: A component in `pathname` does not exist or `pathname` is empty.
stat runlink(const char* pathname) { // Delete a file.

}

void init_ramfs() {
    root = malloc(sizeof(Node));
    root->type = D;
    root->name = malloc(2);
    strcpy(root->name, "/");
    root->nchilds = 0;
    root->childs = NULL;
}

void close_ramfs() {

}