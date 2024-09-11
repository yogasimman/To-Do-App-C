#ifndef FILE_PATH_H
#define FILE_PATH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Response {
    char* htmlpath;
    FILE* filepath;
    char ContentType[20];
} Response;

typedef struct TreeNode {
    Response* val;
    struct TreeNode* left;
    struct TreeNode* right;
    int height;
} TreeNode;

Response* createResponse(char* htmlpath, char* filepath, char* ContentType);
TreeNode* createTreeNode(Response* val, TreeNode* left, TreeNode* right);
int height(TreeNode* n);
int getBalance(TreeNode* n);
TreeNode* rightRotate(TreeNode* y);
TreeNode* leftRotate(TreeNode* x);
TreeNode* insert(TreeNode* node,Response* key);
TreeNode* insertFile(TreeNode* root, char* type, char* path, char* filepath);
TreeNode* search(TreeNode* root, const char* htmlpath);
void readFile(TreeNode** root);
void deallocateTree(TreeNode* node);

#endif
