/******************************************************************/
/* symtablehash.c                                                 */
/* Author: Yavuz Gonen                                            */
/******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "symtable.h"

/* All possible maximum sizes a SymTable can have */
static const size_t auBucketCounts[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};
/* the total number of possible maximum sizes a SymTable can have */
static const size_t numBucketCounts = sizeof(auBucketCounts)/sizeof(auBucketCounts[0]);

/* Return a hash code for pcKey that is between 0 and uBucketCount-1, inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;
    assert(pcKey != NULL);
    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];
    return uHash % uBucketCount;
}

/* struct Binding contains a pairing of char *key and void *value. 
struct Binding points at another struct Binding that comes after it with
struct Binding *next */
struct Binding {
    /* the string key of the Binding */
    char *key; 
    /* the value the Binding stores for a key */
    void *value; 
    /* Binding that comes after current Binding */
    struct Binding *next; 
};

/* struct SymTable points at the first element of an array of pointers to a Binding
with struct Binding **buckets. Each pointer in the array can be used to start a linked 
list of Bindings. struct SymTable stores size_t length that counts the total number 
of Bindings stored within struct Symtable. struct SymTable also stores size_t max which is the 
maximum number of Bindings struct SymTable can store. */
struct SymTable {
    /* an array of pointers to the Bindings in SymTable */
    struct Binding **buckets;
    /* number of bindings in SymTable */
    size_t length;
    /* the maximum number of bindings SymTable can have */
    size_t max; 
};

/* increases the maximum number of bindings oSymTable can have attached to it */
static void SymTable_expand(SymTable_T oSymTable) {
    struct Binding **newBuckets;
    struct Binding **oldBuckets;
    struct Binding *oldTracer;
    struct Binding *temp;
    size_t newHash;
    size_t i;
    size_t newMax = 0;
    assert(oSymTable != NULL);

    /* breaks function if max cannot be increased */
    if(oSymTable->length == auBucketCounts[numBucketCounts-1]) return;
    
    /* gets the maximum number of bindings oSymTable can have */
    for(i = 0; i < numBucketCounts-1; i++) {
        if(oSymTable->length == auBucketCounts[i]) {
            newMax = auBucketCounts[i+1];
            break;
        }
    }

    oldBuckets = oSymTable->buckets;
    newBuckets = (struct Binding**)calloc(newMax, sizeof(struct Binding));
    if (newBuckets == NULL) return;

    /* rearranges the oldBuckets onto the newBucjets with new hash values */
    for(i=0; i < oSymTable->max; i++) {
        for(oldTracer = oldBuckets[i]; oldTracer != NULL; oldTracer = temp) {
            temp = oldTracer->next;
            newHash = SymTable_hash(oldTracer->key, newMax);
            oldTracer->next = newBuckets[newHash];
            newBuckets[newHash] = oldTracer;
        }
    }
    
    free(oldBuckets);
    oSymTable->max = newMax;
    oSymTable->buckets = newBuckets;
}

SymTable_T SymTable_new(void) {
    SymTable_T newHashTable = (SymTable_T)calloc(1, sizeof(struct SymTable));
    if(newHashTable == NULL) return NULL;
    
    newHashTable->length = 0;
    newHashTable->max = auBucketCounts[0];
    
    newHashTable->buckets = (struct Binding**)calloc(auBucketCounts[0], sizeof(struct Binding));
    if(newHashTable->buckets == NULL) return NULL;
    return newHashTable;
}

void SymTable_free(SymTable_T oSymTable) {
    size_t i;
    struct Binding* tracer;
    struct Binding* temp;
    assert(oSymTable != NULL);
    
    for(i = 0; i < oSymTable->max; i++) {
        tracer = oSymTable->buckets[i];
        while(tracer != NULL) {
            temp = tracer->next;
            free(tracer->key);
            free(tracer);
            tracer = temp;
        }
    }
    free(oSymTable->buckets);
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    struct Binding *newEntry;
    size_t hash;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    if(SymTable_contains(oSymTable, pcKey)) return 0;

    newEntry = (struct Binding*)malloc(sizeof(struct Binding));
    if (newEntry == NULL) return 0;
    newEntry->key = (char*)malloc(strlen(pcKey) + 1);
    if (newEntry->key == NULL) {
        free(newEntry);
        return 0;
    }

    /* if(oSymTable->length == oSymTable->max) SymTable_expand(oSymTable); */
    hash = SymTable_hash(pcKey, oSymTable->max);
    
    strcpy((char*)newEntry->key, pcKey);
    newEntry->value = (void*)pvValue;

    newEntry->next = oSymTable->buckets[hash];
    oSymTable->buckets[hash] = newEntry;
    oSymTable->length++;
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    void *output;
    size_t hash;
    struct Binding *trace;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    hash = SymTable_hash(pcKey, oSymTable->max);
    trace = oSymTable->buckets[hash];
    if(trace == NULL) return NULL;
    
    while(trace != NULL) {
        if(!strcmp(trace->key,pcKey)) {
            output = trace->value;
            trace->value = (void*)pvValue;
            return output;
        }
        trace = trace->next;
    }
    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    size_t hash;
    struct Binding *trace;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    hash = SymTable_hash(pcKey, oSymTable->max);
    trace = oSymTable->buckets[hash];

    if(trace == NULL) return 0;
    while(trace != NULL) {
        if(!strcmp(trace->key,pcKey)) return 1;
        trace = trace->next;
    }
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t hash;
    struct Binding *tracer;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    hash = SymTable_hash(pcKey, oSymTable->max);
    tracer = oSymTable->buckets[hash];
    if(tracer == NULL) return NULL;
    
    while(tracer != NULL) {
        if(!strcmp(tracer->key,pcKey)) return tracer->value;
        tracer = tracer->next;
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    void *output;
    size_t hash;
    struct Binding* tracer1; 
    struct Binding* tracer2;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    hash = SymTable_hash(pcKey, oSymTable->max);
    tracer1 = oSymTable->buckets[hash]; 
    if(tracer1 == NULL) return NULL;
    tracer2 = tracer1->next;

    if(!strcmp(tracer1->key,pcKey)) {
        output = tracer1->value;
        oSymTable->buckets[hash] = tracer2;
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
    size_t i;
    struct Binding* bucketTracer;
    assert(oSymTable != NULL);
    for(i = 0; i < oSymTable->max; i++) {
        bucketTracer = oSymTable->buckets[i];
        while(bucketTracer != NULL) {
            pfApply((const char*)bucketTracer->key, (void*)bucketTracer->value, (void*)pvExtra);
            bucketTracer = bucketTracer->next;
        }
    }
}