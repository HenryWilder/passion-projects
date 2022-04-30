#include <stdlib.h>
#include <stdio.h>
#include "Parse.h"

ParseNode* CreateParseNode(Token data)
{
    ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
    node->data = data;
    node->childCount = 0;
    node->children = NULL;
    return node;
}
void AddParseNodeSub(ParseNode* parent, ParseNode* child)
{
    const unsigned nodeBytes = sizeof(ParseNode*);
    const unsigned cpyBytes = nodeBytes * (parent->childCount);
    const unsigned allocBytes = cpyBytes + nodeBytes;
    const char* src = (char*)parent->children;
    parent->children = (ParseNode**)malloc(allocBytes);
    char* dest = (char*)parent->children;
    for (unsigned i = 0; i < cpyBytes; ++i)
    {
        *(dest++) = src[i];
    }
    free(src);
    src = (char*)&child;
    for (unsigned i = 0; i < nodeBytes; ++i)
    {
        dest[i] = src[i];
    }
    parent->childCount++;
}
void DestroyParseBranch(ParseNode* node)
{
    while (node->childCount-- > 0)
    {
        DestroyParseBranch(node->children[node->childCount]);
    }
    free(node);
}

#define STACK_SIZE 64

typedef struct ScopeStack {
    int size;
    Token data[STACK_SIZE];
} ScopeStack;

void PushToStack(ScopeStack* stack, Token token)
{
    stack->data[stack->size++] = token;
}
void PopOffStack(ScopeStack* stack)
{
#if _DEBUG
    if (stack->size == 0)
    {
        puts("Tried to pop an empty stack");
        exit(2);
    }
#endif
    --stack->size;
}
Token StackTop(ScopeStack* stack)
{
#if _DEBUG
    if (stack->size == 0)
    {
        puts("Tried to access top of an empty stack");
        exit(2);
    }
#endif
    return stack->data[stack->size - 1];
}

void Parse(FILE* fp)
{
    ScopeStack stack;
    while (!feof(fp))
    {
        int buffSize = 64;
        char* buff = (char*)malloc(buffSize);
        if (!buff)
        {
            puts("Failed to allocate memory");
            exit(1);
        }
        fgets(buff, buffSize, fp);
        printf(buff);
        for (const char* c = buff; !!*c; ++c)
        {
            switch (*c)
            {
            case '{':
                puts("Open scope");
                break;

            case '}':
                puts("Close scope");
                break;

            case '.':
                puts("Label name");
                break;

            case '[':
                if (c[1] == '[') {
                    puts("Open comment");
                    ++c;
                }
                break;

            case ']':
                if (c[1] == ']') {
                    puts("Close comment");
                    ++c;
                }
                break;

            default:
                break;
            }
        }
        free(buff);
    }
    puts("\nFinished.");
}
