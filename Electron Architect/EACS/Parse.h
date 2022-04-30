#pragma once
struct FILE;

enum Token {
    _scope, // {}
    _comment, // [[]]
    _for,
    _function,
};

typedef struct ParseNode {
    enum Token data;
    unsigned childCount;
    ParseNode* children[];
} ParseNode;
ParseNode* CreateParseNode(enum Token data);
void AddParseNodeChild(ParseNode* parent, ParseNode* child);
void DestroyParseBranch(ParseNodePtr);

void Parse(FILE* fp);
