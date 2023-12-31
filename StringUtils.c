#include "StringUtils.h"
#include <stdlib.h>
#include <stdio.h>

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
    if (end > start || end < 0)
    {
        char* newString = emptyString((end < 0 ? stringLength(string) - start : end-start+1)+1);
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
    return "";
}

char* string(char* string)
{
    return stringCut(string, 0, -1);
}

char* emptyString(unsigned long length)
{
    char* string = malloc(sizeof(char)*length);
    for (unsigned int i = 0; i < length; i++)
    {
        string[i] = '\0';
    }
    return string;
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
            string += stringLength(startReading);
            break;
        }
        string++;
    }
    return stringCut(string, 0, -1);
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

char* intToString(int num)
{
    char* numString = emptyString(11);
    sprintf(numString, "%i", num);
    return numString;
}

char* stringJoin(char* string1, char* string2)
{
    int length1 = stringLength(string1);
    int length2 = stringLength(string2);
    int totalLength = length1 + length2;
    char* totalString = emptyString(totalLength);
    int i = 0;
    while (i < length1)
    {
        totalString[i] = string1[i];
        i++;
    }
    while (i < totalLength)
    {
        totalString[i] = string2[i - length1];
        i++;
    }
    return totalString;
}
