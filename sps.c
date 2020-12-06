#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define ROW_ALLOC_SIZE 5
#define CELL_ALLOC_SIZE 5
#define COMMAND_COUNT 16
#define TEMP_VAR_COUNT 10

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
    for (int i = table->allocated; i < table->allocated + n; i++)
        row_ctor(&table->rows[i]);

    table->allocated += n;
    return 0;
}

int allocateCellsToRow(Row *row, int n)
{
    row->cells = realloc(row->cells, (row->allocated + n) * sizeof(Cell));
    for (int i = row->allocated; i < row->allocated + n; i++)
        cell_ctor(&row->cells[i]);

    row->allocated += n;
    return 0;
}

int addCellToRow(Table *table, int rowID, char *value)
{
    if (table->length == table->allocated)
    {
        allocateRowsToTable(table, (rowID + 1) - table->allocated);
        table->length = rowID + 1;
    }

    Row *row = &table->rows[rowID];

    if (row->length == row->allocated)
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
    if (cell->length == cell->allocated)
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
    cell_dtor(cell);
    cell_ctor(cell);

    if (value == NULL)
        return 0;

    int valueSize = strlen(value);

    cell->value = realloc(cell->value, (valueSize + 1) * sizeof(char));
    strcpy(cell->value, value);

    cell->allocated = valueSize + 1;
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
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (c == str[i])
            return true;
    }

    return false;
}

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

    table->rows = realloc(table->rows, (table->allocated - rowsToDelete) * sizeof(Row));
    table->allocated -= rowsToDelete;
    table->length -= rowsToDelete;

    return 0;
}

int irow(Table *table, Selection *selection)
{
    if (selection->fromRow > table->length)
        return -1;

    if (table->length == table->allocated)
    {
        allocateRowsToTable(table, 1);
    }

    int colCount = table->rows[0].allocated;

    memmove(&table->rows[selection->fromRow + 1], &table->rows[selection->fromRow], (table->allocated - (selection->fromRow + 1)) * sizeof(Row));
    row_ctor(&table->rows[selection->fromRow]);
    table->length++;

    allocateCellsToRow(&table->rows[selection->fromRow], colCount);
    table->rows[selection->fromRow].length = colCount;

    return 0;
}

int arow(Table *table, Selection *selection)
{
    if (selection->toRow > table->length)
        return -1;

    if (table->length == table->allocated)
    {
        allocateRowsToTable(table, 1);
    }

    int colCount = table->rows[0].allocated;

    memmove(&table->rows[selection->toRow + 1], &table->rows[selection->toRow], (table->allocated - (selection->toRow + 1)) * sizeof(Row));
    row_ctor(&table->rows[selection->toRow + 1]);
    table->length++;

    allocateCellsToRow(&table->rows[selection->toRow + 1], colCount);
    table->rows[selection->toRow + 1].length = colCount;

    return 0;
}

int icol(Table *table, Selection *selection)
{
    for (int i = 0; i < table->length; i++)
    {
        if (table->rows[i].length == table->rows[i].allocated)
        {
            allocateCellsToRow(&table->rows[i], 1);
        }

        memmove(&table->rows[i].cells[selection->fromCol + 1], &table->rows[i].cells[selection->fromCol],
                (table->rows[i].allocated - (selection->fromCol + 1)) * sizeof(Cell));

        cell_ctor(&table->rows[i].cells[selection->fromCol]);
        table->rows[i].length++;
    }
    return 0;
}

int acol(Table *table, Selection *selection)
{
    for (int i = 0; i < table->length; i++)
    {
        if (table->rows[i].length == table->rows[i].allocated)
        {
            allocateCellsToRow(&table->rows[i], 1);
        }

        memmove(&table->rows[i].cells[selection->toCol + 1], &table->rows[i].cells[selection->toCol],
                (table->rows[i].allocated - (selection->toCol + 1)) * sizeof(Cell));

        cell_ctor(&table->rows[i].cells[selection->toCol + 1]);
        table->rows[i].length++;
    }
    return 0;
}

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

        table->rows[i].cells = realloc(table->rows[i].cells, (table->rows[i].allocated - colsToDelete) * sizeof(Cell));
        table->rows[i].allocated -= colsToDelete;
        table->rows[i].length -= colsToDelete;
    }

    return 0;
}

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

    return 1;
}

