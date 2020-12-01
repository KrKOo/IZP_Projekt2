#include <stdio.h>
#include <stdlib.h>

// typedef struct {

// } Cell;

// typedef struct {

// } Row;

// typedef struct {

// } Table;

int isValidDelim(char *delim)
{
    if(strchr(delim, '"') != NULL)
        return 0;
    if(strchr(delim, '\\') != NULL)
        return 0;
    
    return 1;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    //Delim validation

    char *delim = NULL;
    if(argc > 1 && strcmp(argv[1], "-d"))
        delim = argv[2];
    
    if(isValidDelim(delim) == 0)
    {
        //ERROR
        return 1;
    }

    FILE *inputFile = fopen("tab.txt", "r");
    char c;
    while((c = fgetc(inputFile)) != EOF)
    {
        printf("%c", c);
    }



    return 0;
}