#pragma once

stat sls(const char* pathname);
stat scat(const char* pathname);
stat smkdir(const char* pathname);
stat stouch(const char* pathname);
stat secho(const char* content);
stat swhich(const char* cmd);
void init_shell();
void close_shell();
