#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define ROW_ALLOC_SIZE 5
#define CELL_ALLOC_SIZE 5
#define COMMAND_COUNT 16
#define TEMP_VAR_COUNT 10

#define INVALID_FILE_OPERATION -1
#define INVALID_MEMORY_ALLOCATION 1
#define INVALID_DELIMITER 2
#define INVALID_COMMAND 3
#define INVALID_ARGUMENT_COUNT 4
#define INVALID_SELECTION 5
#define INVALID_COMMAND_ARGUMENT 6
#define INVALID_VARIABLE 7


typedef struct
{
    char *value;
    int length;
    int allocated;
} Cell;

typedef struct
{
    Cell *cells;
    int length;
    int allocated;
} Row;

typedef struct
{
    Row *rows;
    int length;
    int allocated;
} Table;

typedef struct
{
    int fromRow;
    int toRow;
    int fromCol;
    int toCol;
} Selection;

typedef struct
{
    int row;
    int col;
} Position;

typedef union
{
    char *inputString;
    Position position;
} CmdArgument;

int error(int errorCode)
{
    if(errorCode == INVALID_MEMORY_ALLOCATION)
        fprintf(stderr, "Memory could not be allocated!\n");
    else if(errorCode == INVALID_DELIMITER)
        fprintf(stderr, "Invalid delimiter specified!\n");
    else if(errorCode == INVALID_COMMAND)
        fprintf(stderr, "Invalid command specified!\n");
    else if(errorCode == INVALID_ARGUMENT_COUNT)
        fprintf(stderr, "Invalid argument count!\n");
    else if(errorCode == INVALID_SELECTION)
        fprintf(stderr, "Invalid selection!\n");
    else if(errorCode == INVALID_COMMAND_ARGUMENT)
        fprintf(stderr, "Invalid command argument!\n");
    else if(errorCode == INVALID_VARIABLE)
        fprintf(stderr, "Invalid variable specified!\n");
    else if(errorCode == INVALID_FILE_OPERATION)
        fprintf(stderr, "Invalid file!\n");

    return errorCode;    
}

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
    for (int i = 0; i < table->allocated; i++)
    {
        for (int j = 0; j < table->rows[i].allocated; j++)
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

//Set selection values
void setSelection(Selection *selection, int fromRow, int toRow, int fromCol, int toCol)
{
    selection->fromRow = fromRow;
    selection->toRow = toRow;
    selection->fromCol = fromCol;
    selection->toCol = toCol;
}

//Allocate <n> rows to table
int allocateRowsToTable(Table *table, int n)
{
    if((table->rows = realloc(table->rows, (table->allocated + n) * sizeof(Row))) == NULL)
        return INVALID_MEMORY_ALLOCATION;

    for (int i = table->allocated; i < table->allocated + n; i++)
        row_ctor(&table->rows[i]);

    table->allocated += n;
    return 0;
}

//Allocate <n> cells to row
int allocateCellsToRow(Row *row, int n)
{
    if((row->cells = realloc(row->cells, (row->allocated + n) * sizeof(Cell))) == NULL)
        return INVALID_MEMORY_ALLOCATION;

    for (int i = row->allocated; i < row->allocated + n; i++)
        cell_ctor(&row->cells[i]);

    row->allocated += n;
    return 0;
}

int setCellValue(Cell *cell, char *value)
{
    cell_dtor(cell);
    cell_ctor(cell);

    if (value == NULL)
        return 0;

    int valueSize = strlen(value);

    if((cell->value = realloc(cell->value, (valueSize + 1) * sizeof(char))) == NULL)
        return INVALID_MEMORY_ALLOCATION;

    strcpy(cell->value, value);

    cell->allocated = valueSize + 1;
    cell->length = valueSize + 1;

    return 0;
}

//Append one cell to row and set value
int addCellToRow(Table *table, int rowID, char *value)
{
    int retVal;
    if (table->length == table->allocated)
    {
        if((retVal = allocateRowsToTable(table, (rowID + 1) - table->allocated)) != 0)
        {
            return retVal;
        }

        table->length = rowID + 1;
    }

    Row *row = &table->rows[rowID];

    if (row->length == row->allocated)
        if((retVal = allocateCellsToRow(row, CELL_ALLOC_SIZE)) != 0)
            return retVal;
        
    setCellValue(&row->cells[row->length], value);
    // int valueLength = strlen(value);

    // if((row->cells[row->length].value = realloc(row->cells[row->length].value, (valueLength + 1) * sizeof(char))) == NULL)
    //     return INVALID_MEMORY_ALLOCATION;

    // row->cells[row->length].length = valueLength + 1;
    // row->cells[row->length].allocated = valueLength + 1;

    //strcpy(row->cells[row->length].value, value);
    row->length++;

    return 0;
}

//Append a single character to cell
int appendCharToCell(Cell *cell, char c)
{
    if (cell->length == cell->allocated)
    {
        if((cell->value = realloc(cell->value, (cell->allocated + CELL_ALLOC_SIZE) * sizeof(char))) == NULL)
            return INVALID_MEMORY_ALLOCATION;

        cell->allocated += CELL_ALLOC_SIZE;
    }

    cell->value[cell->length] = c;
    cell->length++;

    return 0;
}

//Delim validation
int isValidDelim(char *delim)
{
    if (strchr(delim, '"') != NULL)
        return INVALID_DELIMITER;
    if (strchr(delim, '\\') != NULL)
        return INVALID_DELIMITER;

    return 0;
}

bool isCharInString(char c, char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (c == str[i])
            return true;
    }

    return false;
}

