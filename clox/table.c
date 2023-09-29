#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

// decides which bucket the key should be found or be put
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {

    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;

    for (;;) {
        // decides where pointer to entry should be from index
        Entry* entry = &entries[index];

        // if bucket is empty or occupied with the correct key return the entry
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // Empty truly entry.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
            } else if (entry->key == key) {
            // We found the key.
            return entry;
        }

        // else the bucket has other occupant and we retry
        // run loop again with new index
        index = (index + 1) % capacity;
    }
}

bool tableGet(Table* table, ObjString* key, Value* value) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjsutCapacity(Table* table, int capactiy) {
    // allocate empty array into hash table 
    Entry* entries = ALLOCATE(Entry, capacity); 
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL; 
        entries[i].value = NIL_VAL;
    }

    table->count = 0;

    // need to re enter every value into new empty array
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        // copy value into new table using hashing functoin
            // find new hash value of entry and put it in new table
        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries; 
    table->capacity = capacity;
}

// adds given key/value pair to the given hash table 
// if entry for that key is already present, the new value overwrites the old value
// returns ture if new entry added
bool tableSet(Table* table, ObjString* key, Value value) {
    // if we dont have enough capacity to insert an item 
        // we reallocate and grow the array
    // adjust before its full. TABLE_MAX_LOAD is a hyperparameter
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }
    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
    if (table->count == 0) return false;

    // Find the entry.
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry.
    // this way we wont break the link between entries
        // when trying to find keys from hash
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

// copies table essentially
void tableAddAll(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}