# Program do obsługi wyrażeń regularnych

## Opis

Program `regex_handler.c` jest prostym narzędziem do kompilacji i sprawdzania dopasowania wyrażeń regularnych w ciągach znaków. Ponadto zawiera funkcje do filtrowania plików na podstawie wyrażeń regularnych i innych przydatnych operacji związanych z wyrażeniami regularnymi.

## Funkcje

1. **Kompilacja wyrażenia regularnego**
   - Funkcja `compile_regex` kompiluje podane wyrażenie regularne i zwraca strukturę `regex_t`.

   ```c
    /** @brief Kompiluje wyrażenie regularne.
    *
    *  Funkcja kompiluje podane wyrażenie regularne i zwraca strukturę regex_t.
    *  @param pattern Wzorzec wyrażenia regularnego do skompilowania.
    *  @return Struktura regex_t reprezentująca skompilowane wyrażenie regularne.
    */

    regex_t compile_regex(const char *pattern) {
        regex_t regex;
        int ret;

        ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
        if (ret != 0) {
            char errbuf[100];
            regerror(ret, &regex, errbuf, sizeof(errbuf));
            fprintf(stderr, "Kompilacja wyrażenia regularnego nie powiodła się: %s\n", errbuf);
            exit(EXIT_FAILURE);
        }

        return regex;
    }

2. **Sprawdzanie dopasowania**
   - Funkcja `match_regex` sprawdza, czy podany ciąg znaków pasuje do skompilowanego wyrażenia regularnego.

    ```c
    /** @brief Sprawdza dopasowanie ciągu do wyrażenia regularnego.
    *
    *  Funkcja sprawdza, czy podany ciąg znaków pasuje do skompilowanego wyrażenia regularnego.
    *  @param regex Skompilowane wyrażenie regularne.
    *  @param str Ciąg znaków do sprawdzenia.
    *  @return true jeśli ciąg znaków pasuje do wyrażenia regularnego, false w przeciwnym razie.
    */

    bool match_regex(const regex_t *regex, const char *str) {
        int ret = regexec(regex, str, 0, NULL, 0);
        if (ret == 0) {
            return true; // Dopasowanie znalezione
        } else if (ret == REG_NOMATCH) {
            return false; // Brak dopasowania
        } else {
            char errbuf[100];
            regerror(ret, regex, errbuf, sizeof(errbuf));
            fprintf(stderr, "Dopasowanie wyrażenia regularnego nie powiodło się: %s\n", errbuf);
            exit(EXIT_FAILURE);
        }
    }