//Delete selected rows
int drow(Table *table, Selection *selection)
{
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = 0; j < table->rows[i].allocated; j++)
        {
            cell_dtor(&table->rows[i].cells[j]);
        }
        row_dtor(&table->rows[i]);
    }

    memmove(&table->rows[selection->fromRow], &table->rows[toRow + 1], (table->allocated - (toRow + 1)) * sizeof(Row));

    int rowsToDelete = (toRow - selection->fromRow) + 1;

    if((table->rows = realloc(table->rows, (table->allocated - rowsToDelete) * sizeof(Row))) == NULL)
        return INVALID_MEMORY_ALLOCATION;
    table->allocated -= rowsToDelete;
    table->length -= rowsToDelete;

    return 0;
}

//Insert one row above selection
int irow(Table *table, Selection *selection)
{
    int retVal;
    if (selection->fromRow > table->length)
        return -1;

    if (table->length == table->allocated)
    {
        if((retVal = allocateRowsToTable(table, 1)) != 0)
            return retVal;
    }

    int colCount = table->rows[0].allocated;

    memmove(&table->rows[selection->fromRow + 1], &table->rows[selection->fromRow], (table->allocated - (selection->fromRow + 1)) * sizeof(Row));
    row_ctor(&table->rows[selection->fromRow]);
    table->length++;

    if((retVal = allocateCellsToRow(&table->rows[selection->fromRow], colCount)) != 0)
        return retVal;
    table->rows[selection->fromRow].length = colCount;

    return 0;
}

//Add one row under selection
int arow(Table *table, Selection *selection)
{
    int retVal;
    if (selection->toRow > table->length)
        return -1;

    if (table->length == table->allocated)
    {
        if((retVal = allocateRowsToTable(table, 1)) != 0)
            return retVal;
    }

    int colCount = table->rows[0].allocated;

    memmove(&table->rows[selection->toRow + 1], &table->rows[selection->toRow], (table->allocated - (selection->toRow + 1)) * sizeof(Row));
    row_ctor(&table->rows[selection->toRow + 1]);
    table->length++;

    if((retVal = allocateCellsToRow(&table->rows[selection->toRow + 1], colCount)) != 0)
        return retVal;
    table->rows[selection->toRow + 1].length = colCount;

    return 0;
}

//Add one column before selection
int icol(Table *table, Selection *selection)
{
    int retVal;
    for (int i = 0; i < table->length; i++)
    {
        if (table->rows[i].length == table->rows[i].allocated)
        {
            if((retVal = allocateCellsToRow(&table->rows[i], 1)) != 0)
                return retVal;
        }

        memmove(&table->rows[i].cells[selection->fromCol + 1], &table->rows[i].cells[selection->fromCol],
                (table->rows[i].allocated - (selection->fromCol + 1)) * sizeof(Cell));

        cell_ctor(&table->rows[i].cells[selection->fromCol]);
        table->rows[i].length++;
    }
    return 0;
}

