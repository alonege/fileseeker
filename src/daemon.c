/** @file daemon.c
 *  @brief Main daemon driver.
 *
 *  These empty function definitions are provided
 *  so that stdio will build without complaining.
 *  You will need to fill these functions in. This
 *  is the implementation of the console driver.
 *  Important details about its implementation
 *  should go in these comments.
 *
 *  @author Kacper Hącia (aloneg)
 */

#include "daemon.h"
#include <bits/getopt_core.h>
#include <stdio.h>
#include <sys/syslog.h>
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

int sleep_time = 60;

volatile sig_atomic_t flag = 0;

/** @brief Fn handles SIGUSR1 signal - sets flag to enable scanning.
*
*/
void handle_sigusr1(int sig) {
	flag = 1;
	syslog(LOG_INFO, "GOT SIGUSR1");
}

/** @brief Fn handles SIGUSR1 signal - sets flag to sleep.
*
*/
void handle_sigusr2(int sig) {
	flag = 2;
	syslog(LOG_INFO, "GOT SIGUSR2");
}

/** @brief Fn takes format(s) for files we'll search with regex.
*
* Function takes table of char* to arguments wchich are formats for usage in regex.
* At the beginning, it opens syslog and validates input.
* @param argc number of args; always at least 1 (for index 0 - program name).
* @param argv table of char tables (table of arguments) AKA char** argv or char* argv[].
*/
int main(int argc, char** argv){
	/** Program registers handlers for SIGUSRs. */
	signal(SIGUSR1, handle_sigusr1);
	signal(SIGUSR2, handle_sigusr2);

	verbose=0;
	program_name = *argv;

	/** function opens syslog. */
	openlog(program_name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

	/* check for no args */
	if(argc<2)
		return print_usage(stdout, 1);

	const char* const short_options = "ht:v";

	/* struct for console options.
	*
	* struct for unix library <getopt.h> implementing command line -v/--verbose option.
	*/
	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"time", 1, NULL, 't'},
		{"verbose", 0, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};

	int next_option;
	int temp_time;

	/** then it scans for -h or -v options. */
	do{
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch (next_option) {
			case 'h': /*-h or --help : help*/
				print_usage(stdout, 0);
			break;

			case 'v': /*-v or --verbose : logging*/
				verbose=1;
			break;

			case 't':
				temp_time = atoi(optarg);
				sleep_time = (temp_time>0)? temp_time : sleep_time;
				if(temp_time<=0)
					printf("Warning: time at -t option is 0 or less. Using default sleep time - %d sec.", sleep_time);
			break;

			case '?': /*invalid opt*/
				print_usage(stdout, 1);
			break;

			case -1: /*all opts done*/
			break;

			default: /*something wrong*/
			abort();
		}
	/** option scan is continued untill we're out of options. */
	} while(next_option!=-1);

	/** we handle other arguments (file name patterns). For each pattern, we do [WARNING - DOCUMENT IT LATER] */
	int i = optind;
	printf("count of patterns: %d\n", argc - i);
	while(i<argc){
		printf("Argument: %s\n", *(argv+i));
		//TODO regex for each arg? or send it later to childrens?
		++i;
	}

	daemon(1, 0);
	overlord(argc, argv, optind);




	return 0;
}

int overlord(int argc, char**argv, int daemons_count){
	//WARNING - HIGHLY EXPERIMENTAL!!!!
	while (1) {
		if (flag == 1) {
			syslog(LOG_INFO, "GOT SIGUSR1, starting search\n");
			//action();
			flag = 0;
		} else if (flag == 2) {
			syslog(LOG_INFO, "GOT SIGUSR2, stopping search\n");
			//stop action
			sleep(sleep_time);
			flag = 0;
		} else {
			//action();
			sleep(sleep_time);  // zastąp t liczbą minut
		}
	}
	//not implemented
	return 127;
}

int subdaemon(){
	return 127;
}

/** @brief Prints help page.
*
* @param stream output of message.
* @param exit_code value to return from function.
*/
int print_usage(FILE* stream, int exit_code){
	fprintf(stream, "Usage: %s [-v] [-t n] [pattern1 pattern2 ...]\n", program_name);
	fprintf(stream,
		"  -h   --help             Shows this help and exits.\n"
		"  -t n --time n           Sets Daemon sleep time for n seconds.\n"
		"  -v   --verbose          Enables verbose logging.\n"
		);
	return exit_code;
}
