#ifndef DISALVOFRANZA_PROGETTOSO_STRINGUTILS_H
#define DISALVOFRANZA_PROGETTOSO_STRINGUTILS_H

#include "CustomTypes.h"

int stringLength(char* string);

char* stringCut(char* string, int start, int end);

char* string(char* string);

char* emptyString(int length);

boolean stringStartsWith(char* string, char* start);

boolean stringEndsWith(char* input, char* end);



#endif //DISALVOFRANZA_PROGETTOSO_STRINGUTILS_H
