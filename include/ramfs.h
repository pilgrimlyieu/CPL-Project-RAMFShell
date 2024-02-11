#pragma once
#include <stdint.h>
#include <stdbool.h>

// Flags
#define O_APPEND 02000 // Append:   offset->EOF(default 0); Readable
#define O_CREAT  00100 // Create:   If Exist, open. Otherwise create.
#define O_TRUNC  01000 // Truncate: If Exist & Writable, clear.
#define O_RDONLY 00000 // Read only
#define O_WRONLY 00001 // Write only
#define O_RDWR   00002 // Read & Write
// NOFLAGS: O_RDONLY
// O_TRUNC  | O_RDONLY (01000) -> O_RDONLY
// O_RDONLY | O_WRONLY (00001) -> O_WRONLY
// O_RDWR   | O_WRONLY (00003) -> O_WRONLY

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define SUCCESS  0
#define PROBLEM  1
#define FAILURE -1
#define ENOENT  -2
#define ENOTDIR -3
#define EINVAL  -4
#define EISFILE -5

typedef intptr_t  ssize_t;
typedef uintptr_t size_t;
typedef long      off_t;
typedef int       fd_t;
typedef int       flags_t;
typedef int       whence_t;
typedef char      stat;

typedef struct Node {
    enum {F, D} type;
    char *name;
    int size; // FILE: content size; DIR: child count
    // FILE
    void *content;
    // DIR
    struct Node **childs;
} Node;

typedef struct Handle {
    off_t offset;
    Node *f;
    flags_t flags;
} Handle;

// Auxiliary functions
Node* find          (const char* pathname);
Node* find_parent   (const char* pathname);
Node* create_node   (Node* parent, const char* name);
Node* create_dir    (Node* parent, const char* name);
Node* create_file   (Node* parent, const char* name);
char* get_basename  (const char* pathname);
int   existed_index (const Node* dir, const char* name);
bool  is_valid_name (const char* name);
bool  is_valid_path (const char* pathname);
bool  fd_usable     (fd_t fd);
bool  fd_isfile     (fd_t fd);
bool  fd_readable   (fd_t fd);
bool  fd_writable   (fd_t fd);
void  pre_fd        (fd_t fd);
void  seek_overflow (fd_t fd);
void  pluck_node    (Node* parent, Node* node);
void  remove_root   (Node* root);

// API functions
fd_t    ropen       (const char* pathname, flags_t flags);
stat    rclose      (fd_t fd);
ssize_t rwrite      (fd_t fd, const void* buf, size_t count);
ssize_t rread       (fd_t fd, void* buf, size_t count);
off_t   rseek       (fd_t fd, off_t offset, whence_t whence);
stat    rmkdir      (const char* pathname);
stat    rrmdir      (const char* pathname);
stat    runlink     (const char* pathname);
void    init_ramfs  ();
void    close_ramfs ();

extern stat FIND_LEVEL;
extern Node *ROOT;