#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROW_ALLOC_SIZE 5
#define CELL_ALLOC_SIZE 5

typedef struct {
    char *value;
    int length;
    int allocated;
} Cell;

typedef struct {
    Cell *cells;
    int length;
    int allocated;
} Row;

typedef struct {
    Row *rows;
    int length;
    int allocated;
} Table;

int table_ctor(Table *table)
{
    table->rows = malloc(sizeof(Row));
    table->length = 0;
    table->allocated = 1;

    table->rows->cells = malloc(sizeof(Cell));
    table->rows->length = 0;
    table->rows->allocated = 1;

    table->rows->cells->value = NULL;
    table->rows->cells->length = 0;
    table->rows->cells->allocated = 0;

    return 0;
}

int addRows(Table *table, int n)
{
    table->rows = realloc(table->rows, (table->allocated + n) * sizeof(Row));
    table->allocated += n;

    return 0;
}

int removeRows(Table *table, int n)
{
    if(table->allocated - n > 0)
    {
        table->rows = realloc(table->rows, (table->allocated - n) * sizeof(Row));
        table->allocated -= n;
    }

    return 0;
}

int addCells(Row *row, int n)
{
    row->cells = realloc(row->cells, (row->allocated + n) * sizeof(Row));
    row->allocated += n;

    return 0;
}

int removeCells(Row *row, size_t n)
{
    if(row->allocated - n > 0)
    {
        row->cells = realloc(row->cells, (row->allocated - n) * sizeof(Row));
        row->allocated -= n;
    }
    else
    {
        //ERROR
    }   

    return 0;
}

int freeTable(Table *table)
{
    for(int i = 0; i < table->allocated; i++)
    {
        for(int j = 0; j < table->rows->allocated; j++)
        {
            free(table->rows[i].cells[j].value);
        }
        free(table->rows[i].cells);
        free(table->rows);
    }
    return 0;
}

int isValidDelim(char *delim)
{
    if (strchr(delim, '"') != NULL)
        return 0;
    if (strchr(delim, '\\') != NULL)
        return 0;

    return 1;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    //Delim validation

    char *delim = " ";
    if (argc > 2)
        if (strcmp(argv[1], "-d"))
            delim = argv[2];

    if (isValidDelim(delim) == 0)
    {
        //ERROR
        return 1;
    }

    Table table;
    table_ctor(&table);

    freeTable(&table);

    FILE *inputFile = fopen("tab.txt", "r");
    char c;
    while ((c = fgetc(inputFile)) != EOF)
    {
        printf("%c", c);
    }

    return 0;
}