//Add one column after selection
int acol(Table *table, Selection *selection)
{
    int retVal;
    for (int i = 0; i < table->length; i++)
    {
        if (table->rows[i].length == table->rows[i].allocated)
        {
            if((retVal = allocateCellsToRow(&table->rows[i], 1)) != 0)
                return INVALID_MEMORY_ALLOCATION;
        }

        memmove(&table->rows[i].cells[selection->toCol + 1], &table->rows[i].cells[selection->toCol],
                (table->rows[i].allocated - (selection->toCol + 1)) * sizeof(Cell));

        cell_ctor(&table->rows[i].cells[selection->toCol + 1]);
        table->rows[i].length++;
    }
    return 0;
}

//Delete selected columns
int dcol(Table *table, Selection *selection)
{
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    for (int i = 0; i < table->allocated; i++)
    {
        for (int j = selection->fromCol; j < toCol; j++)
        {
            cell_dtor(&table->rows[i].cells[j]);
        }
        memmove(&table->rows[i].cells[selection->fromCol], &table->rows[i].cells[toCol + 1],
                (table->rows[i].allocated - (toCol + 1)) * sizeof(Cell));

        int colsToDelete = (toCol - selection->fromCol) + 1;

        if((table->rows[i].cells = realloc(table->rows[i].cells, (table->rows[i].allocated - colsToDelete) * sizeof(Cell))) == NULL)
            return INVALID_MEMORY_ALLOCATION;

        table->rows[i].allocated -= colsToDelete;
        table->rows[i].length -= colsToDelete;
    }
    return 0;
}

//Clear cell value
int clear(Table *table, Selection *selection)
{
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = selection->fromCol; j <= toCol; j++)
        {
            cell_dtor(&table->rows[i].cells[j]);
            cell_ctor(&table->rows[i].cells[j]);
        }
    }

    return 0;
}

//swap selection with <inputArg.position>
int swap(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int retVal;
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = selection->fromCol; j <= toCol; j++)
        {
            Cell *cell1 = &table->rows[inputArg->position.row].cells[inputArg->position.col];
            Cell *cell2 = &table->rows[i].cells[j];

            char *cell1Value = NULL;
            char *cell2Value = NULL;

            if (cell1->value != NULL)
            {
                if((cell1Value = malloc(strlen(cell1->value) + 1)) == NULL)
                    return INVALID_MEMORY_ALLOCATION;
                strcpy(cell1Value, cell1->value);
            }

            if (cell2->value != NULL)
            {
                if((cell2Value = malloc(strlen(cell2->value) + 1)) == NULL)
                    return INVALID_MEMORY_ALLOCATION;
                strcpy(cell2Value, cell2->value);
            }

            if((retVal = setCellValue(cell1, cell2Value)) != 0)
                return retVal;
            if((retVal = setCellValue(cell2, cell1Value)) != 0)
                return retVal;

            free(cell1Value);
            free(cell2Value);
        }
    }
    return 0;
}

//Calculate the sum of all numeric values in the selection
void getSelectionSum(Table *table, Selection *selection, double *sum, int *count)
{
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    if (sum != NULL)
        *sum = 0;
    if (count != NULL)
        *count = 0;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = selection->fromCol; j <= toCol; j++)
        {
            char *value = table->rows[i].cells[j].value;
            if (value != NULL)
            {
                char *endPtr = NULL;
                double num = strtof(value, &endPtr);
                if (*endPtr == '\0' && endPtr != value)
                {
                    if (sum != NULL)
                        *sum += num;
                    if (count != NULL)
                        *count += 1;
                }
            }
        }
    }
}

//Set <inputArg.position> to the sum of all selected numeric values
int sum(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int retVal;
    double sum = 0;
    getSelectionSum(table, selection, &sum, NULL);

    char arr[16];
    sprintf(arr, "%g", sum);

    if((retVal = setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr)) != 0)
        return retVal;

    return 0;
}

//Set <inputArg.position> to the average of all selected numeric values
int avg(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int retVal;
    double sum;
    int count;
    getSelectionSum(table, selection, &sum, &count);

    if (count == 0)
        return -1;

    char arr[16];
    sprintf(arr, "%g", sum / count);

    if((retVal = setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr)) != 0)
        return retVal;

    return 0;
}

//Set the values of the selected cells to <inputArg.inputString>
int set(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int retVal = 0;
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = selection->fromCol; j <= toCol; j++)
        {
            if((retVal = setCellValue(&table->rows[i].cells[j], inputArg->inputString)) != 0)
                return retVal;
        }
    }

    return 0;
}

