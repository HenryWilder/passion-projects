#include <stdlib.h>
#include <stdio.h>
#include "Parse.h"

ParseNodePtr CreateParseNode(Token data)
{
    ParseNodePtr node = (ParseNodePtr)malloc(sizeof(ParseNode));
    node->data = data;
    node->childCount = 0;
    node->children = NULL;
    return node;
}
void AddParseNodeChild(ParseNodePtr parent, ParseNodePtr child)
{
    const unsigned nodeBytes = sizeof(ParseNodePtr);
    const unsigned cpyBytes = nodeBytes * (parent->childCount);
    const unsigned allocBytes = cpyBytes + nodeBytes;
    const char* src = (char*)parent->children;
    parent->children = (ParseNodePtr*)malloc(allocBytes);
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
void DestroyParseBranch(ParseNodePtr node)
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
} ScopeStack, *ScopeStackPtr;

void PushToStack(ScopeStackPtr stack, Token token)
{
    stack->data[stack->size++] = token;
}
void PopOffStack(ScopeStackPtr stack, Token expect)
{
#if _DEBUG
    if (stack->size == 0)
    {
        puts("Tried to pop an empty stack");
        exit(2);
    }
#endif
    if (StackTop(stack) != expect)
    {
        puts("Tried to pop a different token from what was at the stack's top");
        exit(1);
    }
    --stack->size;
}
Token StackTop(ScopeStackPtr stack)
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

int IsWordChar(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '_');
}
int IsDigitChar(char c)
{
    return (c >= '0' && c <= '9');
}

void Parse(FILE* fp)
{
    ScopeStack stack;
    ParseNode topNode;
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
                PushToStack(&stack, _scope);
                break;

            case '}':
                puts("Close scope");
                PopOffStack(&stack, _scope);
                break;

            case '.':
                puts("Label name");
                PushToStack(&stack, _function);
                ++c;
                putchar('\"');
                if (IsWordChar(*c))
                {
                    putchar(*c, fp);
                    for (++c; !!*c; ++c)
                    {
                        if (!(IsWordChar(*c) || IsDigitChar(*c)))
                            PopOffStack(&stack, _function);
                        putchar(*c, fp);
                    }
                }
                putchar('\"');
                break;

            case '[':
                if (c[1] == '[') {
                    puts("Open comment");
                    PushToStack(&stack, _comment);
                    ++c;
                }
                break;

            case ']':
                if (c[1] == ']') {
                    puts("Close comment");
                    PopOffStack(&stack, _comment);
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
