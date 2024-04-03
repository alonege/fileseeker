#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <bits/getopt_ext.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#ifndef __file_seeker_daemon
#define __file_seeker_daemon

#define __file_seeker_max_arg_len 127

#define flag_sleep 0
#define flag_start 1
#define flag_stop 2
#define flag_scan 3
#define flag_termination 4

int print_usage(FILE* stream, int exit_code);
int overlord(int argc, char**argv);
void options_handler(int argc, char** argv);
int create_subdaemons(int argc);
void critical_lock(int sig);
void critical_unlock(int sig);

#endif