//Set <inputArg.position> to the count of selected non-empty values
int count(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int retVal;
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    int count = 0;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = selection->fromCol; j <= toCol; j++)
        {
            if (strcmp(table->rows[i].cells[j].value, "") != 0)
            {
                count++;
            }
        }
    }

    char arr[16];
    sprintf(arr, "%d", count);

    if((retVal = setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr)) != 0)
        return retVal;

    return 0;
}

//Set <inputArg.position> to the length of the selected cell
int len(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int retVal;
    int length = strlen(table->rows[selection->fromRow].cells[selection->fromCol].value);

    char arr[16];
    sprintf(arr, "%d", length);

    if((retVal = setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr)) != 0)
        return retVal;

    return 0;
}

//Set the value of tempVar
int def(Table *table, Selection *selection, CmdArgument *inputArg, char *tempVar[10])
{
    char *cellValue = table->rows[selection->fromRow].cells[selection->fromCol].value;
    if (inputArg->inputString[0] != '_')
        return -1;

    char *endPtr = NULL;
    int varID = strtol(&inputArg->inputString[1], &endPtr, 10);
    if (*endPtr == '\0' && endPtr != &inputArg->inputString[1])
    {
        if (tempVar[varID] != NULL)
            free(tempVar[varID]);

        if((tempVar[varID] = malloc((strlen(cellValue) + 1) * sizeof(char))) == NULL)
            return INVALID_MEMORY_ALLOCATION;
        strcpy(tempVar[varID], cellValue);
    }
    else
    {
        return INVALID_VARIABLE;
    }

    return 0;
}

//Use the value of tempVar
int use(Table *table, Selection *selection, CmdArgument *inputArg, char *tempVar[10])
{
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    char *endPtr = NULL;
    int varID = strtol(&inputArg->inputString[1], &endPtr, 10);
    if (*endPtr == '\0' && endPtr != &inputArg->inputString[1])
    {
        for (int i = selection->fromRow; i <= toRow; i++)
        {
            for (int j = selection->fromCol; j <= toCol; j++)
            {
                if(setCellValue(&table->rows[i].cells[j], tempVar[varID]) != 0)
                    return INVALID_MEMORY_ALLOCATION;
            }
        }
    }
    else
    {
        return INVALID_VARIABLE;
    }

    return 0;
}

//Increment the value of tempVar by 1
int inc(CmdArgument *inputArg, char *tempVar[10])
{
    char *endPtr = NULL;
    int varID = strtol(&inputArg->inputString[1], &endPtr, 10);
    if (*endPtr == '\0' && endPtr != &inputArg->inputString[1])
    {
        double num;
        if (tempVar[varID] != NULL)
        {
            num = strtod(tempVar[varID], &endPtr);
            if (*endPtr == '\0' && endPtr != tempVar[varID])
            {
                num += 1;
                char arr[16];
                sprintf(arr, "%g", num);

                if((tempVar[varID] = realloc(tempVar[varID], (strlen(arr) + 1) * sizeof(char))) == NULL)
                    return INVALID_MEMORY_ALLOCATION;
                strcpy(tempVar[varID], arr);
            }
        }
        else
        {
            num = 1;
            char arr[16];
            sprintf(arr, "%g", num);

            if((tempVar[varID] = malloc((strlen(arr) + 1) * sizeof(char))) == NULL)
                return INVALID_MEMORY_ALLOCATION;
            strcpy(tempVar[varID], arr);
        }
    }
    else
    {
        return INVALID_VARIABLE;
    }

    return 0;
}

//Align table to the given <rowCount> and <colCount>
int alignTable(Table *table, int rowCount, int colCount)
{
    int retVal;
    if (rowCount > table->allocated)
    {
        if((retVal = allocateRowsToTable(table, rowCount - table->allocated)) != 0)
            return retVal;
        table->length = rowCount;
    }

    for (int i = 0; i < table->length; i++)
    {
        if (table->rows[i].length < colCount)
        {
            if((retVal = allocateCellsToRow(&table->rows[i], colCount - table->rows[i].length)) != 0)
                return retVal;
            table->rows[i].length = colCount;
        }
    }

    return 0;
}

//Find the longest non-empty row in table
int getMaxRowLength(Table *table)
{
    int max = 0;
    for (int i = 0; i < table->length; i++)
    {
        for (int j = 0; j < table->rows[i].length; j++)
        {
            if (table->rows[i].cells[j].value != NULL && max < j)
                max = j;
        }
    }

    return max + 1;
}

