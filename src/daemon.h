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

#ifndef __file_seeker_daemon
#define __file_seeker_daemon

int print_usage(FILE* stream, int exit_code);

#endif
