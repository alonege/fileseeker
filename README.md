# Dokumentacja
## PL
### Koncept i funkcjonalności
Program otrzymuje listę argumentów, z których każde jest fragmentem nazwy pliku. Program staje się demonem. Uruchamia dzieci (o których można przeczytać poniżej) i przeradza się w proces nadzorczy (śpi w oczekiwaniu na sygnały lub na zakończenie któregoś z procesów przeszukujących). Wysłanie procesowi nadzorczemu sygnału SIGUSR1 albo SIGUSR2 powoduje jego przekazania wszystkim procesom potomnym.


Co kilkadziesiąt sekund (domyślny czas można zmienić za pomocą opcjonalnego parametru wiersza poleceń `-t czas`), rekurencyjnie skanuje system plików (`/`) poszukując plików lub katalogów w których występuje zadany fragment nazwy. Pliki, do których demon nie ma praw dostępu są pomijane. Również takie katalogi są pomijane, dodatkowo katalog bez prawa odczytu i wykonania nie jest brany pod uwagę na etapie przeszukania rekurencyjnego. Nazwy odnalezionych plików lub katalogów są przekazywane zapisywane do logu systemowego (syslog). Umieszczone są w nim następujące informacje: pełna data, pełna ścieżka pliku, poszukiwany wzorzec. Przeszukanie jest prowadzone współbieżnie przez liczbę procesów równą liczbie argumentów.
Odebranie sygnału SIGUSR1 powoduje rozpoczęcie skanowania, jeśli proces śpi. Odebranie SIGUSR1, w chwili gdy trwa przeszukanie powoduje natychmiastowy restart przeszukiwania. Odebranie SIGUSR2, w chwili gdy trwa przeszukanie powoduje natychmiastowe zakończenie przeszukiwania i ponowne uśpienie demona.

Dodatkowa opcja -v powoduje przesyłanie do logu wyczerpującej informacji o każdej z następujących czynności demona: a) uśpienie, b) obudzenie się c) odbiór sygnału d) porównanie nazwy pliku ze ścieżką.

### Dodatkowe (własne) ulepszenia
Proces będzie wskrzeszał dzieci zabite sygnałem SIGKILL. Zastosowano dodatkowo kilka stopni logowania (-verbose) - dokładniej od 0 do 3.

## Documentation
### Concept and functionalities
The program receives a list of arguments, each of which is a fragment of a file name. The program becomes a daemon. It launches children (which you can read about below) and transforms into a supervisory process (sleeps waiting for signals or the end of one of the searching processes). Sending the supervisory process a SIGUSR1 or SIGUSR2 signal causes it to pass them to all child processes.

Every few dozen seconds (the default time can be changed using the optional command-line parameter `-t time`), it recursively scans the file system (`/`) looking for files or directories in which the specified name fragment appears. Files to which the daemon does not have access rights are skipped. Also, such directories are skipped, additionally, a directory without read and execute rights is not taken into account at the stage of recursive searching. The names of the found files or directories are written to the system log (syslog). The following information is included in it: full date, full file path, searched pattern. The search is conducted concurrently by a number of processes equal to the number of arguments.
Receiving a SIGUSR1 signal starts scanning if the process is asleep. Receiving a SIGUSR1 while searching is in progress causes an immediate restart of the search. Receiving a SIGUSR2 while searching is in progress causes an immediate end to the search and puts the daemon back to sleep.

An additional -v option causes exhaustive information about each of the following daemon activities to be sent to the log: a) falling asleep, b) waking up c) receiving a signal d) comparing the file name with the path.

### Additional (own) enhancements
The process will resurrect children killed by the SIGKILL signal. Additionally, several logging levels (-verbose) have been implemented - specifically from 0 to 3.
