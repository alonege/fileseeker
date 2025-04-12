/** @file recsearch.c
 *  @brief Recursive search driver.
 *
 * Wrapper function gets offset and sets pointer to searched substring. Then it's calling root function, which calls normal recursive function. Inside them, program checks for access and opens dir (if dir and has access). Then it uses strstr to check if searched word is in file/dir name. If yes, it will log it.
 *  @author Kacper Hącia
 */

#include "fileseeker.h"

/** @brief recursive function for finding word in file names in given dir.
 *
 * @param word_to_find char* of word we want to find (pattern)
 * @param root_path our directory
 */
void search_rec(char* word_to_find, char *root_path) {
	if(flag==flag_scan){/** as long as we're in state of scanning */
		DIR *dir;
		struct dirent *entry;
		//struct stat s;

		/** check access - if we don't have permissions, return. */
		if (access(root_path, R_OK) != 0) {
			return;
		}

		/** let's try open dir - if we don't have permissions, return. */
		if (!(dir = opendir(root_path)))
			return;

		char* path = malloc(MAX_PATH_LEN*sizeof(char));

		while ((entry = readdir(dir)) != NULL && flag==flag_scan) {/** as long as we have dir to analyse we're in state of scanning */
		        //char path[1024];
		        snprintf(path, MAX_PATH_LEN, "%s/%s", root_path, entry->d_name);/** concatenate strings */

		        if (entry->d_type == DT_DIR) {
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) /** check for . and .. dirs; ignore them - continue. */
					continue;
				if(verbose>1)/** if verbose, print info about comparation */
					syslog(LOG_INFO ,"dir compare: dir_name %s searched_pattern %s in %s \n", entry->d_name, word_to_find, root_path);
				if (strstr(entry->d_name, word_to_find) != NULL) {/** if word_to_find is in our dir name, log it. */
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					syslog(LOG_INFO ,"found directory: date: %d-%02d-%02d %02d:%02d:%02d full_path: %s pattern: %s\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, path, word_to_find);
				}
				search_rec(word_to_find, path);
			} else if (entry->d_type == DT_REG) {
				if(verbose>1){/** if verbose, print info about comparation */
					syslog(LOG_INFO ,"file compare: file_name %s searched_pattern %s in %s \n", entry->d_name, word_to_find, root_path);
				}
				if (strstr(entry->d_name, word_to_find) != NULL) {/** if wor_to_find is in our file name, log it. */
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					syslog(LOG_INFO ,"found file: date: %d-%02d-%02d %02d:%02d:%02d full_path: %s pattern: %s\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, path, word_to_find);
				}
			}
		}
		closedir(dir);
		free(path);
	}
}

/** @brief see search_rec; minor change.
 * function for only "/" directory - it changes %s/%s in contencation of root dir and new found dir/file to avoid //home... notation - for docs check fn search_rec.
 */
void search_rec_root(char* word_to_find, char *root_path) {
	if(flag==flag_scan){
		DIR *dir;
		struct dirent *entry;
		//struct stat s;

		if (access(root_path, R_OK) != 0) {
			return;
		}

		if (!(dir = opendir(root_path)))
			return;

		char* path = malloc(MAX_PATH_LEN*sizeof(char));

		while ((entry = readdir(dir)) != NULL && flag==flag_scan) {
		        //char path[1024];
		        snprintf(path, MAX_PATH_LEN, "/%s", entry->d_name);

		        if (entry->d_type == DT_DIR) {
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;
				if(verbose>1)
					syslog(LOG_INFO ,"dir compare: dir_name %s searched_pattern %s in %s \n", entry->d_name, word_to_find, root_path);
				if (strstr(entry->d_name, word_to_find) != NULL) {
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					syslog(LOG_INFO ,"found directory: date: %d-%02d-%02d %02d:%02d:%02d full_path: %s pattern: %s\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, path, word_to_find);
				}
				search_rec(word_to_find, path);
			} else if (entry->d_type == DT_REG) {
				if(verbose>1){
					syslog(LOG_INFO ,"file compare: file_name %s searched_pattern %s in %s \n", entry->d_name, word_to_find, root_path);
				}
				if (strstr(entry->d_name, word_to_find) != NULL) {
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					syslog(LOG_INFO ,"found file: date: %d-%02d-%02d %02d:%02d:%02d full_path: %s pattern: %s\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, path, word_to_find);
				}
			}
		}
		closedir(dir);
		free(path);
	}
}

/** @brief function wraps search function for easy call.
 *
* Function calls root version first, which calls normal search_rec, which calls search_rec, etc...
* @param offset is offset in children_pids array - index (number) of child.
*/
void search_wrapper(int offset){
	/** let's get address of char* word_to_find */
	char* word_to_find = *(glargv + optind + offset);
	if(verbose>2)
		syslog(LOG_INFO, "started searching for: %s\n", (word_to_find));
	/** and start rec ROOT search */
	search_rec_root(word_to_find, "/");
}
