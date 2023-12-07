#ifndef DISALVOFRANZA_PROGETTOSO_NODEUTILS_H
#define DISALVOFRANZA_PROGETTOSO_NODEUTILS_H

#include "CustomTypes.h"

typedef struct Node {
    struct Node* previousNode;
    int value;
    struct Node* nextNode;
} Node;

Node* createNode(int value);

void addNode(Node** parent, int value);

void addNodeAfter(Node* parent, int value);

void removeNode(Node* node);

Node* getFirstNode(Node* node);

Node* getLastNode(Node* node);

void setNodeValue(Node* node, int value);

int getNodeValue(Node* node);

boolean hasNextNode(Node* node);

Node* getNextNode(Node* node);

boolean hasPreviousNode(Node* node);

Node* getPreviousNode(Node* node);

Node* searchNodeValue(Node* node, int value);

int nodeSize(Node* node);

#endif //DISALVOFRANZA_PROGETTOSO_NODEUTILS_H