int printTableToFile(Table *table, char delim, char *filename)
{
    int maxRowLength = getMaxRowLength(table);
    FILE *outputFile;
    if((outputFile = fopen(filename, "w")) == NULL)
    {
        return INVALID_FILE_OPERATION;
    }
    for (int i = 0; i < table->length; i++)
    {
        for (int j = 0; j < maxRowLength; j++)
        {
            if (j < maxRowLength - 1)
            {
                fprintf(outputFile, "%s%c", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "", delim);
            }
            else
            {
                fprintf(outputFile, "%s", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "");
            }
        }
        if (table->rows[i].length != 0)
            fprintf(outputFile, "\n");
    }
    fclose(outputFile);

    return 0;
}

void printTable(Table *table, char delim)
{
    int maxRowLength = getMaxRowLength(table);
    //printf("%d", table->length);
    for (int i = 0; i < table->length; i++)
    {
        //printf("Length: %d Allocated: %d\n", table->rows[i].length, table->rows[i].allocated);
        for (int j = 0; j < maxRowLength; j++)
        {
            if (j < maxRowLength - 1)
            {
                printf("%s%c", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "", delim);
            }
            else
            {
                printf("%s", (table->rows[i].cells[j].value) ? table->rows[i].cells[j].value : "");
            }
        }
        if (table->rows[i].length != 0)
            putchar('\n');
    }
}

int loadTableFromFile(Table *table, char *filename, char *delim)
{
    int retVal;
    Cell cell;
    cell_ctor(&cell);
    FILE *inputFile;
    if((inputFile = fopen(filename, "r")) == NULL)
    {
        return INVALID_FILE_OPERATION;
    }

    char c;
    int rowID = 0;
    int colID = 0;
    int maxColID = 0;
    while ((c = fgetc(inputFile)) != EOF)
    {
        if (isCharInString(c, delim))
        {
            if((retVal = appendCharToCell(&cell, '\0')) != 0)
                return retVal;
            if((retVal = addCellToRow(table, rowID, cell.value)) != 0)
                return retVal;
            cell_dtor(&cell);
            cell_ctor(&cell);
            colID++;
        }
        else if (c == '\n')
        {
            if((retVal = appendCharToCell(&cell, '\0')) != 0)
                return retVal;
            if((retVal = addCellToRow(table, rowID, cell.value)) != 0)
                return retVal;
            cell_dtor(&cell);
            cell_ctor(&cell);
            rowID++;
            colID++;
            if (colID > maxColID)
                maxColID = colID;
            colID = 0;
        }
        else
        {
            if((retVal = appendCharToCell(&cell, c)) != 0)
                return retVal;
        }
    }
    fclose(inputFile);

    return maxColID;
}

//Find max number in selection and update the selection to that cell
int findMaxNumber(Table *table, Selection *selection)
{
    float max = INT_MIN;
    int rowID, colID;
    Selection res;

    for (rowID = selection->fromRow; rowID <= selection->toRow; rowID++)
    {
        for (colID = selection->fromCol; colID <= selection->toCol; colID++)
        {
            char *value = table->rows[rowID].cells[colID].value;
            char *endPtr = NULL;
            if (value != NULL)
            {

                float number = strtof(value, &endPtr);
                if (number > max && *endPtr == '\0' && endPtr != value)
                {

                    max = number;
                    setSelection(&res, rowID, rowID, colID, colID);
                }
            }
        }
    }

    setSelection(selection, res.fromRow, res.toRow, res.fromCol, res.toCol);

    return 0;
}

//Find min number in selection and update the selection to that cell
int findMinNumber(Table *table, Selection *selection)
{
    float min = INT_MAX;
    int rowID, colID;
    Selection res;

    for (rowID = selection->fromRow; rowID <= selection->toRow; rowID++)
    {
        for (colID = selection->fromCol; colID <= selection->toCol; colID++)
        {
            char *endPtr = NULL;
            if (table->rows[rowID].cells[colID].value != NULL)
            {
                char *cellValue = table->rows[rowID].cells[colID].value;
                float number = strtof(cellValue, &endPtr);

                if (min > number && *endPtr == '\0' && endPtr != cellValue)
                {
                    min = number;
                    setSelection(&res, rowID, rowID, colID, colID);
                }
            }
        }
    }

    setSelection(selection, res.fromRow, res.toRow, res.fromCol, res.toCol);

    return 0;
}

