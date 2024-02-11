#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

#define NRFD      4096
#define MAX_NODES 65536

Node   *ROOT                = NULL;
Handle *Handles[NRFD]       = {NULL};
fd_t    available_fds[NRFD] = {0};
fd_t    fds_top             = NRFD - 1;
stat    FIND_LEVEL          = SUCCESS;

// Auxiliary functions

Node* find(const char* pathname) {
    if (*pathname == '\0') {
        FIND_LEVEL = SUCCESS;
        return NULL;
    } else if (!is_valid_path(pathname)) {
        FIND_LEVEL = EINVAL;
        return NULL;
    }
    Node *current = ROOT;
    int len = strlen(pathname);
    char *path = malloc(len + 1);
    strcpy(path, pathname);
    char *element = strtok(path, "/");
    while (element != NULL) {
        if (current->type == F) {
            free(path);
            FIND_LEVEL = ENOTDIR;
            return NULL;
        }
        int index = existed_index(current, element);
        if (index == FAILURE) {
            free(path);
            FIND_LEVEL = ENOENT;
            return NULL;
        }
        current = current->childs[index];
        element = strtok(NULL, "/");
    }
    free(path);
    if (current->type == F && pathname[len - 1] == '/') {
        FIND_LEVEL = EISFILE;
        return NULL;
    }
    FIND_LEVEL = SUCCESS;
    return current;
}

Node* find_parent(const char* pathname) {
    int start = strlen(pathname);
    while (start > 0 && pathname[--start] == '/');
    start = (start == 0) ? -1 : start;
    while (start > 0 && pathname[--start] != '/');
    char *path = malloc(start + 2);
    strncpy(path, pathname, start + 1);
    path[start + 1] = '\0';
    Node *parent = find(path);
    free(path);
    if (parent != NULL && parent->type == F) {
        FIND_LEVEL = ENOTDIR;
        return NULL;
    }
    return parent;
}

Node* create_node(Node* parent, const char* name) {
    if (!is_valid_name(name) || existed_index(parent, name) != FAILURE) {
        return NULL;
    }
    Node *node = malloc(sizeof(Node));
    node->name = malloc(strlen(name) + 1);
    strcpy(node->name, name);
    node->size = 0;
    if (parent != NULL) {
        parent->size++;
        parent->childs = realloc(parent->childs, parent->size * sizeof(Node*));
        parent->childs[parent->size - 1] = node;
    }
    return node;
}

Node* create_dir(Node* parent, const char* name) {
    Node *dir = create_node(parent, name);
    if (dir != NULL) {
        dir->type = D;
        dir->childs = NULL;
    }
    return dir;
}

Node* create_file(Node* parent, const char* name) {
    Node *file = create_node(parent, name);
    if (file != NULL) {
        file->type = F;
        file->content = NULL;
    }
    return file;
}

char* get_basename(const char* pathname) {
    int start = strlen(pathname);
    int len = 1;
    while (start > 0 && pathname[--start] == '/');
    while (start > 0 && pathname[--start] != '/' && len++);
    char *basename = malloc(len + 1);
    strncpy(basename, pathname + start + 1, len);
    basename[len] = '\0';
    return basename;
}

int existed_index(const Node* dir, const char* name) {
    if (dir != NULL) {
        for (int i = 0; i < dir->size; i++) {
            if (strcmp(dir->childs[i]->name, name) == 0) {
                return i;
            }
        }
    }
    return FAILURE;
}

bool is_valid_name(const char* name) {
    int len = strlen(name);
    if (len > 32 || len == 0) { // Length Limit
        return false;
    }
    for (int i = 0; i < len; i++) {
        if (!isalnum(name[i]) && name[i] != '.') {
            return false;
        }
    }
    return true;
}

bool is_valid_path(const char* pathname) {
    if (pathname == NULL || pathname[0] != '/') { // All `pathname` should be absolute path.
        return false;
    }
    int len = strlen(pathname);
    for (int i = 0; i < len; i++) {
        if (!isalnum(pathname[i]) && pathname[i] != '/' && pathname[i] != '.') {
            return false;
        }
    }
    return true;
}

bool fd_usable(fd_t fd) {
    return fd >= 0 && Handles[fd] != NULL;
}

bool fd_isfile(fd_t fd) {
    return Handles[fd]->f->type == F;
}

bool fd_readable(fd_t fd) {
    return !(Handles[fd]->flags & O_WRONLY);
}

