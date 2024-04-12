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
 *
 * global verbose option; default 0 - logging disabled; 1 - logging enabled.
 * */
int verbose;

/** @brief global char* to program name.
 *
 * */
const char* program_name;

/** @brief globa sleep time between waking up
 *
 */
int sleep_time = 60;

/** @brief flag for SIGUSRs
 *
 */
volatile sig_atomic_t flag = flag_start;

/** @brief global pid for thread
 *
 * Global pid for thread - used in work, handling SIGUSRs etc
 */
volatile pid_t pid;

/** @brief global parent pid for thread
 *
 * Global pid for thread - used in work, handling SIGUSRs etc
 */
volatile pid_t ppid=0;

/** @brief table with pids to childrens.
 *
 *
 */
child_info_ptr volatile children_pids=NULL;

/** @brief count of forked childrens
 *
 *
 */
int children_count=0;

/** @brief semaphore for synchronizing overlord and children work. It it n-counting semaphore (n=children_count) */
sem_t *sema;

/** @brief semaphore for retarting dead children with previous state - pointer to array of n binary semaphores (n=children_count) */
sem_t *semb;

/** semaphore change additional flag - indicator for semaphore change possibility */
volatile sig_atomic_t check_semaphore=0;

/** global argc argv */
int glargc;
char** glargv;

/** @brief Function checks if pid is child of overlord. if yes, ret child number; else ret 0.
 *
 * @param checked_pid pid of process which we want check
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
 *
 */
volatile int child_sleep_count(){
	int semval;
	if (!sem_getvalue(sema, &semval)){
		return semval;
	}
	else return -1;
}

/** @brief Function checks number of child with status status.
 *
 */
void children_status_set(int status){
	int i = 0;
	while (i<children_count){
		//if((children_pids+i)->status!=flag_termination)
			(children_pids+i)->status=status;
		i++;
	}
}

void children_print_states(){
	int i = 0;
	while (i<children_count){
		syslog(LOG_DEBUG, "status: %d -> %d \n",(children_pids+i)->pid, (children_pids+i)->status);
		i++;
	}
}

//WARNING - untested!!!!
/** @brief send signal sig to all forked children.
 *
 */
int signal_children(int sig){
	int i = 0;
	while(i<children_count){
		syslog(LOG_DEBUG, "signal: %d -> %d \n",sig,(children_pids+i)->pid);
		kill((children_pids+i)->pid,sig);
		i++;
	}
	return 0;
}

void check_and_resurrect_children(){
	int i = 0;
	critical_lock(SIGUSR1);
	while (i<children_count){
		if((children_pids+i)->status==flag_termination){
			int status=0;
			waitpid((children_pids+i)->pid, &status, WNOHANG);
			pid_t newpid=fork();
			if(newpid==-1)
				continue;
			if(newpid==0){//child
				subdaemon(i);
			}
			(children_pids+i)->status=flag_sleep;
			(children_pids+i)->pid=newpid;
			syslog(LOG_DEBUG, "ressurected %d with status %d \n",(children_pids+i)->pid, (children_pids+i)->status);
			
			int semval;
			if (!sem_getvalue(semb+i, &semval)){
				if(semval==0){
			//		syslog(LOG_DEBUG, "BEFORE sem_post - ressurected %d with status %d \n",(children_pids+i)->pid, (children_pids+i)->status);
					sem_post(semb+i);
					sem_post(sema);
			//		syslog(LOG_DEBUG, "after sem_post - ressurected %d with status %d \n",(children_pids+i)->pid, (children_pids+i)->status);
					kill((children_pids+i)->pid,SIGUSR1);
				}
			}
			
		}
		i++;
	}
	critical_unlock(SIGUSR1);
}


/** @brief Fn handles signals - sets flag for overlord.
*
*/
void handle_signals(int sig, siginfo_t* si, void* data) {

	int volatile temp=0;
	switch (sig) {
		case SIGUSR1:
			critical_lock(SIGUSR2);
			flag = flag_start;
		break;
		case SIGUSR2:
			critical_lock(SIGUSR1);
			/** if SIGUSR2 wasn't send by child, we should handle it */
			temp=is_child(si->si_pid);
			if(temp>-1){
				//set child to done
				//(children_pids+temp)->status=flag_sleep;
			} else {
				/** not a child */
				flag = flag_stop;
			}
			check_semaphore=1;
		break;

		case SIGTERM:
			signal_children(SIGTERM);
			for(int i=optind;i<glargc;i++){
				wait(NULL);
			}
			free((void*) children_pids);
		break;

		case SIGCHLD:
			//critical_lock(SIGUSR1);
			/** let's handle SIGCHLD. we set flag_termination for si_pid wchich sended SIGCHLD */
			temp=is_child(si->si_pid);
			(children_pids+temp)->status=flag_termination;
			//check_and_resurrect_children();
		break;

		default:
		break;
	}
}

/** @brief function masks signals input BEFORE critical sections.
 *
 * @param sig signal to mask besides SIGTERM.
 */
void critical_lock(int sig){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	//sigaddset(&sigmask, sig);
	sigaddset(&sigmask, SIGUSR1);
	//sigaddset(&sigmask, SIGUSR2);
	//sigaddset(&sigmask, SIGTERM);
	//sigaddset(&sigmask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
}


