/** @file child.c
 *  @brief Main children process driver.
 *
 * Child after gaining control initializes itself, setting up sigaction signals handlers. Then it's entering state machine in sleeping status. When it gets SIGUSR1 (so flag=flag_start), then it changes state from flag_start to flag_scanning and calls wrapper function for search (using index argument). After search end/interrupt child checks for the cause and makes appropiate steps. If we got stop signal from overlord, it pauses. If it ended scan by itself, it's sending SIGRTMIN to overlord. If it got start signal, it restarts scan, etc... 
 *  @author Kacper Hącia
 */

////////////////Abandon some of hope, ye who enter here.

#include "fileseeker.h"
#include "recsearch.h"
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>

int restarted_scan=0;

/** @brief function masks signals input BEFORE critical sections.
 *
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
 * @return 0 on success; 1 on error.
 */
int send_ack_parent(int sig){
	if(ppid>0){
		kill(ppid, sig);
		return 0;
	} else {
		return 1;
	}
}

/** @brief variable tells us if we ended from sigusr2 (1) or not (0) */
volatile sig_atomic_t got_sigusr2 = 0;

/** @brief Fn handles signals - sets flag for child.
*
* @param sig signal we have received
* @param si siginfo_t element containing info about signal
* @param data unused, but required by sigaction() handler setting function
*/
void handle_signals_child(int sig, siginfo_t* si, void* data) {
	if(si->si_pid==ppid){
		switch (sig) {
			case SIGUSR1:/** case signal SIGUSR1 from overlord - set state to scan. */
				critical_lock_child();
				flag = flag_start;
			break;
			case SIGUSR2:/** case signal SIGUSR2 from overlord - set state to stop; indicate we got SIGUSR2. */
				critical_lock_child();
				flag = flag_stop;
				got_sigusr2 = 1;
			break;

			default:
			break;
		}
	}
}

/** @brief subdaemon is main driver for child. It's state machine. It's checking flag status after waking up and working on this basis.
 *
 * @param index number of child (and number of pattern to use for child)
 */
int subdaemon(int index){
	/** set startup state to sleep. */
	flag=flag_sleep;
	/** and set signals handlers. */
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
	/** let's launch seeker driver switch... */
	while (1) {
		switch (flag) {
			case flag_start:/** we got SIGUSR1 from overlord, which set flag to flag_scan */
				flag=flag_scan;
				if(verbose&&!restarted_scan)
					syslog(LOG_DEBUG, "child: GOT SIGUSR1\n");
				else 
					restarted_scan=0;
				if(verbose)
					syslog(LOG_DEBUG, "child: woke up\n");
			break;

			case flag_scan:/** we entered into scan from flag_scan or restart during previous scan. Let's unlock signals and work. */
				critical_unlock_child();
				/** fn call with while flag==flag_scan loop/recursive checking */
				search_wrapper(index);

				/** we have another internal state submachine */
				switch (flag) {
					case flag_scan:/** if flag is flag_scan - scan ended by itself - let's inform overlord */
						flag=flag_stop;
					break;

					case flag_start:/** if flag is flag_start - during scan we got SIGUSR1 and we need to restart it */

						if(verbose)
							syslog(LOG_DEBUG, "child: GOT SIGUSR1 during search, restarting it\n");
						restarted_scan=1;
					break;

					case flag_stop:

					break;

					default:
						abort();
					break;/** end of state submachine */
				}
			break;

			case flag_stop: /** if flag_stop, we've received SIGUSR2 OR scan ended normally. */
				if(!got_sigusr2)/** ended by itself */
					send_ack_parent(SIGRTMIN);
				if(verbose&&got_sigusr2){/** external end with SIGUSR2 */
					syslog(LOG_INFO, "child: GOT SIGUSR2\n");
					got_sigusr2=0;
				}

				flag = flag_sleep;
				critical_unlock_child();
			break;


			case flag_sleep: /** if flag is flag_sleep - we should pasue and wait for input from overlord. */
				if(verbose)
					syslog(LOG_INFO, "child: went to sleep\n");
				pause();
			break;

			default:
				pause();

			break;
		}
	}
	exit(0);

}

