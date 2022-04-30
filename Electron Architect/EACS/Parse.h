#pragma once
struct FILE;

typedef enum Token {
    _scope, // {}
    _comment, // [[]]
    _for,
} Token;

typedef struct ParseNode {
    Token data;
    unsigned childCount;
    ParseNode** children;
} ParseNode;
ParseNode* CreateParseNode();
void AddParseNodeChild(ParseNode* parent, ParseNode* child);
void DestroyParseBranch(ParseNode*);

void Parse(FILE* fp);
