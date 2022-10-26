#include "stdio.h"
#include "symtable.h"
#include "string.h"
#include "stddef.h"
#include "stdlib.h"
#include "assert.h"

static const size_t auBucketCounts[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};
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

struct Binding {
    char *key;
    void *value;
    struct Binding *next;
};

struct SymTable {
    struct Binding **buckets;
    size_t length;
    size_t max;
};

SymTable_T SymTable_new(void) {
    SymTable_T newHashTable = (SymTable_T)calloc(1, sizeof(SymTable_T));
    newHashTable->length = 0;
    newHashTable->max = auBucketCounts[0];
    newHashTable->buckets = (struct Binding**)calloc(auBucketCounts[0], sizeof(struct Binding*));
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

static int expand(SymTable_T oSymTable) {
    struct Binding **newBuckets;
    struct Binding *oldTracer;
    size_t newMax;
    size_t newHash;
    size_t i;
    assert(oSymTable != NULL);
    if(oSymTable->length == auBucketCounts[numBucketCounts-1]) return 0;
    for(i = 0; i < numBucketCounts-1; i++) {
        if(oSymTable->length == auBucketCounts[i]) {
            newMax = auBucketCounts[i+1];
            break;
        }
    }
    newBuckets = (struct Binding**)calloc(1, sizeof(struct Binding*) * newMax);
    if (newBuckets == NULL) return 0;
    
    for(i=0; i<oSymTable->max; i++) {
        oldTracer = oSymTable->buckets[i];
        while(oldTracer != NULL) {
            newHash = SymTable_hash(oldTracer->key,newMax);
            newBuckets[newHash] = oldTracer;
        }
    }

    free(oSymTable->buckets);
    oSymTable->max = newMax;
    oSymTable->buckets = newBuckets;
    return 1;
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    int out;
    struct Binding *newEntry;
    int hash = SymTable_hash(pcKey, oSymTable->max);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    newEntry = (struct Binding*)malloc(sizeof(struct Binding*));
    assert(newEntry != NULL);

    if(SymTable_contains(oSymTable, pcKey)) return 0;
    
    if(oSymTable->length == oSymTable->max) {
        out = expand(oSymTable);
        if(!out) return out;
    }

    newEntry->key = (char*)malloc(strlen(pcKey) + 1);
    if (newEntry->key == NULL) return 0;
    strcpy((char*)newEntry->key, pcKey);
    newEntry->value = (void*)pvValue;

    if(oSymTable->buckets[hash] == NULL) oSymTable->buckets[hash] = newEntry;
    else {
        newEntry->next = oSymTable->buckets[hash]->next;
        oSymTable->buckets[hash] = newEntry;
    }
    oSymTable->length++;
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    void *output;
    int hash = SymTable_hash(pcKey, oSymTable->max);
    struct Binding *trace;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    trace = oSymTable->buckets[hash];
    if(oSymTable->buckets[hash] == NULL) return NULL;
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
    int hash = SymTable_hash(pcKey, oSymTable->max);
    struct Binding *trace = oSymTable->buckets[hash];
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    if(oSymTable->buckets[hash] == NULL) return 0;
    while(trace != NULL) {
        if(!strcmp(trace->key,pcKey)) return 1;
        trace = trace->next;
    }
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    int hash = SymTable_hash(pcKey, oSymTable->max);
    struct Binding *trace = oSymTable->buckets[hash];
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    if(oSymTable->buckets[hash] == NULL) return NULL;
    while(trace != NULL) {
        if(!strcmp(trace->key,pcKey)) return trace->value;
        trace = trace->next;
    }
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    void *output;
    int hash = SymTable_hash(pcKey, oSymTable->max);
    struct Binding* tracer1 = oSymTable->buckets[hash]; 
    struct Binding* tracer2 = tracer1->next;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    while(tracer2 != NULL) {
        if(!strcmp(tracer2->key,pcKey)) {
            output = tracer2->value;
            tracer1->next = tracer2->next;
            free(tracer2->key);
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
    size_t i;
    struct Binding* bucketTracer;
    for(i = 0; i < oSymTable->max; i++) {
        bucketTracer = oSymTable->buckets[i];
        while(bucketTracer != NULL) {
            pfApply(bucketTracer->key, bucketTracer->value, (void*)pvExtra);
            bucketTracer = bucketTracer->next;
        }
    }
}