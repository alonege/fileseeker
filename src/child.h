#include <signal.h>
#include "daemon.h"
#ifndef FILE_SEEKER_CHILD
#define FILE_SEEKER_CHILD

extern int verbose;
extern const char* program_name;
extern int sleep_time;
extern volatile sig_atomic_t flag;
extern pid_t pid;
extern pid_t ppid;
extern child_info_ptr children_pids;
extern int children_count;


#endif
