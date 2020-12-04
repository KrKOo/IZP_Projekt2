#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

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

typedef struct {
    int fromRow;
    int toRow;
    int fromCol;
    int toCol;
} Selection;


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

void selection_ctor(Selection *selection)
{
    selection->fromRow = 0;
    selection->toRow = 0;
    selection->fromCol = 0;
    selection->toCol = 0;
}

void setSelection(Selection *selection, int fromRow, int toRow, int fromCol, int toCol)
{
    selection->fromRow = fromRow;
    selection->toRow = toRow;
    selection->fromCol = fromCol;
    selection->toCol = toCol;
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

int dcol(Table *table, int colID)
{
    for(int i = 0; i < table->length; i++)
    {
        cell_dtor(&table->rows[i].cells[colID-1]);
        memmove(&table->rows[i].cells[colID - 1], &table->rows[i].cells[colID], (table->rows[i].allocated - (colID)) * sizeof(Cell));

        table->rows[i].cells = realloc(table->rows[i].cells, (table->rows[i].allocated - 1) * sizeof(Cell));
        table->rows[i].allocated--;
        table->rows[i].length--;

    }

    return 0;
}

int alignTable(Table *table, int colCount)
{
    for(int i = 0; i < table->length; i++)
    {
        if(table->rows[i].length < colCount)
        {
            allocateCellsToRow(&table->rows[i], colCount - table->rows[i].length);
            table->rows[i].length = colCount;
        }
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
        {
            if(j < table->rows[i].length-1)
            {
                printf("%s%c", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "", delim);
            }
            else
            {
                printf("%s", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "");
            }
        }
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
            colID++;
            if(colID > maxColID)
                maxColID = colID;
            colID = 0;
        }
        else
        {
            appendCharToCell(&cell, c);
        }
    }
    fclose(inputFile);

    return maxColID;
}

int findMaxNumber(Table *table, Selection *selection)
{
    int max = INT_MIN;
    int rowID, colID;
    for(rowID = 0; rowID < table->length; rowID++)
    {
        for(colID = 0; colID < table->rows[rowID].length; colID++)
        {
            char *endPtr = NULL;
            if(table->rows[rowID].cells[colID].value != NULL)
            {
                int number = strtol(table->rows[rowID].cells[colID].value, &endPtr, 10);
                if(number > max)
                {
                    max = number;
                    setSelection(selection, rowID, rowID, colID, colID);
                }
            }
        }
    }
    return 1;
}

int findMinNumber(Table *table, Selection *selection)
{
    int min = INT_MAX;
    int rowID, colID;
    for(rowID = 0; rowID < table->length; rowID++)
    {
        for(colID = 0; colID < table->rows[rowID].length; colID++)
        {
            char *endPtr = NULL;
            if(table->rows[rowID].cells[colID].value != NULL)
            {
                char *cellValue = table->rows[rowID].cells[colID].value;
                int number = strtol(cellValue, &endPtr, 10);
                
                if(min > number && *endPtr == '\0' && endPtr != cellValue)
                {
                    min = number;
                    setSelection(selection, rowID, rowID, colID, colID);
                }
            }
        }
    }
    return 1;
}

int findString(Table *table, Selection *selection, char *str)
{
    int rowID, colID;
    for(rowID = 0; rowID < table->length; rowID++)
    {
        for(colID = 0; colID < table->rows[rowID].length; colID++)
        {
            if(strcmp(table->rows[rowID].cells[colID].value, str) == 0)
            {
                setSelection(selection, rowID, rowID, colID, colID);
                return 1;
            }
        }
    }

    return 0;
}

int commandExecutor(Table *table, char *cmd, Selection *selection)
{
    if(strcmp(cmd, "irow") == 0)
    {
        irow(table, selection->fromRow);
    }

    (void) selection;

    return 1;
}

