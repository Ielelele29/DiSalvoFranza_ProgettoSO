#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "CustomTypes.h"
#include "NodeUtils.h"

Node* createNode(int value)
{
    Node* node = malloc(sizeof(Node));
    if (node != NULL)
    {
        node->value = value;
        node->previousNode = NULL;
        node->nextNode = NULL;
    }
    return node;
}

void addNode(Node** parentP, int value)
{
    if (*parentP != NULL)
    {
        Node* parent = *parentP;
        parent = getLastNode(parent);
        Node* child = createNode(value);
        if (parent->nextNode != NULL)
        {
            child->nextNode = parent->nextNode;
            parent->nextNode->previousNode = child;
        }
        child->previousNode = parent;
        parent->nextNode = child;
    }
    else
    {
        *parentP = createNode(value);
    }
}

Node* removeNode(Node* node)
{
    if (node != NULL)
    {
        Node* firstNode = getFirstNode(node);
        boolean isFirst = firstNode == node;
        Node* parent = node->previousNode;
        Node* child = node->nextNode;
        if (parent != NULL)
        {
            parent->nextNode = child;
        }
        if (child != NULL)
        {
            child->previousNode = parent;
        }
        free(node);
        if (isFirst)
        {
            return child;
        }
        return firstNode;
    }
    return NULL;
}

Node* getFirstNode(Node* node)
{
    if (node != NULL)
    {
        Node* firstNode = node;
        while (hasPreviousNode(firstNode))
        {
            firstNode = getPreviousNode(firstNode);
            if (firstNode == node)
            {
                return node;
            }
        }
        return firstNode;
    }
    return NULL;
}

Node* getLastNode(Node* node)
{
    if (node != NULL)
    {
        Node* lastNode = node;
        while (hasNextNode(lastNode))
        {
            lastNode = getNextNode(lastNode);
            if (lastNode == node)
            {
                return node;
            }
        }
        return lastNode;
    }
    return NULL;
}

void setNodeValue(Node* node, int value)
{
    if (node != NULL)
    {
        node->value = value;
    }
}

int getNodeValue(Node* node)
{
    if (node != NULL)
    {
        return node->value;
    }
    return -1;
}

boolean hasNextNode(Node* node)
{
    if (node != NULL)
    {
        return node->nextNode != NULL;
    }
    return false;
}

Node* getNextNode(Node* node)
{
    if (node != NULL)
    {
        return node->nextNode;
    }
    return NULL;
}

boolean hasPreviousNode(Node* node)
{
    if (node != NULL)
    {
        return node->previousNode != NULL;
    }
    return false;
}

Node* getPreviousNode(Node* node)
{
    if (node != NULL)
    {
        return node->previousNode;
    }
    return NULL;
}

Node* searchNodeValue(Node* node, int value)
{
    if (node != NULL)
    {
        Node* firstNode = getFirstNode(node);
        while (firstNode != NULL)
        {
            if (firstNode->value == value)
            {
                return firstNode;
            }
            firstNode = getNextNode(firstNode);
            if (firstNode == node)
            {
                break;
            }
        }
    }
    return NULL;
}