#include "ramfs.h"
#include <assert.h>
#include <ctype.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

Node *root = NULL;

#define NRFD 4096
Handle Handles[NRFD];

Node *find(const char *pathname) {
    return NULL;
}

// ERRORS
// EINVAL:  The final component(basename) of `pathname` is invalid.
// ENONENT: O_CREAT is not set and the named file does not exist.
stat ropen(const char *pathname, flags_t flags) { // Open and possibly create a file.

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
// SEEK_SET: The file offset is set to `offset` bytes.
// SEEK_CUR: The file offset is set to its current location plus `offset` bytes.
// SEEK_END: The file offset is set to the size of the file plus `offset` bytes.
// ERRORS
// EINVAL: `whence` is not valid.
off_t rseek(fd_t fd, off_t offset, whence_t whence) { // Reposition read/write file offset.

}

// ERRORS
// EEXIST:  `pathname` already exists (not necessarily as a directory).
// EINVAL:  The final component(basename) of the new directory's pathname is invalid.
// ENOENT:  A directory component in `pathname` does not exist.
// ENOTDIR: A component used as a directory in `pathname` is not a directory.
stat rmkdir(const char *pathname) { // Create a directory.

}

// ERRORS
// ENONENT:   A directory component in `pathname` does not exist.
// ENOTDIR:   `pathname`, or a component used as a directory in `pathname`, is not a directory.
// ENOTEMPTY: `pathname` is not empty.
// EACCESS:   Write access to the directory containing `pathname` was not allowed, or one of the directories in the path prefix of `pathname` did not allow search permission.
stat rrmdir(const char *pathname) { // Delete a directory.

}

// ERRORS
// EISDIR: `pathname` refers to a directory.
// ENOENT: A component in `pathname` does not exist or `pathname` is empty.
stat runlink(const char *pathname) { // Delete a file.

}

void init_ramfs() {

}

void close_ramfs() {

}