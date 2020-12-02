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

int allocateRowsToTable(Table *table, int n)
{
    table->rows = realloc(table->rows, (table->allocated + n) * sizeof(Row));
    for(int i = table->allocated; i < table->allocated + n; i++)
        row_ctor(&table->rows[i]);

    table->allocated += n;    
    return 0;
}

int allocateCellsToRow(Row *row, int n)
{
    row->cells = realloc(row->cells, (row->allocated + n) * sizeof(Cell));
    for(int i = row->allocated; i < row->allocated + n; i++)
        cell_ctor(&row->cells[i]);

    row->allocated += n;
    return 0;
}

int addCellToRow(Table *table, int rowID, char *value)
{
    if(table->length == table->allocated)
    {
        allocateRowsToTable(table, (rowID+1) - table->allocated);
        table->length = rowID + 1;
    }
        

    Row *row = &table->rows[rowID];
    
    if(row->length == row->allocated)
        allocateCellsToRow(row, CELL_ALLOC_SIZE);

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

int setCellValue(Cell *cell, char *value)
{
    int valueSize = strlen(value);

    cell->value = realloc(cell->value, valueSize + 1);
    cell->allocated = valueSize + 1;
    strcpy(cell->value, value);
    cell->length = valueSize + 1;

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

int drow(Table *table, int rowID)
{
    for(int i = 0; i < table->rows[rowID - 1].allocated; i++)
    {
        cell_dtor(&table->rows[rowID - 1].cells[i]);
    }
    row_dtor(&table->rows[rowID - 1]);

    memmove(&table->rows[rowID-1], &table->rows[rowID], (table->allocated - (rowID)) * sizeof(Row));
    
    table->rows = realloc(table->rows, (table->allocated - 1) * sizeof(Row));
    table->allocated--;
    table->length--;

    return 0;
}

int irow(Table *table, int rowID)
{
    if(table->length == table->allocated)
    {
        allocateRowsToTable(table, 1);
    }

    memmove(&table->rows[rowID], &table->rows[rowID-1], (table->allocated - rowID) * sizeof(Row));
    row_ctor(&table->rows[rowID-1]);
    table->length++;

    return 0;
}

int icol(Table *table, int colID)
{
    for(int i = 0; i < table->length; i++)
    {
        if(table->rows[i].length == table->rows[i].allocated)
        {
            allocateCellsToRow(&table->rows[i], 1);
        }

        memmove(&table->rows[i].cells[colID], &table->rows[i].cells[colID - 1], (table->rows[i].allocated - colID) * sizeof(Cell));
        cell_ctor(&table->rows[i].cells[colID-1]);
        table->rows[i].length++;
    }
    return 0;
}

void printTable(Table *table, char delim)
{
    //printf("%d", table->length);
    for(int i = 0; i < table->length; i++)
    {
        //printf("Length: %d Allocated: %d\n", table->rows[i].length, table->rows[i].allocated);
        for(int j = 0; j < table->rows[i].length; j++)
            printf("%s%c", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "", delim);
        putchar('\n');
    }
}

int loadTableFromFile(Table *table, char *filename, char *delim)
{
    Cell cell;
    cell_ctor(&cell);

    FILE *inputFile = fopen(filename, "r");

    char c;
    int rowID = 0;
    int colID = 0;
    int maxColID = 0;
    while ((c = fgetc(inputFile)) != EOF)
    {
        if(isCharInString(c, delim))
        {
            appendCharToCell(&cell, '\0');
            addCellToRow(table, rowID, cell.value);
            cell_dtor(&cell);
            cell_ctor(&cell);
            colID++;
        }
        else if(c == '\n')
        {
            appendCharToCell(&cell, '\0');
            addCellToRow(table, rowID, cell.value);
            cell_dtor(&cell);
            cell_ctor(&cell);            
            rowID++;
            if(colID < maxColID)
                maxColID = colID;
            colID = 0;
        }
        else
        {
            appendCharToCell(&cell, c);
        }
    }
    //drow(table, rowID+1);
    fclose(inputFile);

<<<<<<< HEAD
    for(int i = 0; i < table.length; i++)
=======
    return maxColID;
}

int main(int argc, char **argv)
{
    // (void)argc;
    // (void)argv;

    //Delim validation

    char *delim = " ";
    if (argc > 2)
        if (strcmp(argv[1], "-d") == 0)
            delim = argv[2];

    if (isValidDelim(delim) == 0)
>>>>>>> 4bf12ab4d7f6f5e0f84df350d870cf4b965b7e4a
    {
        //ERROR
        return 1;
    }

    Table table;
    table_ctor(&table);
    int colCount = loadTableFromFile(&table, "tab.txt", delim);
    (void) colCount;

    //setCellValue(&table.rows[1].cells[2], "34q5");

    //drow(&table, 2);
    icol(&table, 5);

    printTable(&table, delim[0]);

    table_dtor(&table);

    return 0;
}