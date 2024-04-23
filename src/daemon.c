/** @file daemon.c
 *  @brief Main daemon driver.
 *
 *  @author Kacper Hącia (aloneg)
 */

////////////////Abandon all hope, ye who enter here.

#include "daemon.h"
#include <assert.h>
#include <bits/getopt_core.h>
#include <bits/types/sigset_t.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/** @brief global verbose option.
 * global verbose option; default 0 - logging disabled; 1 - logging enabled; 2 - high level of logging; 3 - max level logging.
 * */
int verbose;

/** @brief global char* to program name.*/
const char* program_name;

/** @brief globa sleep time between waking up */
int sleep_time = 60;

/** @brief flag for SIGUSRs (machine state changes with signals) */
volatile sig_atomic_t flag = flag_start;

/** @brief global pid for thread
 * Global pid for thread - used in work, handling SIGUSRs etc */
volatile pid_t pid;

/** @brief global parent pid for thread
 * Global pid for thread - used in work, handling SIGUSRs etc */
volatile pid_t ppid=0;

/** @brief table with pids to childrens (pids (volatile pid_t) + status (volatile int) [alive=flag_sleeping/dead=flag_termination]). */
child_info_ptr volatile children_pids=NULL;

/** @brief count of arguments (how much children we should have) */
int children_count=0;

/** @brief semaphore for synchronizing overlord and children work. It it n-counting semaphore (n=children_count) */
sem_t *sema;

/** @brief semaphore/s for retarting dead children with previous state - pointer to array of n binary semaphores (n=children_count) */
sem_t *semb;

/** semaphore change additional flag - indicator for semaphore change possibility */
volatile sig_atomic_t check_semaphore=0;

/** global argc argv */
int glargc;
char** glargv;

/** @brief Function checks if pid is child of overlord.
 *
 * We're checking if given pid is in pid_t fragment of children_pids array.
 * @param checked_pid pid of process which we want check
 * @return if it's child - return it's (i)ndex; if not, return -1.
 */
volatile int is_child(pid_t checked_pid){
	int i = 0;
	while (i<children_count){
		if((children_pids+i)->pid==checked_pid)
			return i;
		i++;
	}
	return -1;
}

/** @brief Function checks semaphore value - which tells us about number of sleeping children.
 * We're getting number of children not occupying semaphore while sleping. It's because semaphore is decremented while child is working.
 * @return count of sleeping children; on error return -1.  */
volatile int child_sleep_count(){
	int tmp = 0;
	int i = 0;
	while(i<children_count){
		if((children_pids+i)->status==flag_sleep)
			tmp++;
		i++;
	}
	return tmp;
}

/** @brief Function sets given status in children_pids array for each child.
 * 
 * @param status - status which function will set for all children.
 */
void children_status_set(int status){
	int i = 0;
	while (i<children_count){
		//if((children_pids+i)->status!=flag_termination)
			(children_pids+i)->status=status;
		i++;
	}
}

/** @brief debug function for analysing alive children. 
 * */
void children_print_states(){
	int i = 0;
	while (i<children_count){
		syslog(LOG_DEBUG, "status: %d -> %d; alive = %d \n",(children_pids+i)->pid, (children_pids+i)->status, (children_pids+i)->alive);
		i++;
	}
}

/** @brief send signal sig to all forked children.
 *
 * @param sig signal to send
 * @return unused. always returns 0.
 */
int signal_children(int sig){
	int i = 0;
	while(i<children_count){
		if (verbose > 2)
			syslog(LOG_DEBUG, "signal: %d -> %d \n",sig,(children_pids+i)->pid);
		kill((children_pids+i)->pid,sig);
		i++;
	}
	return 0;
}

/** @brief function will check life of all children and ressurect dead ones.
 *
 * Function is making critical lock, after it's iterating through array 
 * children_pids. if it finds dead children (status=flag_termination)
 * it collects zombie kids. Then it tries to fork and subdaemonize new 
 * child. if fork is unsuccesfull, it continues without change. otherwise, 
 * it's setting alive status and saving pid of new children in the place 
 * of old ones. then it checks for previous status of child (by it's 
 * exclusive semaphore in semb array) and if it was working, fixes
 * semaphores sema and semb+i, incrementing their values, altered in process
 * of child being killed. Then, if child was working, it sends SIGUSR1 to it.
 * then, it's analyzing next child (if there's any left)
 */
