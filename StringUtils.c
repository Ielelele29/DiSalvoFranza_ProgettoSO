#include "StringUtils.h"
#include <stdlib.h>


int stringLength(char* string)
{
    int length = 0;
    while (*string != '\0')
    {
        length++;
        string++;
    }
    return length;
}

char* stringCut(char* string, int start, int end)
{
    char* newString = malloc(sizeof(char)*stringLength(string));
    int i = 0;
    int j = 0;
    while (string[i] != '\0')
    {
        if (i >= start && (i <= end || end < 0))
        {
            newString[j] = string[i];
            j++;
        }
        i++;
    }
    return newString;
}

char* string(char* string)
{
    return stringCut(string, 0, -1);
}

char* emptyString(int length)
{
    return malloc(sizeof(char)*length);
}

char* stringClearChar(char* string, char c)
{
    char* newString = emptyString(stringLength(string));
    int i = 0;
    int j = 0;
    while (string[i] != '\0')
    {
        if (string[i] != c)
        {
            newString[j] = string[i];
            j++;
        }
        i++;
    }
    j++;
    while (j <= i)
    {
        newString[j] = '\0';
        j++;
    }
    return newString;
}

boolean stringEquals(char* string1, char* string2)
{
    while (*string1 != '\0' && *string2 != '\0')
    {
        if (*string1 != *string2)
        {
            return false;
        }
        string1++;
        string2++;
    }
    return *string1 == *string2;
}

boolean stringStartsWith(char* string, char* start)
{
    while (*string != '\0' && *start != '\0')
    {
        if (*string != *start)
        {
            return false;
        }
        string++;
        start++;
    }
    return *start == '\0';
}

boolean stringEndsWith(char* input, char* end)
{
    int inputLength = stringLength(input);
    int endLength = stringLength(end);
    if (inputLength < endLength)
    {
        return false;
    }
    char* cutInput;
    if (endLength < inputLength)
    {
        cutInput = stringCut(input, inputLength - endLength, inputLength);
    }
    else
    {
        cutInput = stringCut(input, 0, -1);
    }
    int i = 0;
    while (cutInput[i] != '\0')
    {
        if (cutInput[i] != end[i])
        {
            return false;
        }
        i++;
    }
    return true;
}

char* stringAfter(char* string, char* startReading)
{
    while (*string != '\0')
    {
        if (stringStartsWith(string, startReading))
        {
            break;
        }
        string++;
    }
    return stringCut(string+stringLength(startReading), 0, -1);
}

char* stringBefore(char* input, char* endReading)
{
    char* newString = string(input);
    int i = 0;
    boolean replace = false;
    while (newString[i] != '\0')
    {
        if (!replace)
        {
            if (stringStartsWith(newString + i, endReading))
            {
                replace = true;
            }
        }
        if (replace)
        {
            newString[i] = '\0';
        }
        i++;
    }
    return newString;
}