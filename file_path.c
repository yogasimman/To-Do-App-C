#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_path.h"

// Assuming Response and TreeNode structs are defined in file_path.h, you don't need to redefine them here.

// Implement the max function as a macro
#define max(a, b) ((a) > (b) ? (a) : (b))

Response* createResponse(char* htmlpath, char* filepath, char* ContentType) {
    Response* response = (Response*)malloc(sizeof(Response));
    if (response == NULL) {
        perror("Failed to allocate memory for Response");
        return NULL;
    }
    
    response->htmlpath = strdup(htmlpath);
    if (response->htmlpath == NULL) {
        perror("Failed to duplicate htmlpath");
        free(response);
        return NULL;
    }
    
    response->filepath = fopen(filepath, "r");
    if (response->filepath == NULL) {
        perror("Failed to open file");
        free(response->htmlpath);
        free(response);
        return NULL;
    }
    
    strcpy(response->ContentType, ContentType);
    return response;
}


TreeNode* createTreeNode(Response* val,TreeNode* left,TreeNode* right){
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    t->val = val;
    t->left = left;
    t->right = right;
    t->height = 1;
    return t;
}

int height(TreeNode* n){
    if(n == NULL)
        return 0;
    return n->height;
}

int getBalance(TreeNode *n){
    if(n == NULL){
        return 0;
    }
    return height(n->left) - height(n->right);
}

TreeNode *rightRotate(TreeNode *y){
    TreeNode* x = y->left;
    TreeNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left),height(x->right)) + 1;
    return x;
}

TreeNode *leftRotate(TreeNode *x){
    TreeNode *y = x->right;
    TreeNode *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left),height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

TreeNode* insert(TreeNode* node, Response* key) {
    if (node == NULL) {
        return createTreeNode(key, NULL, NULL);
    }

    if (strcmp(key->htmlpath, node->val->htmlpath) < 0) {
        node->left = insert(node->left, key);
    } else if (strcmp(key->htmlpath, node->val->htmlpath) > 0) {
        node->right = insert(node->right, key);
    } else {
        return node; // Duplicate keys are not allowed
    }

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && strcmp(key->htmlpath, node->left->val->htmlpath) < 0) {
        return rightRotate(node);
    }

    if (balance < -1 && strcmp(key->htmlpath, node->right->val->htmlpath) > 0) {
        return leftRotate(node);
    }

    if (balance > 1 && strcmp(key->htmlpath, node->left->val->htmlpath) > 0) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance < -1 && strcmp(key->htmlpath, node->right->val->htmlpath) < 0) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}


TreeNode* insertHTML(TreeNode* root, char* htmlpath, char* filepath) {
    Response* response = createResponse(htmlpath, filepath, "text/html");
    return insert(root, response);
}

TreeNode* insertCSS(TreeNode* root, char* csspath, char* filepath) {
    Response* response = createResponse(csspath, filepath, "text/css");
    return insert(root, response);
}

TreeNode* insertJS(TreeNode* root, char* jspath, char* filepath) {
    Response* response = createResponse(jspath, filepath, "application/javascript");
    return insert(root, response);
}

TreeNode* insertJSON(TreeNode* root, char* jsonpath, char* filepath) {
    Response* response = createResponse(jsonpath, filepath, "application/json");
    return insert(root, response);
}

// Wrapper function
TreeNode* insertFile(TreeNode* root, char* type, char* path, char* filepath) {
    if (strcmp(type, "html") == 0) {
        return insertHTML(root, path, filepath);
    } else if (strcmp(type, "css") == 0) {
        return insertCSS(root, path, filepath);
    } else if (strcmp(type, "js") == 0) {
        return insertJS(root, path, filepath);
    } else if (strcmp(type, "json") == 0) {
        return insertJSON(root, path, filepath);
    }
    return root; // If type is not recognized, return the root unchanged
}

TreeNode* search(TreeNode* root, const char* htmlpath) {
    if (root == NULL) {
        printf("Search reached a NULL node\n"); // Debug
        return NULL;
    }

    if (root->val == NULL) {
        printf("TreeNode val is NULL\n"); // Debug
        return NULL;
    }

    if (root->val->htmlpath == NULL) {
        printf("Response htmlpath is NULL\n"); // Debug
        return NULL;
    }

    if (strcmp(root->val->htmlpath, htmlpath) == 0) {
        return root;
    }

    if (strcmp(htmlpath, root->val->htmlpath) < 0) {
        return search(root->left, htmlpath);
    }

    return search(root->right, htmlpath);
}


void readFile(TreeNode** root) {
    FILE* file = fopen("file-exist.txt", "r");
    if (!file) {
        perror("Failed to open file-exist.txt");
        return;
    }

    char type[10], path[128], filepath[128];
    while (fscanf(file, "%s %s %s", type, path, filepath) != EOF) {
        *root = insertFile(*root, type, path, filepath);
    }

    fclose(file);
}

// Function to recursively deallocate memory in the AVL tree
void deallocateTree(TreeNode* node) {
    if (node == NULL) {
        return;
    }

    deallocateTree(node->left);
    deallocateTree(node->right);

    // Close the file pointer if it is open
    if (node->val->filepath) {
        fclose(node->val->filepath);
    }

    free(node->val);
    free(node);
}
