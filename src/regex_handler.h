#ifndef REGEX_HANDLER_H
#define REGEX_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#define MAX_MATCHES 10 // Maximum number of matches

/** @brief Compiles a regular expression pattern.
 *
 *  This function compiles the given regular expression pattern and returns a regex_t structure.
 *  @param pattern The regular expression pattern to compile.
 *  @return A regex_t structure representing the compiled regular expression.
 */
regex_t compile_regex(const char *pattern);

/** @brief Checks if a string matches a regular expression.
 *
 *  This function checks if the given string matches the compiled regular expression.
 *  @param regex The compiled regular expression.
 *  @param str The string to check.
 *  @return 0 if the string matches the regular expression, 1 otherwise.
 */
int match_regex(const regex_t *regex, const char *str);

/** @brief Validates a regular expression pattern.
 *
 *  This function validates the syntactic correctness of the provided regular expression pattern.
 *  @param pattern The regular expression pattern to validate.
 *  @return 1 if the pattern is valid, 0 otherwise.
 */
int validate_regex_pattern(const char *pattern);

/** @brief Wrapper function to convert SQL-style regular expression.
 *
 *  This function converts the given regular expression pattern from SQL style to the desired format.
 *  @param sql_regex The SQL-style regular expression pattern.
 *  @return The converted regular expression pattern.
 */
char *convert_sql_regex(const char *sql_regex);

#endif /* REGEX_HANDLER_H */