//Find <str> in selection and update the selection to that cell
int findString(Table *table, Selection *selection, char *str)
{
    int rowID, colID;
    for (rowID = 0; rowID < table->length; rowID++)
    {
        for (colID = 0; colID < table->rows[rowID].length; colID++)
        {
            if (strcmp(table->rows[rowID].cells[colID].value, str) == 0)
            {
                setSelection(selection, rowID, rowID, colID, colID);
                return 0;
            }
        }
    }

    return 0;
}

typedef struct
{
    char *cmd;
    int (*function)();
    bool inputArg;
    bool tempVar;
} commandList;
//Array of all supported command (without len())
const commandList cmdList[COMMAND_COUNT - 1] = {
    {"irow", irow, false, false},
    {"arow", arow, false, false},
    {"drow", drow, false, false},
    {"icol", icol, false, false},
    {"acol", acol, false, false},
    {"dcol", dcol, false, false},
    {"clear", clear, false, false},
    {"swap", swap, true, false},
    {"sum", sum, true, false},
    {"avg", avg, true, false},
    {"set", set, true, false},
    {"count", count, true, false},
    {"len", len, true, false},
    {"def", def, true, true},
    {"use", use, true, true}};

//Execute the given <cmd> over <selection>
int commandExecutor(Table *table, char *cmd, Selection *selection, CmdArgument *inputArg, char *tempVar[10])
{
    int retVal;
    for (int i = 0; i < COMMAND_COUNT - 1; i++)
    {
        if (strcmp(cmd, cmdList[i].cmd) == 0)
        {
            if (cmdList[i].inputArg && cmdList[i].tempVar)
                retVal = cmdList[i].function(table, selection, inputArg, tempVar);
            else if (cmdList[i].inputArg)
                retVal = cmdList[i].function(table, selection, inputArg);
            else if (cmdList[i].tempVar)
                retVal = cmdList[i].function(table, selection, tempVar);
            else
                retVal = cmdList[i].function(table, selection);

            return retVal;
        }
    }

    if (strcmp(cmd, "inc") == 0)
    {
        retVal = inc(inputArg, tempVar);
        return retVal;
    }

    return INVALID_COMMAND; //command Not Found
}

//Parse selectoin string
int parseSelection(Table *table, char *selectionStr, Selection *selection, Selection *tempSelection)
{
    int retVal;
    char *selectionStrCopy = NULL;
    if((selectionStrCopy = malloc((strlen(selectionStr) + 1) * sizeof(char))) == NULL)
        return INVALID_MEMORY_ALLOCATION;

    strcpy(selectionStrCopy, selectionStr);

    bool isFind = false;
    char *lastPtr = &selectionStrCopy[1];
    int arrIndex = 0;
    for (int i = 1; selectionStrCopy[i] != '\0'; i++)
    {
        if (selectionStrCopy[i] == ',' || selectionStrCopy[i] == ']' || selectionStrCopy[i] == ' ')
        {
            bool end = false;
            if (selectionStrCopy[i] == ']')
                end = true;

            selectionStrCopy[i] = '\0';
            char *endPtr = NULL;
            int value = strtol(lastPtr, &endPtr, 10);
            if (value == 0 && endPtr != lastPtr)
            {
                return INVALID_SELECTION;
            }
            value -= 1;
            if (strcmp(lastPtr, "min") == 0)
            {
                findMinNumber(table, selection);
                break;
            }
            else if (strcmp(lastPtr, "max") == 0)
            {
                findMaxNumber(table, selection);
                break;
            }
            else if (strcmp(lastPtr, "find") == 0)
            {
                isFind = true;
            }
            else if (isFind)
            {
                printf("%s", lastPtr);
                findString(table, selection, lastPtr);
                break;
            }
            else if (strcmp(lastPtr, "set") == 0)
            {
                setSelection(tempSelection, selection->fromRow, selection->toRow,
                             selection->fromCol, selection->toCol);
                break;
            }
            else if (strcmp(lastPtr, "_") == 0)
            {
                if (arrIndex == 0 && end)
                {
                    setSelection(selection, tempSelection->fromRow, tempSelection->toRow,
                                 tempSelection->fromCol, tempSelection->toCol);
                    break;
                }
                if (arrIndex == 0)
                {
                    selection->fromRow = 0;
                    selection->toRow = table->length - 1;
                }
                else if (arrIndex == 1)
                {
                    selection->fromCol = 0;
                    selection->toCol = table->rows[0].length - 1;
                }
                else
                {
                    return INVALID_SELECTION;
                }
            }
            else if (strcmp(lastPtr, "-") == 0)
            {
                if (arrIndex == 2)
                    selection->toRow = table->length - 1;
                else if (arrIndex == 3)
                    selection->toCol = table->rows[0].length - 1;
            }
            else if(*endPtr == '\0')
            {
                if (arrIndex == 0)
                {
                    selection->fromRow = value;
                    selection->toRow = value;
                }
                else if (arrIndex == 1)
                {
                    selection->fromCol = value;
                    selection->toCol = value;
                }
                else if (arrIndex == 2)
                    selection->toRow = value;
                else if (arrIndex == 3)
                    selection->toCol = value;
            }
            else
            {
                return INVALID_SELECTION;
            }

            arrIndex++;
            lastPtr = &selectionStrCopy[i + 1];
        }
    }

    if((retVal = alignTable(table, selection->toRow + 1, selection->toCol + 1)) != 0)
        return retVal;

    free(selectionStrCopy);
    return 0;
}

