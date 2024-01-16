#include "ramfs.h"
#include "shell.h"
#ifndef ONLINE_JUDGE
    #define print(...) printf("\033[31m");printf(__VA_ARGS__);printf("\033[0m");
#else
    #define print(...) 
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

}

void close_shell() {

}