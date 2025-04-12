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
#include <semaphore.h>
#include <sys/mman.h>

#ifndef __file_seeker_daemon
#define __file_seeker_daemon

#define __file_seeker_max_arg_len 127

/** Automata states */
#define flag_sleep 0
#define flag_start 1
#define flag_scan 2
#define flag_stop 3
#define flag_termination 4

/** alive states */
#define child_alive 1
#define child_dead 0

/** @brief struct for holding data about childrens - pids and their status.
*
*
*/
typedef volatile struct chld_info {
	volatile pid_t pid;
	volatile sig_atomic_t status;
	volatile sig_atomic_t alive;
} child_info, * volatile child_info_ptr;

int print_usage(FILE* stream, int exit_code);
int overlord(int argc, char**argv);
void options_handler(int argc, char** argv);
int create_subdaemons(int argc, char** argv);
void critical_lock();
void critical_unlock();
int subdaemon(int index);

#endif