3. **Filtrowanie plików na podstawie wzorca**
   - Funkcja `list_files_matching_pattern` przeszukuje określony katalog w poszukiwaniu plików, których nazwy pasują do określonego wzorca wyrażenia regularnego.

     ```c
        /** @brief Funkcja zwraca listę plików w katalogu pasujących do wzorca.
        *
        *  Funkcja przeszukuje określony katalog w poszukiwaniu plików, których nazwy pasują do podanego wzorca.
        *  @param directory Ścieżka do katalogu do przeszukania.
        *  @param regex Skompilowane wyrażenie regularne do dopasowania nazw plików.
        *  @return Tablica ciągów znaków zawierająca nazwy pasujących plików.
        */

        char **list_files_matching_pattern(const char *directory, const regex_t *regex) {
            DIR *dir;
            struct dirent *entry;
            char **files = NULL;
            int file_count = 0;

            dir = opendir(directory);
            if (dir == NULL) {
                perror("Błąd przy otwieraniu katalogu");
                exit(EXIT_FAILURE);
            }

            // Przeszukiwanie katalogu
            while ((entry = readdir(dir)) != NULL) {
                if (match_regex(regex, entry->d_name)) {
                    // Jeśli nazwa pliku pasuje do wzorca, dodaj ją do tablicy plików
                    files = realloc(files, (file_count + 1) * sizeof(char *));
                    files[file_count] = strdup(entry->d_name);
                    file_count++;
                }
            }

            closedir(dir);
            return files;
        }
4. **Sprawdzanie wszystkich dopasowań**
   - Funkcja `get_all_matches` sprawdza wszystkie dopasowania ciągu znaków do skompilowanego wyrażenia regularnego.

     ```c
        /** @brief Sprawdza wszystkie dopasowania ciągu do wyrażenia regularnego.
        *
        *  Funkcja sprawdza wszystkie dopasowania ciągu znaków do skompilowanego wyrażenia regularnego.
        *  @param regex Skompilowane wyrażenie regularne.
        *  @param str Ciąg znaków do sprawdzenia.
        *  @return Tablica ciągów znaków zawierająca wszystkie dopasowania.
        */
        char **get_all_matches(const regex_t *regex, const char *str) {
            regmatch_t matches[MAX_MATCHES];
            int ret;
            char **matches_strings = NULL;
            int match_count = 0;

            // Wyszukuje wszystkie dopasowania
            while ((ret = regexec(regex, str, MAX_MATCHES, matches, 0)) == 0) {
                // Dopasowanie znalezione
                for (int i = 0; i < MAX_MATCHES; i++) {
                    if (matches[i].rm_so == -1) break; // Koniec dopasowań
                    int start = matches[i].rm_so;
                    int end = matches[i].rm_eo;
                    int length = end - start;

                    // Wydziel dopasowany ciąg
                    char *match_string = malloc((length + 1) * sizeof(char));
                    strncpy(match_string, str + start, length);
                    match_string[length] = '\0';

                    // Dodaj dopasowanie do tablicy
                    matches_strings = realloc(matches_strings, (match_count + 1) * sizeof(char *));
                    matches_strings[match_count] = match_string;
                    match_count++;
                }
                // Przesuń wskaźnik na kolejne dopasowanie
                str += matches[0].rm_eo;
            }

            if (ret != REG_NOMATCH) {
                char errbuf[100];
                regerror(ret, regex, errbuf, sizeof(errbuf));
                fprintf(stderr, "Błąd podczas wyszukiwania dopasowań: %s\n", errbuf);
                exit(EXIT_FAILURE);
            }

            return matches_strings;
        }
5. **Walidacja wzorca**
   - Funkcja `validate_regex_pattern` sprawdza poprawność składniową podanego wzorca wyrażenia regularnego.

     ```c
        /** @brief Waliduje wzorzec wyrażenia regularnego.
        *
        *  Funkcja sprawdza poprawność składniową podanego wzorca wyrażenia regularnego.
        *  @param pattern Wzorzec wyrażenia regularnego do zweryfikowania.
        *  @return true jeśli wzorzec jest poprawny, false w przeciwnym razie.
        */
        bool validate_regex_pattern(const char *pattern) {
            regex_t regex;
            int ret = regcomp(&regex, pattern, REG_EXTENDED);
            regfree(&regex);
            return ret == 0;
        }

        int main() {
            const char *pattern = "abc.*"; // Przykładowy wzorzec wyrażenia regularnego
            regex_t regex = compile_regex(pattern);

            const char *test_string = "abcdef"; // Przykładowy ciąg znaków
        }


## Instrukcje użytkowania

1. Skompiluj program za pomocą odpowiedniego kompilatora dla języka C, np. `gcc regex_handler.c -o regex_handler -std=c11 -Wall`.
2. Uruchom skompilowany program, wprowadzając odpowiednie argumenty w wierszu poleceń.
3. Wykorzystaj dostępne funkcje w zależności od potrzeb.

## Przykład użycia

```bash
./regex_handler -v "abc.*" "test_directory"
```

W tym przykładzie program przeszuka pliki w katalogu "test_directory", sprawdzając, czy ich nazwy pasują do wyrażenia regularnego "abc.*". Opcja `-v` włącza tryb szczegółowych komunikatów.