//Parse command argument positions
int getPositionBetweenBrackets(char *str, CmdArgument *cmdArg)
{
    char *endPtr = NULL;
    cmdArg->position.row = strtol(&str[1], &endPtr, 10) - 1;
    if (endPtr[0] != ',')
    {
        return INVALID_COMMAND_ARGUMENT;
    }
    cmdArg->position.col = strtol(&endPtr[1], &endPtr, 10) - 1;

    return 0;
}

//Parse commands and run them over selections
int parseCommands(Table *table, char *cmdSequence, Selection *tempSelection, char *tempVar[10])
{
    int retVal;
    char *cmd;
    Selection selection;
    selection_ctor(&selection);
    CmdArgument cmdArg;

    cmd = strtok(cmdSequence, ";");
    while (cmd != NULL)
    {
        if (cmd[0] == '[')
        {
            if((retVal = parseSelection(table, cmd, &selection, tempSelection)) != 0)
                return retVal;
        }
        else
        {
            char *space = strchr(cmd, ' ');
            if (space != NULL)
            {
                space[0] = '\0';
                if (space[1] == '[')
                {
                    if((retVal = getPositionBetweenBrackets(&space[1], &cmdArg)) != 0)
                        return retVal;
                }
                else
                {
                    cmdArg.inputString = &space[1];
                }
            }
            if((retVal = commandExecutor(table, cmd, &selection, &cmdArg, tempVar)) != 0)
                return retVal;
        }
        cmd = strtok(NULL, ";");
    }

    return 0;
}

int main(int argc, char **argv)
{
    char *delim = " ";
    char *cmdSequence = "";
    char *fileName = "";
    int retVal;

    if (argc == 5)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            delim = argv[2];
            cmdSequence = argv[3];
            fileName = argv[4];
        }
        else
        {
            return error(INVALID_ARGUMENT_COUNT);
        }
    }
    else if(argc == 3)
    {
        cmdSequence = argv[1];
        fileName = argv[2];
    }
    else
    {
        return error(INVALID_ARGUMENT_COUNT);
    }

    if ((retVal = isValidDelim(delim)) != 0)
    {
        error(retVal);
    }

    char *tempVar[TEMP_VAR_COUNT] = {NULL};

    Selection tempSelection;
    selection_ctor(&tempSelection);

    Table table;
    table_ctor(&table);    

    int colCount = loadTableFromFile(&table, fileName, delim);
    if(colCount < 0)
        return error(colCount);

    if((retVal = alignTable(&table, table.length, colCount)) != 0)
        return error(retVal);
    if((retVal = parseCommands(&table, cmdSequence, &tempSelection, tempVar)) != 0)
        return error(retVal);

    if((retVal = printTableToFile(&table, delim[0], fileName)) != 0)
        return error(retVal);
    printTable(&table, delim[0]);
    

    table_dtor(&table);

    for (int i = 0; i < TEMP_VAR_COUNT; i++)
    {
        if (tempVar[i] != NULL)
        {
            free(tempVar[i]);
        }
    }

    return 0;
}