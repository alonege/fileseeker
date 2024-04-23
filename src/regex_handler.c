#include "regex_handler.h"

regex_t compile_regex(const char *pattern) {
    regex_t regex;
    int ret;

    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        char errbuf[100];
        regerror(ret, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regular expression compilation failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    return regex;
}

int match_regex(const regex_t *regex, const char *str) {
    int ret = regexec(regex, str, 0, NULL, 0);
    if (ret == 0) {
        return 0; // Match found
    } else if (ret == REG_NOMATCH) {
        return 1; // No match
    } else {
        char errbuf[100];
        regerror(ret, regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regular expression matching failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }
}

int validate_regex_pattern(const char *pattern) {
    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    regfree(&regex);
    return ret == 0;
}

char *convert_sql_regex(const char *sql_regex) {
    // Placeholder implementation for SQL to desired format regex conversion
    // Here, you should place your actual conversion logic

    // Example conversion: SQL wildcard % to regex .*
    const char wildcard_char = '%';
    const char regex_wildcard[] = ".*";

    char *converted_regex = malloc(strlen(sql_regex) * 2 + 1); // Allocate memory for converted regex

    if (converted_regex == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    char *ptr_converted = converted_regex;
    const char *ptr_sql = sql_regex;

    while (*ptr_sql != '\0') {
        if (*ptr_sql == wildcard_char) {
            strcpy(ptr_converted, regex_wildcard);
            ptr_converted += strlen(regex_wildcard);
        } else {
            *ptr_converted = *ptr_sql;
            ptr_converted++;
        }
        ptr_sql++;
    }

    *ptr_converted = '\0'; // Null-terminate the converted regex string

    return converted_regex;
}