int swap(Table *table, Selection *selection, CmdArgument *inputArg)
{
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
                cell1Value = malloc(strlen(cell1->value) + 1);
                strcpy(cell1Value, cell1->value);
            }

            if (cell2->value != NULL)
            {
                cell2Value = malloc(strlen(cell2->value) + 1);
                strcpy(cell2Value, cell2->value);
            }

            setCellValue(cell1, cell2Value);
            setCellValue(cell2, cell1Value);

            free(cell1Value);
            free(cell2Value);
        }
    }
    return 0;
}

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

int sum(Table *table, Selection *selection, CmdArgument *inputArg)
{
    double sum = 0;
    getSelectionSum(table, selection, &sum, NULL);

    char arr[16];
    sprintf(arr, "%g", sum);

    setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr);

    return 0;
}

int avg(Table *table, Selection *selection, CmdArgument *inputArg)
{
    double sum;
    int count;
    getSelectionSum(table, selection, &sum, &count);

    if (count == 0)
        return -1;

    char arr[16];
    sprintf(arr, "%g", sum / count);

    setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr);

    return 0;
}

int set(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int toRow = (selection->toRow > table->length) ? table->length - 1 : selection->toRow;
    int toCol = (selection->toCol > table->rows->length) ? table->rows->length - 1 : selection->toCol;

    for (int i = selection->fromRow; i <= toRow; i++)
    {
        for (int j = selection->fromCol; j <= toCol; j++)
        {
            setCellValue(&table->rows[i].cells[j], inputArg->inputString);
        }
    }

    return 0;
}

int count(Table *table, Selection *selection, CmdArgument *inputArg)
{
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

    setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr);

    return 0;
}

int len(Table *table, Selection *selection, CmdArgument *inputArg)
{
    int length = strlen(table->rows[selection->fromRow].cells[selection->fromCol].value);

    char arr[16];
    sprintf(arr, "%d", length);

    setCellValue(&table->rows[inputArg->position.row].cells[inputArg->position.col], arr);

    return 0;
}

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

        tempVar[varID] = malloc((strlen(cellValue) + 1) * sizeof(char));
        strcpy(tempVar[varID], cellValue);
    }

    return 0;
}

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
                setCellValue(&table->rows[i].cells[j], tempVar[varID]);
            }
        }
    }

    return 0;
}

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

                tempVar[varID] = realloc(tempVar[varID], (strlen(arr) + 1) * sizeof(char));
                strcpy(tempVar[varID], arr);
            }
        }
        else
        {
            num = 1;
            char arr[16];
            sprintf(arr, "%g", num);

            tempVar[varID] = malloc((strlen(arr) + 1) * sizeof(char));
            strcpy(tempVar[varID], arr);
        }
    }

    return 0;
}

int alignTable(Table *table, int rowCount, int colCount)
{
    if (rowCount > table->allocated)
    {
        allocateRowsToTable(table, rowCount - table->allocated);
        table->length = rowCount;
    }

    for (int i = 0; i < table->length; i++)
    {
        if (table->rows[i].length < colCount)
        {
            allocateCellsToRow(&table->rows[i], colCount - table->rows[i].length);
            table->rows[i].length = colCount;
        }
    }

    return 0;
}

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
    FILE *outputFile = fopen(filename, "w");
    for (int i = 0; i < table->length; i++)
    {
        //printf("Length: %d Allocated: %d\n", table->rows[i].length, table->rows[i].allocated);
        for (int j = 0; j < maxRowLength; j++)
        {
            if (j < table->rows[i].length - 1)
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
    Cell cell;
    cell_ctor(&cell);
    FILE *inputFile = fopen(filename, "r");

    char c;
    int rowID = 0;
    int colID = 0;
    int maxColID = 0;
    while ((c = fgetc(inputFile)) != EOF)
    {
        if (isCharInString(c, delim))
        {
            appendCharToCell(&cell, '\0');
            addCellToRow(table, rowID, cell.value);
            cell_dtor(&cell);
            cell_ctor(&cell);
            colID++;
        }
        else if (c == '\n')
        {
            appendCharToCell(&cell, '\0');
            addCellToRow(table, rowID, cell.value);
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
            appendCharToCell(&cell, c);
        }
    }
    fclose(inputFile);

    return maxColID;
}

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

    return 1;
}

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

    return 1;
}

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
                return 1;
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

