#include <signal.h>
#include "daemon.h"
#ifndef FILE_SEEKER_CHILD
#define FILE_SEEKER_CHILD

extern int verbose;
extern const char* program_name;
extern int sleep_time;
extern volatile sig_atomic_t flag;
extern volatile pid_t pid;
extern volatile pid_t ppid;
extern child_info_ptr children_pids;
extern int children_count;
extern sem_t *sema;
extern sem_t *semb;
extern int glargc;
extern char** glargv;

#endif
