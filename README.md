# Program do obsługi wyrażeń regularnych

## Opis

Program `regex_handler.c` jest prostym narzędziem do kompilacji i sprawdzania dopasowania wyrażeń regularnych w ciągach znaków. Ponadto zawiera funkcje do filtrowania plików na podstawie wyrażeń regularnych i innych przydatnych operacji związanych z wyrażeniami regularnymi.

## Funkcje

1. **Kompilacja wyrażenia regularnego**
   - Funkcja `compile_regex` kompiluje podane wyrażenie regularne i zwraca strukturę `regex_t`.

2. **Sprawdzanie dopasowania**
   - Funkcja `match_regex` sprawdza, czy podany ciąg znaków pasuje do skompilowanego wyrażenia regularnego.

3. **Filtrowanie plików na podstawie wzorca**
   - Funkcja `list_files_matching_pattern` przeszukuje określony katalog w poszukiwaniu plików, których nazwy pasują do określonego wzorca wyrażenia regularnego.

4. **Sprawdzanie wszystkich dopasowań**
   - Funkcja `get_all_matches` sprawdza wszystkie dopasowania ciągu znaków do skompilowanego wyrażenia regularnego.

5. **Walidacja wzorca**
   - Funkcja `validate_regex_pattern` sprawdza poprawność składniową podanego wzorca wyrażenia regularnego.

## Instrukcje użytkowania

1. Skompiluj program za pomocą odpowiedniego kompilatora dla języka C, np. `gcc regex_handler.c -o regex_handler -std=c11 -Wall`.
2. Uruchom skompilowany program, wprowadzając odpowiednie argumenty w wierszu poleceń.
3. Wykorzystaj dostępne funkcje w zależności od potrzeb.

## Przykład użycia

```bash
./regex_handler -v "abc.*" "test_directory"
```

W tym przykładzie program przeszuka pliki w katalogu "test_directory", sprawdzając, czy ich nazwy pasują do wyrażenia regularnego "abc.*". Opcja `-v` włącza tryb szczegółowych komunikatów.