int parseSelection(Table *table, char *selectionStr, Selection *selection, Selection *tempSelection)
{
    char *selectionStrCopy = malloc((strlen(selectionStr) + 1) * sizeof(char));
    strcpy(selectionStrCopy, selectionStr);

    bool isFind = false;
    char *lastPtr = &selectionStrCopy[1];
    int arrIndex = 0;
    for(int i = 1; selectionStrCopy[i] != '\0'; i++)
    {
        if(selectionStrCopy[i] == ',' || selectionStrCopy[i] == ']' || selectionStrCopy[i] == ' ')
        {
            bool end = false;
            if(selectionStrCopy[i] == ']')
                end = true;

            selectionStrCopy[i] = '\0';
            char *endPtr = NULL;
            int value = strtol(lastPtr, &endPtr, 10);
            if(strcmp(lastPtr, "min") == 0)
            {
                findMinNumber(table, selection);
                break;
            }
            else if(strcmp(lastPtr, "max") == 0)
            {
                findMaxNumber(table, selection);
                break;
            }
            else if(strcmp(lastPtr, "find") == 0)
            {
                isFind = true;
            }
            else if(isFind)
            {
                printf("%s", lastPtr);
                findString(table, selection, lastPtr);
                break;
            }
            else if(strcmp(lastPtr, "_") == 0)
            {
                if(arrIndex == 0 && end)
                {
                    selection = tempSelection;
                    break;
                }
                if(arrIndex == 0)
                {
                    selection->fromRow = 0;
                    selection->toRow = -1;
                }
                else if(arrIndex == 1)
                {
                    selection->fromCol = 0;
                    selection->toCol = -1;
                }     
                else
                {
                    //ERROR
                }         
            }
            else if(arrIndex == 0)
            {
                selection->fromRow = value;
                selection->toRow = value;
            }
            else if(arrIndex == 1)
            {
                selection->fromCol = value;
                selection->toCol = value;
            }
            else if(arrIndex == 2)
                selection->toRow = value;
            else if(arrIndex == 3)
                selection->toCol = value;

            arrIndex++;
            lastPtr = &selectionStrCopy[i+1];
        }
    }

    printf("fromRow: %d\n", selection->fromRow);
    printf("toRow: %d\n", selection->toRow);
    printf("fromCol: %d\n", selection->fromCol);
    printf("toCol: %d\n", selection->toCol);

    free(selectionStrCopy);
    return 1;
}

int parseCommands(Table *table, char *cmdSequence, Selection *tempSelection)
{
    char *cmd;
    Selection selection;
    cmd = strtok(cmdSequence, ";");
    while(cmd != NULL)
    {
        if(cmd[0] == '[')
        {
            parseSelection(table, cmd, &selection, tempSelection);
        }
        printf("%s\n", cmd);
        cmd = strtok(NULL, ";");
    }

    return 0;
}

int main(int argc, char **argv)
{
    char *delim = " ";
    char *cmdSequence = "";
    char *fileName = "";
    for(int i = 0; i < argc; i++)
    {
        char* asdf = argv[i];
        (void) asdf;
    }
    if (argc > 2)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            delim = argv[2];
            if (argc > 3)
            {
                cmdSequence = argv[3];
                fileName = argv[4];
            }
            else
            {
                //ERROR
            }
            
        }
        else
        {
            cmdSequence = argv[1];
            fileName = argv[2]; 
        }
    }
    else
    {
        //ERROR
    }

    if (isValidDelim(delim) == 0)
    {
        //ERROR
        return 1;
    }

    

    Table table;
    table_ctor(&table);
    Selection tempSelection;
    selection_ctor(&tempSelection);
            
    int colCount = loadTableFromFile(&table, fileName, delim);

    //setCellValue(&table.rows[1].cells[2], "34q5");

    //drow(&table, 2);
    alignTable(&table, colCount);
    parseCommands(&table, cmdSequence, &tempSelection);
    //dcol(&table, 3);

    //printTable(&table, delim[0]);

    table_dtor(&table);

    return 0;
}