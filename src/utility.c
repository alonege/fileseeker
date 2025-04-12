/** @file utility.c
 *  @brief Utility for options scrapping.
 *
 *  @author Kacper Hącia
 */

#include "fileseeker.h"

extern int verbose;
extern int sleep_time;


/** @brief Fn takes arguments to analyse.
*
* Function takes table of char* to arguments wchich will be searched.
* At the beginning, it opens syslog and validates input.
* @param argc number of args; always at least 1 (for index 0 - program name).
* @param argv table of char tables (table of arguments) AKA char** argv or char* argv[].
*/
void options_handler(int argc, char** argv){
	const char* const short_options = "ht:v";
	verbose=0;

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
				verbose++;
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

	/** we handle other arguments (file name patterns). */
	int i = optind;
	//printf("count of patterns: %d\n", argc - i);
	//while(i<argc){
	//	printf("Argument: %s\n", *(argv+i));
	//	++i;
	//}

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
		"  -v   --verbose          Enables verbose logging (-vv or -vvv for debug logging).\n"
		);
	return exit_code;
}
