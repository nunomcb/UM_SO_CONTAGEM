#ifndef AVL_H
#define AVL_H

#include <stdlib.h>
#include "stack.h"

#define AVL_DEF_HEADER(prefix, keyType, valueType)                                          \
    typedef struct prefix##AVLNode_s {                                                      \
        keyType key;                                                                        \
        valueType value;                                                                    \
        int balance;                                                                        \
        struct prefix##AVLNode_s *left, *right;                                             \
    } * prefix##AVLNode;                                                                    \
                                                                                            \
    STACK_DEF_HEADER(prefix##AVLNode)                                                       \
                                                                                            \
    typedef struct prefix##AVL_s {                                                          \
        struct {                                                                            \
            prefix##AVLNodeStack stack;                                                     \
            int initialized;                                                                \
        } generator;                                                                        \
        prefix##AVLNode root;                                                               \
        int (*compare)(keyType, keyType);                                                   \
        valueType (*collision)(valueType, valueType);                                       \
    } * prefix##AVL;                                                                        \
                                                                                            \
    typedef struct prefix##Entry_s {                                                        \
        keyType key;                                                                        \
        valueType value;                                                                    \
    } * prefix##Entry;                                                                      \

#define AVL_DEF(prefix, keyType, valueType)                                                 \
                                                                                            \
    STACK_DEF(prefix##AVLNode)                                                              \
                                                                                            \
    static prefix##AVLNode __##prefix##StClone(prefix##AVLNode node) {                      \
        return node;                                                                        \
    }                                                                                       \
                                                                                            \
    static void prefix##StackMin(prefix##AVLNodeStack stack, prefix##AVLNode node) {        \
                                                                                            \
        while (node) {                                                                      \
            stackPush(prefix##AVLNode, stack, node);                                        \
            node = node->left;                                                              \
        }                                                                                   \
                                                                                            \
        return;                                                                             \
    }                                                                                       \
                                                                                            \
    static int prefix##Yield(prefix##AVL avl, prefix##Entry *ret) {                         \
        int initialized = avl->generator.initialized;                                       \
        prefix##AVLNode node;                                                               \
                                                                                            \
        node = NULL;                                                                        \
                                                                                            \
        if (!initialized && ret) {                                                          \
                                                                                            \
            if (!avl->root)                                                                 \
                return -1;                                                                  \
                                                                                            \
            avl->generator.stack = stackNew(prefix##AVLNode, NULL, &__##prefix##StClone);   \
            avl->generator.initialized = 1;                                                 \
            prefix##StackMin(avl->generator.stack, avl->root);                              \
        }                                                                                   \
                                                                                            \
        if (!ret) {                                                                         \
            if (initialized) {                                                              \
                stackDestroy(prefix##AVLNode, avl->generator.stack);                        \
                avl->generator.initialized = 0;                                             \
            }                                                                               \
        } else {                                                                            \
            stackPop(prefix##AVLNode, avl->generator.stack, &node);                         \
            prefix##StackMin(avl->generator.stack, node->right);                            \
                                                                                            \
            *ret = (prefix##Entry)malloc(sizeof(struct prefix##Entry_s));                   \
            (*ret)->key = node->key;                                                        \
            (*ret)->value = node->value;                                                    \
                                                                                            \
            if (stackIsEmpty(prefix##AVLNode, avl->generator.stack)) {                      \
                prefix##Yield(avl, NULL);                                                   \
                return 1;                                                                   \
            }                                                                               \
        }                                                                                   \
                                                                                            \
        return 0;                                                                           \
    }                                                                                       \
                                                                                            \
    static int prefix##GenerateFrom(prefix##AVL avl, keyType key) {                         \
        prefix##AVLNode node = avl->root;                                                   \
        prefix##AVLNodeStack stack = avl->generator.stack;                                  \
        int cmp;                                                                            \
                                                                                            \
        if (avl->generator.initialized)                                                     \
           prefix##Yield(avl, NULL);                                                        \
                                                                                            \
        avl->generator.stack = stackNew(prefix##AVLNode, NULL, &__##prefix##StClone);       \
        avl->generator.initialized = 1;                                                     \
                                                                                            \
        while (node){                                                                       \
            cmp = avl->compare(key, node->key);                                             \
                                                                                            \
            if (cmp > 0) {                                                                  \
                node =  node->right;                                                        \
            } else if (cmp < 0) {                                                           \
                stackPush(prefix##AVLNode, stack, node);                                    \
                node = node->left;                                                          \
            } else {                                                                        \
                stackPush(prefix##AVLNode, stack, node);                                    \
                return 0;                                                                   \
            }                                                                               \
        }                                                                                   \
                                                                                            \
        prefix##Yield(avl, NULL);                                                           \
        return -1;                                                                          \
    }                                                                                       \
                                                                                            \
    static int prefix##RewindGenerator(prefix##AVL avl) {                                   \
        return prefix##Yield(avl, NULL);                                                    \
    }                                                                                       \
                                                                                            \
                                                                                            \
    static prefix##AVLNode __##prefix##NodeNew(keyType key, valueType value) {              \
        prefix##AVLNode node;                                                               \
                                                                                            \
        node = (prefix##AVLNode)malloc(sizeof(struct prefix##AVLNode_s));                   \
                                                                                            \
        if (!node)                                                                          \
            return NULL;                                                                    \
                                                                                            \
        node->key = key;                                                                    \
        node->value = value;                                                                \
        node->balance = 0;                                                                  \
        node->left = node->right = NULL;                                                    \
                                                                                            \
        return node;                                                                        \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##NodeClone(prefix##AVL avl, prefix##AVLNode node) {   \
        prefix##AVLNode newNode;                                                            \
                                                                                            \
        newNode = NULL;                                                                     \
                                                                                            \
        if (node) {                                                                         \
                                                                                            \
            newNode = __##prefix##NodeNew(node->key, node->value);                          \
                                                                                            \
            newNode->balance = node->balance;                                               \
            newNode->right = __##prefix##NodeClone(avl, node->right);                       \
            newNode->left = __##prefix##NodeClone(avl, node->left);                         \
        }                                                                                   \
                                                                                            \
        return newNode;                                                                     \
    }                                                                                       \
                                                                                            \
    static prefix##AVL prefix##New(int (*compare)(keyType, keyType),                        \
                                   valueType (*collision)(valueType, valueType)) {          \
        prefix##AVL avl;                                                                    \
                                                                                            \
        avl = (prefix##AVL)malloc(sizeof(struct prefix##AVL_s));                            \
                                                                                            \
        if (!avl)                                                                           \
            return NULL;                                                                    \
                                                                                            \
        avl->root = NULL;                                                                   \
        avl->compare = compare;                                                             \
        avl->collision = collision;                                                         \
        avl->generator.initialized = 0;                                                     \
        avl->generator.stack = NULL;                                                        \
                                                                                            \
        return avl;                                                                         \
    }                                                                                       \
                                                                                            \
    static prefix##AVL prefix##Clone(prefix##AVL avl) {                                     \
        prefix##AVL newAvl;                                                                 \
                                                                                            \
        newAvl = prefix##New(avl->compare,                                                  \
                             avl->collision);                                               \
                                                                                            \
        newAvl->root = __##prefix##NodeClone(newAvl, avl->root);                            \
                                                                                            \
        return newAvl;                                                                      \
    }                                                                                       \
                                                                                            \
    static void __##prefix##DestroyNode(void (*deleteKey)(keyType),                         \
                                        void (*deleteValue)(valueType),                     \
                                        prefix##AVLNode node) {                             \
                                                                                            \
        if (node) {                                                                         \
            if (deleteKey)                                                                  \
                deleteKey(node->key);                                                       \
            if (deleteValue)                                                                \
                deleteValue(node->value);                                                   \
            __##prefix##DestroyNode(deleteKey, deleteValue, node->left);                    \
            __##prefix##DestroyNode(deleteKey, deleteValue, node->right);                   \
            free(node);                                                                     \
        }                                                                                   \
                                                                                            \
        return;                                                                             \
    }                                                                                       \
                                                                                            \
    static void prefix##Destroy(prefix##AVL avl,                                            \
                                void (*deleteKey)(keyType),                                 \
                                void (*deleteValue)(valueType)) {                           \
                                                                                            \
        __##prefix##DestroyNode(deleteKey, deleteValue, avl->root);                         \
        avl->root = NULL;                                                                   \
        if (avl->generator.initialized)                                                     \
            prefix##Yield(avl, NULL);                                                       \
        free(avl);                                                                          \
                                                                                            \
        return;                                                                             \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##RotateRight(prefix##AVLNode node) {                  \
        prefix##AVLNode auxNode;                                                            \
                                                                                            \
        auxNode = node->left;                                                               \
        node->left = auxNode->right;                                                        \
        auxNode->right = node;                                                              \
                                                                                            \
        return auxNode;                                                                     \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##RotateLeft(prefix##AVLNode node) {                   \
        prefix##AVLNode auxNode;                                                            \
                                                                                            \
        auxNode = node->right;                                                              \
        node->right = auxNode->left;                                                        \
        auxNode->left = node;                                                               \
                                                                                            \
        return auxNode;                                                                     \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##BalanceRight(prefix##AVLNode node, int *growth) {    \
        int balance;                                                                        \
                                                                                            \
        balance = ++node->balance;                                                          \
                                                                                            \
        if (balance > 1) {                                                                  \
            if (node->right->balance == 1) {                                                \
                /* Simple left rotation */                                                  \
                node = __##prefix##RotateLeft(node);                                        \
                node->left->balance = 0;                                                    \
            } else {                                                                        \
                /* Double rotation (right child goes right, this goes left) */              \
                node->right = __##prefix##RotateRight(node->right);                         \
                node = __##prefix##RotateLeft(node);                                        \
                                                                                            \
                if (node->balance == -1) {                                                  \
                    node->left->balance = 0;                                                \
                    node->right->balance = 1;                                               \
                } else if (node->balance == 1) {                                            \
                    node->left->balance = -1;                                               \
                    node->right->balance = 0;                                               \
                } else {                                                                    \
                    node->left->balance = 0;                                                \
                    node->right->balance = 0;                                               \
                }                                                                           \
            }                                                                               \
                                                                                            \
            node->balance = 0;                                                              \
            *growth = 0;                                                                    \
        } else if (balance == 0) {                                                          \
            *growth = 0;                                                                    \
        }                                                                                   \
                                                                                            \
        return node;                                                                        \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##BalanceLeft(prefix##AVLNode node, int *growth) {     \
        int balance;                                                                        \
                                                                                            \
        balance = --node->balance;                                                          \
                                                                                            \
        if (balance < -1) {                                                                 \
            if (node->left->balance == -1) {                                                \
                /* Simple right rotation */                                                 \
                node = __##prefix##RotateRight(node);                                       \
                node->right->balance = 0;                                                   \
            } else {                                                                        \
                /* Double rotation (left child goes left, this goes right) */               \
                node->left = __##prefix##RotateLeft(node->left);                            \
                node = __##prefix##RotateRight(node);                                       \
                                                                                            \
                if (node->balance == -1) {                                                  \
                    node->left->balance = 0;                                                \
                    node->right->balance = 1;                                               \
                } else if (node->balance == 1) {                                            \
                    node->left->balance = -1;                                               \
                    node->right->balance = 0;                                               \
                } else {                                                                    \
                    node->left->balance = 0;                                                \
                    node->right->balance = 0;                                               \
                }                                                                           \
            }                                                                               \
                                                                                            \
            node->balance = 0;                                                              \
            *growth = 0;                                                                    \
        } else if (balance == 0) {                                                          \
            *growth = 0;                                                                    \
        }                                                                                   \
                                                                                            \
        return node;                                                                        \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##Insert(prefix##AVL      avl,                         \
                                             prefix##AVLNode node,                          \
                                             prefix##AVLNode *newNode,                      \
                                             int             *growth,                       \
                                             int             *col) {                        \
        int cmp;                                                                            \
                                                                                            \
        cmp = avl->compare((*newNode)->key, node->key);                                     \
                                                                                            \
        if (cmp == 0){                                                                      \
            *col = 1;                                                                       \
            if (avl->collision)                                                             \
                node->value = avl->collision(node->value, (*newNode)->value);               \
            *newNode = node;                                                                \
            *growth = 0;                                                                    \
        } else if (cmp > 0) {                                                               \
                                                                                            \
            if (!node->right) {                                                             \
                *col = 0;                                                                   \
                node->right = *newNode;                                                     \
                                                                                            \
                if (++node->balance)                                                        \
                    *growth = 1;                                                            \
                                                                                            \
            } else {                                                                        \
                node->right =__##prefix##Insert(avl, node->right, newNode, growth, col);    \
                                                                                            \
                if (*growth) {                                                              \
                    node = __##prefix##BalanceRight(node, growth);                          \
                }                                                                           \
            }                                                                               \
                                                                                            \
        } else {                                                                            \
                                                                                            \
            if (!node->left) {                                                              \
                *col = 0;                                                                   \
                node->left = *newNode;                                                      \
                                                                                            \
                if (--node->balance)                                                        \
                    *growth = 1;                                                            \
                                                                                            \
            } else {                                                                        \
                node->left = __##prefix##Insert(avl, node->left, newNode, growth, col);     \
                                                                                            \
                if (*growth) {                                                              \
                    node = __##prefix##BalanceLeft(node, growth);                           \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
                                                                                            \
        return node;                                                                        \
    }                                                                                       \
                                                                                            \
    static int __##prefix##InsertFind(prefix##AVL avl,                                      \
                                    keyType key,                                            \
                                    valueType value,                                        \
                                    prefix##AVLNode *ret) {                                 \
        prefix##AVLNode newNode, temp;                                                      \
        int growth, col;                                                                    \
                                                                                            \
        growth = 0;                                                                         \
        col = 0;                                                                            \
                                                                                            \
        temp = newNode = __##prefix##NodeNew(key, value);                                   \
                                                                                            \
        if (!newNode)                                                                       \
            return -1;                                                                      \
                                                                                            \
        if (!avl->root){                                                                    \
            col = 0;                                                                        \
            avl->root = newNode;                                                            \
        }                                                                                   \
        else {                                                                              \
            avl->root = __##prefix##Insert(avl, avl->root, &newNode, &growth, &col);        \
        }                                                                                   \
                                                                                            \
        if (avl->generator.initialized)                                                     \
            prefix##Yield(avl, NULL);                                                       \
                                                                                            \
        *ret = newNode;                                                                     \
                                                                                            \
        if (temp != newNode){                                                               \
            free(temp);                                                                     \
            return 1;                                                                       \
        }                                                                                   \
        else {                                                                              \
            return 0;                                                                       \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    static int prefix##Insert(prefix##AVL avl, keyType key, valueType value) {              \
        prefix##AVLNode ret;                                                                \
                                                                                            \
        return __##prefix##InsertFind(avl, key, value, &ret);                               \
    }                                                                                       \
                                                                                            \
    static int prefix##InsertFind(prefix##AVL avl,                                          \
                                  keyType key,                                              \
                                  valueType value,                                          \
                                  valueType *ret) {                                         \
        prefix##AVLNode node;                                                               \
                                                                                            \
        if (__##prefix##InsertFind(avl, key, value, &node) == -1)                           \
            return -1;                                                                      \
                                                                                            \
        *ret = node->value;                                                                 \
                                                                                            \
        return 0;                                                                           \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode __##prefix##Find(int (*compare)(keyType, keyType),               \
                                          prefix##AVLNode node,                             \
                                          keyType key) {                                    \
        int cmp;                                                                            \
                                                                                            \
        if (!node)                                                                          \
            return NULL;                                                                    \
                                                                                            \
        cmp = compare(key, node->key);                                                      \
                                                                                            \
        if (cmp > 0) {                                                                      \
            return __##prefix##Find(compare, node->right, key);                             \
        } else if (cmp < 0) {                                                               \
            return __##prefix##Find(compare, node->left, key);                              \
        } else {                                                                            \
            return node;                                                                    \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    static int prefix##Find(prefix##AVL avl, keyType key, valueType *ret) {                 \
        prefix##AVLNode node;                                                               \
                                                                                            \
        node = __##prefix##Find(avl->compare, avl->root, key);                              \
                                                                                            \
        if (!node)                                                                          \
            return -1;                                                                      \
                                                                                            \
        *ret = node->value;                                                                 \
                                                                                            \
        return 0;                                                                           \
    }                                                                                       \
                                                                                            \
    static int prefix##Exists(prefix##AVL avl, keyType key) {                               \
                                                                                            \
        if (!__##prefix##Find(avl->compare, avl->root, key))                                \
            return 0;                                                                       \
        else                                                                                \
            return 1;                                                                       \
    }                                                                                       \
                                                                                            \
    static int prefix##Update(prefix##AVL avl, keyType key, valueType value) {              \
        prefix##AVLNode node;                                                               \
                                                                                            \
        node = __##prefix##Find(avl->compare, avl->root, key);                              \
                                                                                            \
        if (!node)                                                                          \
            return -1;                                                                      \
                                                                                            \
        node->value = value;                                                                \
                                                                                            \
        return 0;                                                                           \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode prefix##GetLeftChild(prefix##AVLNode node) {                     \
        return node->left;                                                                  \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode prefix##GetRightChild(prefix##AVLNode node) {                    \
        return node->right;                                                                 \
    }                                                                                       \
                                                                                            \
    static prefix##AVLNode prefix##GetRoot(prefix##AVL avl) {                               \
        return avl->root;                                                                   \
    }                                                                                       \
                                                                                            \
    static valueType prefix##GetNodeValue(prefix##AVLNode node) {                           \
        return node->value;                                                                 \
    }                                                                                       \

#endif
