#ifndef DISALVOFRANZA_PROGETTOSO_STRINGUTILS_H
#define DISALVOFRANZA_PROGETTOSO_STRINGUTILS_H

#include "CustomTypes.h"

int stringLength(char* string);

char* stringCut(char* string, int start, int end);

char* string(char* string);

char* emptyString(unsigned long length);

char* stringClearChar(char* string, char c);

boolean stringEquals(char* string1, char* string2);

boolean stringStartsWith(char* string, char* start);

boolean stringEndsWith(char* input, char* end);

char* stringAfter(char* string, char* startReading);

char* stringBefore(char* input, char* endReading);

char* intToString(int num);

char* stringJoin(char* string1, char* string2);



#endif //DISALVOFRANZA_PROGETTOSO_STRINGUTILS_H