void check_and_resurrect_children(){
	int i = 0;
	critical_lock();
	while (i<children_count){
		syslog(LOG_DEBUG, "CHILD check\n");
		if((children_pids+i)->alive==child_dead){
			int status=0;
			syslog(LOG_DEBUG, "overlord: CHILD DEAD \n");
			waitpid((children_pids+i)->pid, &status, WNOHANG);
			pid_t newpid=fork();
			if(newpid==-1)
				continue;
			if(newpid==0){//child
				subdaemon(i);
			}
			(children_pids+i)->alive=child_alive;
			(children_pids+i)->pid=newpid;
			syslog(LOG_DEBUG, "overlord: ressurected %d with status %d \n",(children_pids+i)->pid, (children_pids+i)->status);
			
			if ((children_pids+i)->status==flag_scan){
				kill((children_pids+i)->pid,SIGUSR1);
			}
			
		}
		i++;
	}
	critical_unlock();
}

/** @brief function masks signals input BEFORE critical sections.
 *
 */
void critical_lock_termstage(){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
}

/** @brief Fn handles signals - sets flag for overlord.
*
* @param sig signal we have received
* @param si siginfo_t element containing info about signal
* @param data unused, but required by sigaction() handler setting function
*/
void handle_signals(int sig, siginfo_t* si, void* data) {

	int temp=0;
	switch (sig) {
		case SIGUSR1:
			critical_lock();
			flag = flag_start;
		break;
		case SIGUSR2:
			critical_lock();
			/** if SIGUSR2 wasn't send by child, we should handle it */
			flag = flag_stop;
			/** let's require semaphore check (for countin sleeping children) */
			check_semaphore=1;
		break;

		case SIGTERM:
			critical_lock_termstage();
			flag=flag_termination;

		break;

		case SIGCHLD:
			/** let's handle SIGCHLD. we set flag_termination for si_pid wchich sended SIGCHLD */
			temp=is_child(si->si_pid);
			(children_pids+temp)->alive=child_dead;
		break;


		default:
		break;
	}
}

void handle_rt(int sig, siginfo_t* si, void* data){
	int temp=is_child(si->si_pid);
	(children_pids+temp)->status=flag_sleep;
	return;
}

/** @brief function masks SIGUSR1 input BEFORE critical sections. */
void critical_lock(){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	sigaddset(&sigmask, SIGRTMIN);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
}



/** @brief function UNmasks SIGUSR1 input AFTER critical sections. */
void critical_unlock(){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	sigaddset(&sigmask, SIGRTMIN);
	sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
}


