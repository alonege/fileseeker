#include "fileseeker.h"
#include "recsearch.h"
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>

int restarted_scan=0;

/** @brief function masks signals input BEFORE critical sections.
 *
 * @param sig signal to mask besides SIGTERM.
 */
void critical_lock_child(){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	//sigaddset(&sigmask, SIGTERM);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
}


/** @brief function UNmasks signals input AFTER critical sections.
 *
 * @param sig signal to UNmask besides SIGTERM.
 */
void critical_unlock_child(){
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	//sigaddset(&sigmask, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
}

/** @brief send SIGCHLD signal (aka ACK) to parent process
 *
 *
 */
int send_ack_parent(int sig){
	if(ppid>0){
		kill(ppid, sig);
		//syslog(LOG_INFO, "child: ACKed\n");
		return 0;
	} else {
		return 1;
	}
}

/** @brief variable tells us if we ended from sigusr2 (1) or not (0) */
volatile sig_atomic_t got_sigusr2 = 0;

/** @brief Fn handles signals - sets flag for children.
*
*/
void handle_signals_child(int sig, siginfo_t* si, void* data) {
	if(si->si_pid==ppid){
		switch (sig) {
			case SIGUSR1:
				critical_lock_child();
				flag = flag_start;
			break;
			case SIGUSR2:
				critical_lock_child();
				flag = flag_stop;
				got_sigusr2 = 1;
			break;

			default:
			break;
		}
	}
}

int subdaemon(int index){
	flag=flag_sleep;
	struct sigaction sa1;
	memset(&sa1, 0, sizeof(sa1));
	sa1.sa_flags = SA_SIGINFO;
	sa1.sa_sigaction = handle_signals_child;
	if (sigaction(SIGUSR1, &sa1, 0) == -1) {
		return 120;
	}
	struct sigaction sa2;
	memset(&sa2, 0, sizeof(sa2));
	sa2.sa_flags = SA_SIGINFO;
	sa2.sa_sigaction = handle_signals_child;
	if (sigaction(SIGUSR2, &sa2, 0) == -1) {
		return 121;
	}
	memset(&sa2, 0, sizeof(sa2));
	sa2.sa_flags = SA_SIGINFO;
	sa2.sa_handler = SIG_DFL;
	if (sigaction(SIGTERM, &sa2, 0) == -1) {
		return 121;
	}
	memset(&sa2, 0, sizeof(sa2));
	sa2.sa_flags = SA_SIGINFO;
	sa2.sa_handler = SIG_DFL;
	if (sigaction(SIGCHLD, &sa2, 0) == -1) {
		return 121;
	}
	pid=getpid();
	ppid=getppid();
	if(verbose>2)
		syslog(LOG_DEBUG, "child: parent pid is %d\n", ppid);
	critical_unlock_child();
	free((void*) children_pids);
	/** In each new process, launch seeker driver function ...() */
	while (1) {
		switch (flag) {
			case flag_start:
				flag=flag_scan;
				if(verbose&&!restarted_scan)
					syslog(LOG_DEBUG, "child: GOT SIGUSR1\n");
				else 
					restarted_scan=0;
				if(verbose)
					syslog(LOG_DEBUG, "child: woke up\n");
			break;

			case flag_scan:
				critical_unlock_child();
				//work to do - fn call with while flag==flag_scan loop/recursive checking
				//
				//sleep(8);
				search_wrapper(index);

				//TEMPORARY SLEEP FOR SIGNAL DEBUG
				switch (flag) {
					case flag_scan:
						//scan ended by itself
						//syslog(LOG_DEBUG, "succesfully ended search\n");

						flag=flag_stop;//let's inform overlord
					break;

					case flag_start:
						if(verbose)
							syslog(LOG_DEBUG, "child: GOT SIGUSR1 during search, restarting it\n");
						restarted_scan=1;
					break;

					case flag_stop:

					break;

					default:
						abort();
					break;
				}

			break;

			case flag_stop:
				if(!got_sigusr2)
					send_ack_parent(SIGRTMIN);
				if(verbose&&got_sigusr2){
					syslog(LOG_INFO, "child: GOT SIGUSR2\n");
					got_sigusr2=0;
				}

				//syslog(LOG_DEBUG, "CHILD: sended SIGUSR2 to ppid %d\n", ppid);
				flag = flag_sleep;
				//syslog(LOG_DEBUG, "CHILD: unlocked\n");
				critical_unlock_child();
			break;


			case flag_sleep:
				if(verbose)
					syslog(LOG_INFO, "child: went to sleep\n");
				pause();
			break;

			default:
				pause();

			break;
		}
	}
	//printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
	exit(0);

}

