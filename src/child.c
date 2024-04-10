#include "fileseeker.h"
#include <stdlib.h>

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
		syslog(LOG_INFO, "child: ACKed\n");
		return 0;
	} else {
		return 1;
	}
}

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
			break;

			default:
			break;
		}
	}
}

int subdaemon(int index){
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
	pid=getpid();
	ppid=getppid();
	free((void*) children_pids);
	/** In each new process, launch seeker driver function ...() */
	while (1) {
		switch (flag) {
			case flag_start:
				flag=flag_scan;
				syslog(LOG_DEBUG, "GOT SIGUSR1, starting search\n");
				syslog(LOG_DEBUG, "CHILD: unlocked\n");
				critical_unlock_child();
				//work to do - fn call with while flag==flag_scan loop/recursive checking
				switch (flag) {
					case flag_scan:
						//scan ended by itself
						syslog(LOG_DEBUG, "succesfully ended search\n");
						flag=flag_stop;//let's inform overlord
					break;

					case flag_stop:
						syslog(LOG_DEBUG, "GOT SIGUSR2, stopping search\n");
						flag=flag_sleep;
					break;

					case flag_start:
						syslog(LOG_DEBUG, "GOT SIGUSR1 during search, restarting it\n");
					break;

					default:
						abort();
					break;
				}
			break;

			case flag_stop:
				//stop action
				syslog(LOG_DEBUG, "CHILD: flag_stop case\n");
				send_ack_parent(SIGUSR2);
				flag = flag_sleep;
				syslog(LOG_DEBUG, "CHILD: unlocked\n");
				critical_unlock_child();
			break;


			case flag_sleep:
				syslog(LOG_DEBUG, "CHILD: flag_sleep case\n");
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

