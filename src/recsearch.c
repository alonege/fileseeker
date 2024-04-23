#include "fileseeker.h"

void search_rec(char* word_to_find, char *root_path) {
	if(flag==flag_scan){
		DIR *dir;
		struct dirent *entry;
		struct stat s;

		if (access(root_path, R_OK) != 0) {
			return;
		}

		if (!(dir = opendir(root_path)))
			return;

		size_t file_path_size;
		char* path = malloc(MAX_PATH_LEN*sizeof(char));

		while ((entry = readdir(dir)) != NULL && flag==flag_scan) {
		        //char path[1024];
		        snprintf(path, MAX_PATH_LEN, "%s/%s", root_path, entry->d_name);

		        if (entry->d_type == DT_DIR) {
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;
				if(verbose)
					syslog(LOG_INFO ,"dir compare: dir_name %s searched_pattern %s in %s \n", entry->d_name, word_to_find, root_path);
				if (strstr(entry->d_name, word_to_find) != NULL) {
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					syslog(LOG_INFO ,"found directory: date: %d-%02d-%02d %02d:%02d:%02d full_path: %s pattern: %s\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, path, word_to_find);
				}
				search_rec(word_to_find, path);
			} else if (entry->d_type == DT_REG) {
				if(verbose){
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

void search_rec_root(char* word_to_find, char *root_path) {
	if(flag==flag_scan){
		DIR *dir;
		struct dirent *entry;
		struct stat s;

		if (access(root_path, R_OK) != 0) {
			return;
		}

		if (!(dir = opendir(root_path)))
			return;

		size_t file_path_size;
		char* path = malloc(MAX_PATH_LEN*sizeof(char));

		while ((entry = readdir(dir)) != NULL && flag==flag_scan) {
		        //char path[1024];
		        snprintf(path, MAX_PATH_LEN, "/%s", entry->d_name);

		        if (entry->d_type == DT_DIR) {
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;
				if (strstr(entry->d_name, word_to_find) != NULL) {
					syslog(LOG_INFO ,"found directory: %s\n", path);
				}
				search_rec(word_to_find, path);
			} else if (entry->d_type == DT_REG) {
				if (strstr(entry->d_name, word_to_find) != NULL) {
					syslog(LOG_INFO, "found file: %s\n", path);
				}
			}
		}
		closedir(dir);
		free(path);
	}
}


void search_wrapper(int offset){
	char* word_to_find = *(glargv + optind + offset);
	if(verbose>2)
		syslog(LOG_INFO, "started searching for: %s\n", (word_to_find));
	search_rec_root(word_to_find, "/");
}
