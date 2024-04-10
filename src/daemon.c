/** @file daemon.c
 *  @brief Main daemon driver.
 *
 *  @author Kacper Hącia (aloneg)
 */

#include "daemon.h"
#include <assert.h>
#include <bits/getopt_core.h>
#include <bits/types/sigset_t.h>
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
 * flag for SIGUSRs - 0 - no flag, 1 - SIGUSR1, 2 - SIGUSR2
 */
volatile sig_atomic_t flag = 0;

/** @brief global pid for thread
 *
 * Global pid for thread - used in work, handling SIGUSRs etc
 */
pid_t pid;

/** @brief global parent pid for thread
 *
 * Global pid for thread - used in work, handling SIGUSRs etc
 */
volatile pid_t ppid=0;

/** @brief table with pids to childrens.
 *
 *
 */
child_info_ptr children_pids=NULL;

/** @brief count of forked childrens
 *
 *
 */
int children_count=0;

/** @brief Fn handles signals - sets flag for overlord.
*
*/
void handle_signals(int sig, siginfo_t* si, void* data) {
	switch (sig) {
		case SIGUSR1:
			critical_lock(SIGUSR2);
			flag = flag_start;
		break;
		case SIGUSR2:
			critical_lock(SIGUSR1);
			flag = flag_stop;
		break;

		case SIGTERM:
			flag = flag_termination;
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
	sigaddset(&sigmask, sig);
	//sigaddset(&sigmask, SIGUSR1);
	//sigaddset(&sigmask, SIGUSR2);
	sigaddset(&sigmask, SIGTERM);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
	//syslog(LOG_DEBUG, "overlord: critical section masked %d.\n", sig);
}


/** @brief function UNmasks signals input AFTER critical sections.
 *
 * @param sig signal to UNmask besides SIGTERM.
 */
void critical_unlock(int sig){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, sig);
	//sigaddset(&sigmask, SIGUSR1);
	//sigaddset(&sigmask, SIGUSR2);
	sigaddset(&sigmask, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
	//syslog(LOG_DEBUG, "overlord: critical section UNmasked %d.\n", sig);
	//signal(SIGUSR1, handle_signals);
	//signal(SIGUSR2, handle_signals);
	//syslog(LOG_DEBUG, "overlord: critical section lock DISABLED (OFF).\n");
}

//WARNING - untested!!!!
/** @brief send signal sig to all forked children.
 *
 */
int signal_children(int sig){
	int i = 0;
	while(i<children_count){
		kill((children_pids+i)->status,sig);
		i++;
	}
	return 0;
}

//WARNING - untested!!!!
/** @brief send signal sig to all forked children and awaits for SIGCHLD response.
 *
 */
int signal_children_wait(int sig){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGCHLD);
	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	int status=0;
	int i = 0;
	/** For each children pid */
	while(i<children_count){
		/** send sig signal */
		kill((children_pids+i)->status,sig);
		i++;
	}
	for (i = 0; i < children_count; i++){
		sigwait(&sigmask, &status);
		syslog(LOG_INFO, "overlord: WAITING FOR CHILD SIGNAL\n");
	}

	return 0;
}

//WARNING - bugged
/** @brief check if all children are alive. 0 - alive, 1 - at least one process is dead.
 *
 *
 */
int check_children_alive(){
	int status;
	child_info_ptr tempchildrenpids = children_pids;
	int count_of_alive = 0;
	int i = 0;
	while (i<children_count){
		pid_t result = waitpid((tempchildrenpids+i)->status, &status, WNOHANG);
		if (result == 0) {
			// Child still alive
			count_of_alive++;
		} else if (result == -1) {
			// Error 
		} else {
			// Child exited
		}
	}
	if(children_count==count_of_alive)
		return 0;
	else
		return 1;
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
	
	create_subdaemons(argc);

	if(pid){//overlord process
		pid = getpid();
		/** Handle SIGTERM */
		struct sigaction sa1;
		memset(&sa1, 0, sizeof(sa1));
		sa1.sa_flags = SA_SIGINFO;
		sa1.sa_sigaction = handle_signals;
		if (sigaction(SIGTERM, &sa1, 0) == -1) {
			return 123;
		}

		while (flag!=flag_termination) {
			//add life validation
			switch (flag) {
				case flag_start: /** case flag==1: send SIGUSR1 to child to start search */

					syslog(LOG_INFO, "overlord: GOT SIGUSR1, sending\n");
					signal_children(SIGUSR1);
					syslog(LOG_INFO, "overlord: got ACK SIGUSR1\n");
					flag = flag_scan;
					critical_unlock(SIGUSR2);
				break;

				case flag_stop: /** case flag==2: send SIGUSR2 to child to stop search */

					syslog(LOG_INFO, "overlord: GOT SIGUSR2, sending\n");
					signal_children(SIGUSR2);
					syslog(LOG_INFO, "overlord: got ACK SIGUSR2\n");
					flag = flag_sleep;
					critical_unlock(SIGUSR1);
				break;

				case flag_scan:
					syslog(LOG_DEBUG, "overlord: flag_scan case\n");
					pause();
				break;

				case flag_sleep:
					syslog(LOG_DEBUG, "overlord: flag_sleep case\n");
					sleep(sleep_time);
				break;

				case flag_termination:

					//signal_children(SIGTERM);
				break;

			}		
		}
	} else {//child process
		assert(!pid);
	}

	syslog(LOG_DEBUG, "overlord: flag_termination case\n");
	signal_children(SIGTERM);
	
	for(int i=optind;i<argc;i++){
		wait(NULL);
	}
	free((void*) children_pids);
	//not implemented
	return 0;
}

int create_subdaemons(int argc){
	/** From getopt, we use optind to finde first pattern argument. For each pattern create process. */
	for(int i=0;i<children_count;i++){
		pid=fork();
		if(pid == 0){
			subdaemon(i);
		}
		(children_pids+i)->pid=pid;
		(children_pids+i)->status=flag_sleep;
	}
	return 0;
}


