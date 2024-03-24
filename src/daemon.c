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

/** @brief global verbose option.
 *
 * global verbose option; default 0 - logging disabled; 1 - logging enabled.
 * */
int verbose=0;

const char* program_name;


/** @brief Fn takes format(s) for files we'll search with regex.
*
* Function takes table of char* to arguments wchich are formats for usage in regex.
* At the beginning, it opens syslog and validates input.
*/
int main(int argc, char** argv){
	openlog("FileSearchDaemon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	program_name = *argv;

	/* check for no args */
	if(argc<2)
		return print_usage(stdout, 1);


	/* struct for console options.
	*
	* struct for unix library <getopt.h> implementing command line -v/--verbose option.
	*/
	const struct option long_options[] = {
		{"verbose", 0, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};




	return 0;
}

int overdaemon(size_t subDeamonsCount){

}

int subdaemon(){

}

/** @brief Prints help page.
*
* @param stream output of message.
* @param exit_code value to return from function.
*/
int print_usage(FILE* stream, int exit_code){
	fprintf(stream, "Usage: %s [-v] [pattern1 pattern2 ...]\n", program_name);
	fprintf(stream,
		"   -v  --verbose          Enables verbose logging.\n"
		);
	return exit_code;
}