bool fd_writable(fd_t fd) {
    return Handles[fd]->flags & O_WRONLY || Handles[fd]->flags & O_RDWR;
}

void pre_fd(fd_t fd) {
    if (Handles[fd]->flags & O_APPEND) {
        Handles[fd]->offset = Handles[fd]->f->size;
    }
    if ((Handles[fd]->flags & O_TRUNC) && fd_writable(fd)) {
        Handles[fd]->f->size = 0;
        free(Handles[fd]->f->content);
        Handles[fd]->f->content = NULL;
    }
    if ((Handles[fd]->flags & O_RDWR) && (Handles[fd]->flags & O_WRONLY)) {
        Handles[fd]->flags ^= O_RDWR;
    }
}

void seek_overflow(fd_t fd) {
    if (Handles[fd]->offset > Handles[fd]->f->size) {
        Handles[fd]->f->content = realloc(Handles[fd]->f->content, Handles[fd]->offset);
        memset((char*) Handles[fd]->f->content + Handles[fd]->f->size, '\0', Handles[fd]->offset - Handles[fd]->f->size);
        Handles[fd]->f->size = Handles[fd]->offset;
    }
}

void pluck_node(Node* parent, Node* node) { // The node must be FILE or empty DIR.
    int index = existed_index(parent, node->name);
    parent->size--;
    if (index != parent->size) {
        parent->childs[index] = parent->childs[parent->size];
    }
    if (parent->size == 0) {
        free(parent->childs);
        parent->childs = NULL;
    } else {
        parent->childs = realloc(parent->childs, parent->size * sizeof(Node*));
    }
    if (node->type == F) {
        free(node->content);
    }
    free(node->name);
    free(node);
}

void remove_root(Node* root) { // Remove root and all its childs thoroughly.
    Node* stack[MAX_NODES];
    int top = 0;
    stack[top++] = root;
    while (top > 0) {
        Node* current = stack[--top];
        if (current->type == D) {
            for (int i = 0; i < current->size; i++) {
                stack[top++] = current->childs[i];
            }
            free(current->childs);
        } else if (current->type == F) {
            free(current->content);
        }
        free(current->name);
        free(current);
    }
}

// API functions

// ERRORS
// EINVAL: Invalid value in flags.
// EINVAL: The final component(basename) of `pathname` is invalid.
// ENOENT: O_CREAT is not set and the named file does not exist.
// ENOENT: A directory in `pathname` does not exist.
fd_t ropen(const char* pathname, flags_t flags) { // Open and possibly create a file.
    if (flags & ~(O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_TRUNC | O_APPEND)) {
        return FAILURE;
    }
    Node *node = find(pathname);
    if (node == NULL) {
        if (pathname[strlen(pathname) - 1] == '/') {
            return FAILURE;
        } else if (flags & O_CREAT) {
            Node *parent = find_parent(pathname);
            char *name = get_basename(pathname);
            if (parent == NULL || (node = create_file(parent, name)) == NULL) { // ENOENT, ENOTDIR; EINVAL, EEXIST
                free(name);
                return FAILURE;
            }
            free(name);
        } else {
            return FAILURE;
        }
    } else if (node->type == F && pathname[strlen(pathname) - 1] == '/') {
        return FAILURE;
    }
    Handle *handle = malloc(sizeof(Handle));
    fd_t fd = available_fds[fds_top--];
    Handles[fd] = handle;
    handle->offset = 0;
    handle->f      = node;
    handle->flags  = flags;
    if (node->type == F) {
        pre_fd(fd);
    }
    return fd;
}

// ERRORS
// EBADF: `fd` isn't a valid open file descriptor.
stat rclose(fd_t fd) { // Close a file descriptor.
    if (!fd_usable(fd)) {
        return FAILURE;
    }
    free(Handles[fd]);
    Handles[fd] = NULL;
    available_fds[++fds_top] = fd;
    return SUCCESS;
}

// ERRORS
// EBADF:  `fd` is not a valid file descriptor or is not open for writing.
// EISDIR: `fd` refers to a directory.
ssize_t rwrite(fd_t fd, const void* buf, size_t count) { // Write to a file descriptor.
    if (!(fd_usable(fd) && fd_isfile(fd) && fd_writable(fd))) {
        return FAILURE;
    }
    seek_overflow(fd);
    if (Handles[fd]->f->size < Handles[fd]->offset + count) {
        Handles[fd]->f->size = Handles[fd]->offset + count;
        Handles[fd]->f->content = realloc(Handles[fd]->f->content, Handles[fd]->f->size);
    }
    memcpy(Handles[fd]->f->content + Handles[fd]->offset, buf, count);
    Handles[fd]->offset += count;
    return count;
}

