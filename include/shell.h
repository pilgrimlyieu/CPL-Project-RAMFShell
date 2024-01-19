#pragma once

// Auxiliary functions
void  read_path       (void);
void  access_error    (const char* cmd, const char* custom, const char* pathname);
bool  can_be_env      (const char* str, int position);
char* basic_directory (const char* pathname);

// Shell functions
stat sls         (const char* pathname);
stat scat        (const char* pathname);
stat smkdir      (const char* pathname);
stat stouch      (const char* pathname);
stat secho       (const char* content);
stat swhich      (const char* cmd);
void init_shell  ();
void close_shell ();
