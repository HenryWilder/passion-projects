#pragma once
struct FILE;

typedef enum Token {
    _scope, // {}
    _comment, // [[]]
    _for,
    _function,
} Token;

typedef struct ParseNode {
    Token data;
    unsigned childCount;
    ParseNode** children;
} ParseNode, *ParseNodePtr;
ParseNodePtr CreateParseNode(Token data);
void AddParseNodeChild(ParseNodePtr parent, ParseNodePtr child);
void DestroyParseBranch(ParseNodePtr);

void Parse(FILE* fp);
