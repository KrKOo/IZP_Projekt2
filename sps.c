#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

int cell_ctor(Cell *cell)
{
    cell->value = NULL;
    cell->length = 0;
    cell->allocated = 0;

    return 0;
}

int cell_dtor(Cell *cell)
{
    free(cell->value);
    cell->length = 0;
    cell->allocated = 0;

    return 0;
}

int row_ctor(Row *row)
{
    row->cells = NULL;
    row->length = 0;
    row->allocated = 0;

    return 0;
}

int row_dtor(Row *row)
{
    free(row->cells);
    row->length = 0;
    row->allocated = 0;

    return 0;
}

int table_ctor(Table *table)
{

    table->rows = NULL;    
    table->allocated = 0;
    table->length = 0;

    return 0;
}

int table_dtor(Table *table)
{
    for(int i = 0; i < table->allocated; i++)
    {
        for(int j = 0; j < table->rows[i].allocated; j++)
        {
            free(table->rows[i].cells[j].value);
        }
        free(table->rows[i].cells);
    }
    free(table->rows);
    return 0;
}

int addEmptyRow(Table *table)
{
    table->rows = realloc(table->rows, (table->allocated + 1) * sizeof(Row));
    table->allocated++;    
    row_ctor(&table->rows[table->length]);
    table->length++;
    return 0;
}

int allocateCellsToRow(Row *row)
{
    row->cells = realloc(row->cells, (row->allocated + CELL_ALLOC_SIZE) * sizeof(Cell));
    row->allocated += CELL_ALLOC_SIZE;

    return 0;
}

int addCellToRow(Row *row, char *value)
{
    if(row->length == row->allocated)
        allocateCellsToRow(row);

    int valueLength = strlen(value);

    row->cells[row->length].value = realloc(row->cells[row->length].value, (valueLength + 1) * sizeof(char));
    row->cells[row->length].length = valueLength + 1;
    row->cells[row->length].allocated = valueLength + 1;

    strcpy(row->cells[row->length].value, value);
    row->length++;

    return 0;
}

int appendCharToCell(Cell *cell, char c)
{
    if(cell->length == cell->allocated)
    {
        cell->value = realloc(cell->value, (cell->allocated + CELL_ALLOC_SIZE) * sizeof(char));
        cell->allocated += CELL_ALLOC_SIZE;
    }
    
    cell->value[cell->length] = c;
    cell->length++;

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

bool isCharInString(char c, char *str)
{
    for(int i = 0; str[i] != '\0'; i++)
    {
        if(c == str[i])
            return true;
    }

    return false;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    //Delim validation

    char *delim = " ";
    if (argc > 2)
        if (strcmp(argv[1], "-d") == 0)
            delim = argv[2];

    if (isValidDelim(delim) == 0)
    {
        //ERROR
        return 1;
    }

    Table table;

    table_ctor(&table);

    Cell cell;
    cell_ctor(&cell);

    FILE *inputFile = fopen("tab.txt", "r");
    char c;
    int rowID = 0;
    int colID = 0;
    addEmptyRow(&table);
    while ((c = fgetc(inputFile)) != EOF)
    {
        
        if(isCharInString(c, delim))
        {
            appendCharToCell(&cell, '\0');
            addCellToRow(&table.rows[rowID], cell.value);
            cell_dtor(&cell);
            cell_ctor(&cell);
            colID++;
        }
        else if(c == '\n')
        {
            appendCharToCell(&cell, '\0');
            addCellToRow(&table.rows[rowID], cell.value);
            cell_dtor(&cell);
            cell_ctor(&cell);
            addEmptyRow(&table);
            rowID++;
            colID=0;
        }
        else
        {
            appendCharToCell(&cell, c);
        }
    }

    fclose(inputFile);

    for(int i = 0; i < table.length - 1; i++)
    {
        for(int j = 0; j < table.rows->length; j++)
        {
            printf("%s%c", table.rows[i].cells[j].value, delim[0]);
        }
        putchar('\n');
    }

    table_dtor(&table);

    return 0;
}