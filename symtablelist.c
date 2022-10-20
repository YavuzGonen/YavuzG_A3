#include "stdio.h"
#include "symtable.h"
#include "string.h"
#include "stddef.h"
#include "stdlib.h"
#include "assert.h"

struct Node {
    const char *key;
    void *value;
    struct Node *next;
};

struct SymTable {
    struct Node *first;
    size_t length;
};

SymTable_T SymTable_new(void) {
    SymTable_T p = calloc(1, sizeof(SymTable_T));
    if(p == NULL) return NULL;
    p->length = 0;
    return p;
}

void SymTable_free(SymTable_T oSymTable) {
    struct Node* tracer1 = oSymTable->first;
    struct Node* tracer2;
    assert(oSymTable != NULL);
    while(tracer1 != NULL) {
        tracer2 = tracer1->next;
        free(tracer1);
        tracer1 = tracer2;
    }
    oSymTable->length = 0;
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    struct Node *psNewNode = (struct Node*)malloc(sizeof(struct Node));
    if (psNewNode == NULL) return 0;
    
    if(SymTable_contains(oSymTable, pcKey)) return 0;
    
    assert(oSymTable != NULL);
    psNewNode->key = (const char*)malloc(strlen(pcKey) + 1);
    if (psNewNode->key == NULL) return 0;
    
    psNewNode->value = (void*)pvValue;    
    strcpy((char*)psNewNode->key, pcKey);
    psNewNode->next = oSymTable->first;
    oSymTable->first = psNewNode;
    oSymTable->length++;
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    void *oldValue;
    struct Node* tracer = oSymTable->first;
    assert(oSymTable != NULL); 
    while(tracer != NULL) {
        if(!strcmp(tracer->key,pcKey)) {
            oldValue = tracer->value;
            tracer->value = (void*)pvValue;
            return oldValue;
        }
        tracer = tracer->next;
    }
    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct Node* tracer = oSymTable->first; 
    assert(oSymTable != NULL);
    while(tracer != NULL) {
        if(!strcmp(tracer->key,pcKey)) return 1;
        tracer = tracer->next;
    }
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct Node* tracer = oSymTable->first; 
    assert(oSymTable != NULL);
    while(tracer != NULL) {
        if(!strcmp(tracer->key,pcKey)) return tracer->value;
        tracer = tracer->next;
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    void *output;
    struct Node* tracer1; 
    struct Node* tracer2;
    assert(oSymTable != NULL);
    tracer1 = oSymTable->first; 
    tracer2 = tracer1->next;
    while(tracer2 != NULL) {
        if(!strcmp(tracer2->key,pcKey)) {
            output = tracer2->value;
            tracer1->next = tracer2->next;
            free(tracer2);
            oSymTable->length--;
            return output;
        }
        tracer2 = tracer2->next;
        tracer1 = tracer1->next;
    }
    return NULL;
}

void SymTable_map(SymTable_T oSymTable, void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra), 
const void *pvExtra) {
    struct Node* tracer = oSymTable->first;
    assert(oSymTable != NULL);
    while(tracer != NULL) {
        pfApply(tracer->key, tracer->value, (void*)pvExtra);
        tracer = tracer->next;
    }
}