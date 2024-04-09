#include "fileseeker.h"

extern int verbose;
extern int sleep_time;


/** @brief Fn takes format(s) for files we'll search with regex.
*
* Function takes table of char* to arguments wchich are formats for usage in regex.
* At the beginning, it opens syslog and validates input.
* @param argc number of args; always at least 1 (for index 0 - program name).
* @param argv table of char tables (table of arguments) AKA char** argv or char* argv[].
*/
void options_handler(int argc, char** argv){
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

}