/** @brief Fn is main driver for other functionalities.
*
* Function takes table of char* to arguments wchich are formats for usage in regex.
* @param argc number of args; always at least 1 (for index 0 - program name).
* @param argv table of char tables (table of arguments) AKA char** argv or char* argv[].
*/
int main(int argc, char** argv){
	/** Set verbose to 0 and get program_name from first argument. */
	verbose=0;
	program_name = *argv;

	/** Open syslog. */
	openlog(program_name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

	/* Check for no args */
	if(argc<2)
		return print_usage(stdout, 1);

	glargc=argc;
	glargv=argv;

	/** Function call options_handler to handle options and set optind for overlord. */
	options_handler(argc, argv);

	children_count=argc-optind;

	/** Initalizes array for children_pids with memset to 0. */
	children_pids = malloc(sizeof(child_info)*children_count);
	if(!children_pids)
		abort();
	memset((void*) children_pids, 0, children_count*sizeof(child_info));


	
	/** Deamonize program */
	daemon(1, 0);

	/** Transfer program control for overlord function. */
	overlord(argc, argv);

	return 0;
}


/** @brief Fn is driver for creating and overwatching working process.
*
* Function takes table of char* to arguments wchich are formats for usage in regex.
* @param argc number of args; always at least 1 (for index 0 - program name).
* @param argv table of char tables (table of arguments) AKA char** argv or char* argv[].
*/
int overlord(int argc, char**argv){
	/** children_count = argc - optind; */
	
	/** create our subdaemons */
	create_subdaemons(argc, argv);

	/** let our children init themself by sleeping a while */
	sleep(1+children_count/5);
	if(pid){//overlord process
		pid = getpid();
		/** Handle SIGTERM */
		/** Registers handlers for SIGUSRs. */
		struct sigaction sa1;
		memset(&sa1, 0, sizeof(sa1));
		sa1.sa_flags = SA_SIGINFO;
		sa1.sa_sigaction = handle_signals;
		if (sigaction(SIGUSR1, &sa1, 0) == -1) {
			free((void*)children_pids);
			return 120;
		}
		struct sigaction sa2;
		memset(&sa2, 0, sizeof(sa2));
		sa2.sa_flags = SA_SIGINFO;
		sa2.sa_sigaction = handle_signals;
		if (sigaction(SIGUSR2, &sa2, 0) == -1) {
			free((void*)children_pids);
			return 121;
		}

		memset(&sa1, 0, sizeof(sa1));
		sa1.sa_flags = SA_SIGINFO;
		sa1.sa_sigaction = handle_signals;
		if (sigaction(SIGTERM, &sa1, 0) == -1) {
			free((void*)children_pids);
			return 123;
		}	
		memset(&sa2, 0, sizeof(sa2));
		sa2.sa_flags = SA_SIGINFO;
		sa2.sa_sigaction = handle_signals;
		if (sigaction(SIGCHLD, &sa2, 0) == -1) {
			free((void*)children_pids);
			return 121;
		}
		memset(&sa2, 0, sizeof(sa2));
		sa2.sa_flags = SA_SIGINFO;
		sa2.sa_sigaction = handle_rt;
		if (sigaction(SIGRTMIN, &sa2, 0) == -1) {
			free((void*)children_pids);
			return 121;
		}


		/** let's start our first scan! */
		raise(SIGUSR1);

		while (1) {
			switch (flag) {
				case flag_start: /** case flag==1: send SIGUSR1 to child to start search */
					if (verbose)
						syslog(LOG_INFO, "overlord: GOT SIGUSR1\n");
					signal_children(SIGUSR1);
					flag = flag_scan;
					children_status_set(flag_scan);

				break;

				case flag_stop: /** case flag==2: send SIGUSR2 to child to stop search */
					if (verbose)
						syslog(LOG_INFO, "overlord: GOT SIGUSR2\n");
					children_status_set(flag_sleep);
					signal_children(SIGUSR2);
					flag = flag_sleep;
					critical_unlock();
				break;

				case flag_scan:
					if (verbose)
						syslog(LOG_INFO, "overlord: woke up\n");
					/** if all children are in state of sleeping, it means all children have ended work. */
					critical_unlock();
					if (verbose > 2)
						children_print_states();
					check_and_resurrect_children();
					if (verbose > 2)
						children_print_states();
					if(child_sleep_count()==children_count){
						flag=flag_sleep;
						if (verbose > 2)
							syslog(LOG_DEBUG, "overlord: all children sleeps\n");
					} else if (flag==flag_scan) {
						if (verbose > 2)
							syslog(LOG_DEBUG, "overlord: %d children sleep\n",child_sleep_count());
						if (verbose)
							syslog(LOG_INFO, "overlord: went to sleep\n");
						pause();

					}


				break;

				case flag_sleep:

					//restart children if needed
					check_and_resurrect_children();
					critical_unlock();
					if (verbose)
						syslog(LOG_INFO, "overlord: went to sleep for %d seconds; job done\n", sleep_time);
					sleep(sleep_time);
					check_and_resurrect_children();
					switch (flag) {
						case flag_sleep:
							critical_lock();
							signal_children(SIGUSR1);
							flag = flag_scan;
							children_status_set(flag_scan);
						break;
						default:

						break;
					}
				break;
				case flag_termination:
					critical_lock_termstage();
					signal_children(SIGTERM);
					for(int i=optind;i<glargc;i++){
						wait(NULL);
					}
					free((void*) children_pids);
					return 0;
				break;

			}		
		}
	} else {//child process
		assert(!pid);
	}

	return 0;
}

int create_subdaemons(int argc, char** argv){
	/** From getopt, we use optind to finde first pattern argument. For each pattern create process. */
	for(int i=0;i<children_count;i++){
		pid=fork();
		if(pid == 0){
			subdaemon(i);
		}
		(children_pids+i)->pid=pid;
		syslog(LOG_DEBUG, "overlord: created child with pid %d\n", (children_pids+i)->pid);
		(children_pids+i)->status=flag_sleep;
		(children_pids+i)->alive=child_alive;
	}
	return 0;
}


