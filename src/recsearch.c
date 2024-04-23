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

		while ((entry = readdir(dir)) != NULL) {
		        //char path[1024];
		        snprintf(path, MAX_PATH_LEN, "%s/%s", root_path, entry->d_name);

		        if (entry->d_type == DT_DIR) {
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;
				if (strstr(entry->d_name, word_to_find) != NULL) {
					printf("found directory: %s\n", path);
				}
				search_rec(word_to_find, path);
			} else if (entry->d_type == DT_REG) {
				if (strstr(entry->d_name, word_to_find) != NULL) {
				printf("found file: %s\n", path);
				}
			}
		}
		closedir(dir);
		free(path);
	}
}


void searchwrapper(int offset){
	char* word_to_find = *(glargv + optind + offset);
	search_rec(word_to_find, ".");
}
