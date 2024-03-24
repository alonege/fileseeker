# Dokumentacja
## Ogólnie - funkcje
Aby ułatwić dokumentowanie oprogramowania, należy przyjąć styl komentarzy zgodny z doxygen, tj. dla komentarzy blokowych:
```c
/** @brief Krótki komentarz
*
* Długi komentarz - omówienie funkcji
* druga linia
* i trzecia
*  @param a tutaj krótki komentarz o wymaganiach a, czym ewentualnie jest etc
*  @param width tutaj czym jest width etc
*  @param count tutaj czym jest width etc
*  @return Tutaj opisujemy jaki będzie output
*/
int example(void* a, size_t width, size_t count){
    //kod...
}
```
na przykład:
```c
/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *  @return Void.
 */
void draw_char(int row, int col, int ch, int color);
```
## Komentarze plikowe
W przypadku komentarzy dot. plików:
```c
/** @file plik.c
 *  @brief A console driver.
 *
 *  These empty function definitions are provided
 *  so that stdio will build without complaining.
 *  You will need to fill these functions in. This
 *  is the implementation of the console driver.
 *  Important details about its implementation
 *  should go in these comments.
 *
 *  @author Fred Hacker (fhacker)
 */

/* -- Includes -- */

/* libc includes. */
#include <stdio.h>        /* for lprintf_kern() */

/* multiboot header file */
#include <multiboot.h>    /* for boot_info */

/*
 * state for kernel memory allocation.
 */
extern lmm_t malloc_lmm;
```
więcej na (aka ukradzione między innymi z :3) [link](https://www.cs.cmu.edu/~410/doc/doxygen.html)
# ANSI C - wskaźniki
## Definiowanie
jeśli definiujemy wskaźnik, to w tej samej linii co definicja jeśli coś do niego przypisujemy, to adres, np:
```c
int *a = malloc(sizeof(int));
```
jest równoważne
```c
int *a;
a = malloc(sizeof(int));
```
## Arytmetyka wskaźników
Przypuśćmy, że mamy tablicę A intów, przy czym została już zalokowana, wypełniona tak że index 0 ma 0, index 1 ma 1 etc. Niech będzie 100 elementów. Wtedy
```c
int *ptr = A;
printf("%d\n", *ptr);
ptr++; //ptr=1;
printf("%d\n", *ptr);
ptr+=2; //ptr=3
printf("%d\n", *ptr);
```
wypisze nam kolejno w nowych liniach 0, 1 i 3. Należy uważać, by nie wyskoczyć poza index 99 (błąd typu buffer overflow, undefined!)
## Odwoływanie do elementów wskaźników
Jeśli mamy element w structie, do którego mamy wskaźnik, to do elementu struktury możemy odowływać się z pomocą albo (*a).val, albo a->val.
```c
typedef struct el{
    int val;
    struct el* next;
} el, *elPtr;

int main(){
    el* ll = malloc(sizeof(el));
    ll->val = 5;
    ll->next = malloc(sizeof(el));
    ll->next->next=0;
    return 0;
}
```
# Użyte funkcje biblioteki linuxa
Do rozwinięcia...