commandList cmdList[COMMAND_COUNT - 1] = {
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

int commandExecutor(Table *table, char *cmd, Selection *selection, CmdArgument *inputArg, char *tempVar[10])
{
    for (int i = 0; i < COMMAND_COUNT - 1; i++)
    {
        if (strcmp(cmd, cmdList[i].cmd) == 0)
        {
            if (cmdList[i].inputArg && cmdList[i].tempVar)
                cmdList[i].function(table, selection, inputArg, tempVar);
            else if (cmdList[i].inputArg)
                cmdList[i].function(table, selection, inputArg);
            else if (cmdList[i].tempVar)
                cmdList[i].function(table, selection, tempVar);
            else
                cmdList[i].function(table, selection);
            return 0;
        }
    }

    if (strcmp(cmd, "inc") == 0)
    {
        inc(inputArg, tempVar);
        return 0;
    }

    return -1; //command Not Found
}

int parseSelection(Table *table, char *selectionStr, Selection *selection, Selection *tempSelection)
{
    char *selectionStrCopy = malloc((strlen(selectionStr) + 1) * sizeof(char));
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
                //error
                return -1;
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
                    //ERROR
                }
            }
            else if (strcmp(lastPtr, "-") == 0)
            {
                if (arrIndex == 2)
                    selection->toRow = table->length - 1;
                else if (arrIndex == 3)
                    selection->toCol = table->rows[0].length - 1;
            }
            else if (arrIndex == 0)
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

            arrIndex++;
            lastPtr = &selectionStrCopy[i + 1];
        }
    }
    // printf("fromRow: %d\n", selection->fromRow);
    // printf("toRow: %d\n", selection->toRow);
    // printf("fromCol: %d\n", selection->fromCol);
    // printf("toCol: %d\n", selection->toCol);

    alignTable(table, selection->toRow + 1, selection->toCol + 1);

    free(selectionStrCopy);
    return 1;
}

int getPositionBetweenBrackets(char *str, CmdArgument *cmdArg)
{
    char *endPtr = NULL;
    cmdArg->position.row = strtol(&str[1], &endPtr, 10) - 1;
    if (endPtr[0] != ',')
    {
        return -1;
    }
    cmdArg->position.col = strtol(&endPtr[1], &endPtr, 10) - 1;

    return 0;
}

int parseCommands(Table *table, char *cmdSequence, Selection *tempSelection, char *tempVar[10])
{
    char *cmd;
    Selection selection;
    CmdArgument cmdArg;

    cmd = strtok(cmdSequence, ";");
    while (cmd != NULL)
    {
        if (cmd[0] == '[')
        {
            parseSelection(table, cmd, &selection, tempSelection);
        }
        else
        {
            char *space = strchr(cmd, ' ');
            if (space != NULL)
            {
                space[0] = '\0';
                if (space[1] == '[')
                    getPositionBetweenBrackets(&space[1], &cmdArg);
                else
                    cmdArg.inputString = &space[1];
            }
            commandExecutor(table, cmd, &selection, &cmdArg, tempVar);
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
            //INVALID ARGUMENT COUNT
            return 1;
        }
    }
    else if(argc == 3)
    {
        cmdSequence = argv[1];
        fileName = argv[2];
    }
    else
    {
        //INVALID ARGUMENT COUNT
        return 1;
    }

    if (isValidDelim(delim) == 0)
    {
        //INVALID DELIMITER
        return 1;
    }

    char *tempVar[TEMP_VAR_COUNT] = {NULL};

    Selection tempSelection;
    selection_ctor(&tempSelection);

    Table table;
    table_ctor(&table);    

    int colCount = loadTableFromFile(&table, fileName, delim);

    alignTable(&table, table.length, colCount);
    parseCommands(&table, cmdSequence, &tempSelection, tempVar);

    printTable(&table, delim[0]);
    printTableToFile(&table, delim[0], fileName);

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