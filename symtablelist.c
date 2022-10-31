/******************************************************************/
/* symtablelist.c                                                 */
/* Author: Yavuz Gonen                                            */
/******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "symtable.h"

/* a structure that has a char *key and void *value that connects 
it to the other node that comes after it with Node *next pointer */
struct Node {
    char *key; /* the string key of the node */
    void *value; /* the value the node stores for a key */
    struct Node *next; /* node that comes after current node */
};

/* a structure that stores a char *key and void *value */
struct SymTable {
    struct Node *first; /* the first node on the SymTable */
    size_t length; /* the number of nodes in the SymTable */
};

SymTable_T SymTable_new(void) {
    SymTable_T out = (SymTable_T)malloc(sizeof(struct SymTable));
    if(out == NULL) return NULL;
    out->length = 0;
    out->first = NULL;
    return out;
}

void SymTable_free(SymTable_T oSymTable) {
    struct Node* tracer;
    struct Node* temp;
    assert(oSymTable != NULL);
    for(tracer = oSymTable->first; tracer != NULL; tracer = temp) {
        temp = tracer->next;
        free(tracer->key);
        free(tracer);
    }
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    struct Node *psNewNode;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    if(SymTable_contains(oSymTable, pcKey)) return 0;

    psNewNode = (struct Node*)malloc(sizeof(struct Node));
    if (psNewNode == NULL) return 0;
    
    psNewNode->key = (char*)malloc(strlen(pcKey)+ 1);
    if (psNewNode->key == NULL) {
        free(psNewNode);
        return 0;
    }

    strcpy((char*)psNewNode->key, pcKey);
    psNewNode->value = (void*)pvValue;    

    psNewNode->next = oSymTable->first;
    oSymTable->first = psNewNode;
    oSymTable->length++;
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    void *oldValue;
    struct Node* tracer;
    struct Node* temp;
    assert(oSymTable != NULL); 
    assert(pcKey != NULL);

    for(tracer = oSymTable->first; tracer != NULL; tracer = temp) {
        temp = tracer->next;
        if(!strcmp(tracer->key,pcKey)) {
            oldValue = tracer->value;
            tracer->value = (void*)pvValue;
            return oldValue;
        }
    }
    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct Node* tracer; 
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    tracer = oSymTable->first; 
    while(tracer != NULL) {
        if(!strcmp(tracer->key,pcKey)) return 1;
        tracer = tracer->next;
    }
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct Node* tracer; 
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    tracer = oSymTable->first; 

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
    assert(pcKey != NULL);
    tracer1 = oSymTable->first;
    if(tracer1 == NULL) return NULL; 
    tracer2 = tracer1->next;

    if(!SymTable_contains(oSymTable, pcKey)) return NULL;

    if(!strcmp(tracer1->key,pcKey)) {
        output = tracer1->value;
        oSymTable->first = tracer1->next;
        free(tracer1->key);
        free(tracer1);
        oSymTable->length--;
        return output;
    }

    while(tracer2 != NULL) {
        if(!strcmp(tracer2->key,pcKey)) {
            output = tracer2->value;
            tracer1->next = tracer2->next;
            free(tracer2->key);
            free(tracer2);
            oSymTable->length--;
            return output;
        }
        tracer1 = tracer2;
        tracer2 = tracer2->next;
    }

    return NULL;
}

void SymTable_map(SymTable_T oSymTable, void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra), 
const void *pvExtra) {
    struct Node* tracer;
    assert(oSymTable != NULL);
    tracer = oSymTable->first;
    while(tracer != NULL) {
        pfApply(tracer->key, (void*)tracer->value, (void*)pvExtra);
        tracer = tracer->next;
    }
}