/** @brief function UNmasks signals input AFTER critical sections.
 *
 * @param sig signal to UNmask besides SIGTERM.
 */
void critical_unlock(int sig){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	//sigaddset(&sigmask, sig);
	sigaddset(&sigmask, SIGUSR1);
	//sigaddset(&sigmask, SIGUSR2);
	//sigaddset(&sigmask, SIGTERM);
	//sigaddset(&sigmask, SIGCHLD);
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

	/** Function call options_handler to handle options and set optind for overlord. */
	options_handler(argc, argv);

	children_count=argc-optind;

	/** Initalizes array for children_pids with memset to 0. */
	children_pids = malloc(sizeof(child_info)*children_count);
	if(!children_pids)
		abort();
	memset((void*) children_pids, 0, children_count*sizeof(child_info));

	/* place semaphore in shared memory */
	sema = mmap(NULL, sizeof(*sema), PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (sema == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	/* place semaphore in shared memory */
	semb = mmap(NULL, sizeof(*semb)*children_count, PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (sema == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	/* create/initialize semaphore */
	if ( sem_init(sema, 1, children_count) < 0) {
	        perror("sem_init");
	        exit(EXIT_FAILURE);
	}
	/* create/initialize semaphores - one for each subdaemon */
	int i = 0;
	while (i<children_count){
		if ( sem_init(semb+i, 1, 1) < 0) {
			perror("sem_init");
			exit(EXIT_FAILURE);
		}
		i++;
	}
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
	//sa2.sa_flags = SA_SIGINFO|SA_NODEFER;
	sa2.sa_flags = SA_SIGINFO;
	sa2.sa_sigaction = handle_signals;
	if (sigaction(SIGUSR2, &sa2, 0) == -1) {
		free((void*)children_pids);
		return 121;
	}
	
	/** Deamonize program */
	daemon(1, 0);

	/** WARNING - EXPERIMENTAL - Transfer program control for overlord fun. */
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
	//WARNING - HIGHLY EXPERIMENTAL!!!!
	//children_count = argc - optind;
	
	create_subdaemons(argc, argv);

	//let our children init themself
	sleep(1+children_count/5);
	if(pid){//overlord process
		pid = getpid();
		/** Handle SIGTERM */
		struct sigaction sa1;
		memset(&sa1, 0, sizeof(sa1));
		sa1.sa_flags = SA_SIGINFO;
		sa1.sa_sigaction = handle_signals;
		if (sigaction(SIGTERM, &sa1, 0) == -1) {
			free((void*)children_pids);
			return 123;
		}	
		struct sigaction sa2;
		memset(&sa2, 0, sizeof(sa2));
		sa2.sa_flags = SA_SIGINFO;
		sa2.sa_sigaction = handle_signals;
		if (sigaction(SIGCHLD, &sa2, 0) == -1) {
			free((void*)children_pids);
			return 121;
		}

		critical_lock(SIGUSR2);
		flag = flag_start;
		//raise(SIGUSR1);

		while (1) {
			//add life validation
			switch (flag) {
				case flag_start: /** case flag==1: send SIGUSR1 to child to start search */
					//children_status_set(flag_scan);
					if (verbose)
						syslog(LOG_INFO, "overlord: woke up\n");
					signal_children(SIGUSR1);
					flag = flag_scan;

					if (verbose)
						syslog(LOG_INFO, "overlord: went to sleep\n");
					//sleep(1);
					pause();
					critical_unlock(SIGUSR2);
				break;

				case flag_stop: /** case flag==2: send SIGUSR2 to child to stop search */
					if (verbose)
						syslog(LOG_INFO, "overlord: GOT SIGUSR2\n");
					children_status_set(flag_sleep);
					signal_children(SIGUSR2);
					//syslog(LOG_INFO, "overlord: got ACK SIGUSR2\n");
					flag = flag_sleep;
					critical_unlock(SIGUSR1);
				break;

				case flag_scan:
					if (verbose)
						syslog(LOG_INFO, "overlord: woke up\n");
					//restart children if needed
					/** if all children are in state of sleeping, it means all children have ended work. */
					if(child_sleep_count()==children_count){
						flag=flag_sleep;
						syslog(LOG_DEBUG, "overlord: all children sleeps\n");
					} else {
						syslog(LOG_DEBUG, "overlord: %d children sleep\n",child_sleep_count());
					}
					children_print_states();
					check_and_resurrect_children();
					critical_unlock(SIGUSR1);
					if(flag==flag_scan&&(!check_semaphore)){
						syslog(LOG_INFO, "overlord: went to sleep\n");
						pause();
					} else {
						//syslog(LOG_INFO, "overlord: went to short sleep during scan\n");
						check_semaphore=0;
						sleep(1);
					}
				break;

				case flag_sleep:

					//restart children if needed
					critical_unlock(SIGUSR1);
					if (verbose)
						syslog(LOG_INFO, "overlord: went to sleep\n");
					sleep(sleep_time);
					check_and_resurrect_children();
					switch (flag) {
						case flag_sleep:
							critical_lock(SIGUSR1);
							flag=flag_start;
						break;
						case flag_start:
							if (verbose)
								syslog(LOG_INFO, "overlord: GOT SIGUSR1\n");
						break;
						default:

						break;
					}
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
	}
	return 0;
}


