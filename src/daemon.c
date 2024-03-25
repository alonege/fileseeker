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

/** @brief global verbose option.
 *
 * global verbose option; default 0 - logging disabled; 1 - logging enabled.
 * */
int verbose;

/** @brief global char* to program name.
 *
 * */
const char* program_name;


/** @brief Fn takes format(s) for files we'll search with regex.
*
* Function takes table of char* to arguments wchich are formats for usage in regex.
* At the beginning, it opens syslog and validates input.
* @param argc number of args; always at least 1 (for index 0 - program name).
* @param argv table of char tables (table of arguments) AKA char** argv or char* argv[].
*/
int main(int argc, char** argv){
	/** function opens syslog. */
	openlog("FileSearchDaemon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1); //WARNING - no idea what's going on there XD

	verbose=0;
	program_name = *argv;

	/* check for no args */
	if(argc<2)
		return print_usage(stdout, 1);

	const char* const short_options = "hv";

	/* struct for console options.
	*
	* struct for unix library <getopt.h> implementing command line -v/--verbose option.
	*/
	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"verbose", 0, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};

	int next_option;

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
	while(i<argc){
		printf("Argument: %s\n", *(argv+i));
		++i;
	}





	return 0;
}

int overdaemon(size_t sub_daemon_count){
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
	fprintf(stream, "Usage: %s [-v] [pattern1 pattern2 ...]\n", program_name);
	fprintf(stream,
		"   -h  --help             Shows this help and exits.\n"
		"   -v  --verbose          Enables verbose logging.\n"
		);
	return exit_code;
}
