#ifndef DISALVOFRANZA_PROGETTOSO_NODEUTILS_H
#define DISALVOFRANZA_PROGETTOSO_NODEUTILS_H
#include "CustomTypes.h"

Node* createNode(int value);

void addNode(Node** parent, int value);

Node* removeNode(Node* node);

Node* getFirstNode(Node* node);

Node* getLastNode(Node* node);

void setNodeValue(Node* node, int value);

int getNodeValue(Node* node);

boolean hasNextNode(Node* node);

Node* getNextNode(Node* node);

boolean hasPreviousNode(Node* node);

Node* getPreviousNode(Node* node);

Node* searchNodeValue(Node* node, int value);

#endif //DISALVOFRANZA_PROGETTOSO_NODEUTILS_H
