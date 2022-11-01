/******************************************************************/
/* symtable.h                                                     */
/* Author: Yavuz Gonen                                            */
/******************************************************************/

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include <stddef.h>
/* struct SymTable stores pairings of strings and values attatched to
those strings */
struct SymTable;
/* SymTable_T is an alias for SymTable */
typedef struct SymTable *SymTable_T;

/* Returns a new SymTable object with no bindings, or NULL if insufficient memory is available */
SymTable_T SymTable_new(void);

/* Frees all memory occupied by oSymTable. */
void SymTable_free(SymTable_T oSymTable);

/* Returns the number of entries oSymTables has as size_t */
size_t SymTable_getLength(SymTable_T oSymTable);

/* Adds pcKey into oSymTable and assigns pcKey the value pvValue. Returns int 1
if the addition was successful and int 0 if unsuccessful due to no more memory
being left. Returns 0 if pcKey is already in oSymTable */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue);

/* Changes the value previously assigned to pcKey inside oSymTable and changes the value to pvValue. 
Returns the previous value of pcKey or NULL if pcKey isn't in oSymtable */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue);

/* Returns 1 if oSymTable has an entry with pcKey as its key. Returns 0 if not. */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Returns the value assigned to pcKey inside oSymTable or NULL if pcKey isn't in oSymtable */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* If oSymTable contains an entry with key pcKey, then removes that entry from oSymTable 
and returns the entry's value. Otherwise the function must not change oSymTable and return NULL. */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Applies the function (*pfApply)(pcKey, pvValue, pvExtra) for each pcKey/pvValue 
binding in oSymTable, passing pvExtra as an extra parameter. */
void SymTable_map(SymTable_T oSymTable, void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra), 
const void *pvExtra);

#endif