// ERRORS
// EBADF:  `fd` is not a valid file descriptor or is not open for reading.
// EISDIR: `fd` refers to a directory.
ssize_t rread(fd_t fd, void* buf, size_t count) { // Read from a file descriptor.
    if (!(fd_usable(fd) && fd_isfile(fd) && fd_readable(fd))) {
        return FAILURE;
    }
    size_t count_to_read = count;
    if (Handles[fd]->offset + count > Handles[fd]->f->size) {
        count_to_read = Handles[fd]->f->size - Handles[fd]->offset;
    }
    if (count_to_read <= 0) {
        return 0;
    }
    memcpy(buf, Handles[fd]->f->content + Handles[fd]->offset, count_to_read);
    Handles[fd]->offset += count_to_read;
    return count_to_read;
}

// `whence`
// SEEK_SET (0): The file offset is set to `offset` bytes.
// SEEK_CUR (1): The file offset is set to its current location plus `offset` bytes.
// SEEK_END (2): The file offset is set to the size of the file plus `offset` bytes.
// ERRORS
// EINVAL: `whence` is not valid.
off_t rseek(fd_t fd, off_t offset, whence_t whence) { // Reposition read/write file offset.
    if (!(fd_usable(fd) && fd_isfile(fd))) {
        return FAILURE;
    }
    switch (whence) {
        case (SEEK_SET):
            if (offset < 0) {
                return FAILURE;
            }
            Handles[fd]->offset = offset;
            break;
        case (SEEK_CUR):
            if (Handles[fd]->offset + offset < 0) {
                return FAILURE;
            }
            Handles[fd]->offset += offset;
            break;
        case (SEEK_END):
            if (Handles[fd]->f->size + offset < 0) {
                return FAILURE;
            }
            Handles[fd]->offset = Handles[fd]->f->size + offset;
            break;
        default:
            return FAILURE;
    }
    return Handles[fd]->offset;
}

// ERRORS
// EEXIST:  `pathname` already exists (not necessarily as a directory).
// EINVAL:  The final component(basename) of the new directory's pathname is invalid.
// ENOENT:  A directory component in `pathname` does not exist.
// ENOTDIR: A component used as a directory in `pathname` is not a directory.
stat rmkdir(const char* pathname) { // Create a directory.
    Node *parent = find_parent(pathname);
    if (parent == NULL) {
        return FAILURE;
    }
    char *name = get_basename(pathname);
    if (!is_valid_name(name) || create_dir(parent, name) == NULL) { // ENOENT, ENOTDIR; EINVAL; EEXIST
        free(name);
        return FAILURE;
    }
    free(name);
    return SUCCESS;
}

// ERRORS
// ENOENT:    A directory component in `pathname` does not exist.
// ENOTDIR:   `pathname`, or a component used as a directory in `pathname`, is not a directory.
// ENOTEMPTY: `pathname` is not empty.
// EACCES:    Write access to the directory containing `pathname` was not allowed, or one of the directories in the path prefix of `pathname` did not allow search permission. (?)
stat rrmdir(const char* pathname) { // Delete a directory.
    Node *parent = find_parent(pathname);
    Node *dir;
    if (parent == NULL || (dir = find(pathname)) == NULL || dir->type != D || dir->size != 0) { // ENOENT, ENOTDIR, ENOTEMPTY
        return FAILURE;
    }
    pluck_node(parent, dir);
    return SUCCESS;
}

// ERRORS
// EISDIR: `pathname` refers to a directory.
// ENOENT: A component in `pathname` does not exist or `pathname` is empty.
stat runlink(const char* pathname) { // Delete a file.
    Node *parent = find_parent(pathname);
    Node *file;
    if (parent == NULL || (file = find(pathname)) == NULL || file->type == D) { // ENOENT, EISDIR
        return FAILURE;
    }
    pluck_node(parent, file);
    return SUCCESS;
}

void init_ramfs() {
    ROOT = create_dir(NULL, "ROOT");
    for (int i = NRFD - 1; i >= 0; i--) {
        available_fds[NRFD - 1 - i] = i;
    }
}

void close_ramfs() {
    for (int i = 0; i < NRFD; i++) {
        if (Handles[i] != NULL) {
            free(Handles[i]);
            Handles[i] = NULL;
        }
    }
    remove_root(ROOT);
    ROOT = NULL;
}