#include "fileseeker.h"
/** @brief Fn handles signals - sets flag for children.
*
*/
void handle_signals_child(int sig, siginfo_t* si, void* data) {
	switch (sig) {
		case SIGUSR1:
			flag = flag_start;
		break;
		case SIGUSR2:
			flag = flag_stop;
		break;

		default:
		break;
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
				syslog(LOG_INFO, "GOT SIGUSR1, starting search\n");
				//action();
				//send_ack_parent(SIGCHLD);
				flag = flag_scan;
			break;

			case flag_stop:
				syslog(LOG_INFO, "GOT SIGUSR2, stopping search\n");
				//stop action
				flag = flag_sleep;
			break;

			case flag_scan:
				//WARNING - development and debug pause
				pause();
			break;

			case flag_sleep:
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

