#ifndef DISALVOFRANZA_PROGETTOSO_CONFIGUTILS_H
#define DISALVOFRANZA_PROGETTOSO_CONFIGUTILS_H

#include "CustomTypes.h"

void createConfig();

void deleteConfig();

void unloadConfig();

void setConfigValue(Config config, int value);

int getConfigValue(Config config);

#endif //DISALVOFRANZA_PROGETTOSO_CONFIGUTILS_H
