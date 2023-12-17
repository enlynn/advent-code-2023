#ifndef _DARRAY_H_
#define _DARRAY_H_

#include "chibi_types.h"

typedef enum 
{
    darray_capacity,
    darray_length,
    darray_stride,
    darray_field_length,
} darray_fields;

#define DARRAY_DEFAULT_CAPACTIY      1
#define DARRAY_DEFAULT_RESIZE_FACTOR 2

// Initializes the array with the default capacity
#define darray_init(Type)              chibi_darray_init(DARRAY_DEFAULT_CAPACTIY, sizeof(Type))
// Initializes the array with the specified capacity
#define darray_reserve(Type, Capacity) chibi_darray_init(Capacity, sizeof(Type))
// Frees the array
#define darray_free(Array)             chibi_darray_free(Array)
// Retrieves the array length 
#define darray_len(Array)              chibi_darray_field_get(Array, darray_length)
// Sets the array's length.
#define darray_set_len(Array, Len)      chibi_darray_field_set(Array, Len, darray_length)
// Get the capacity of the array
#define darray_cap(Array)              chibi_darray_field_get(Array, darray_capacity)
// Set the capacity of the array
#define darray_set_cap(Array, Cap)      chibi_darray_field_set(Array, Cap, darray_capacity);
// Push an element into the array at the end of the list.
#define darray_push(Array, Value) {          \
        typeof(Value) Copy = Value;          \
        Array = chibi_darray_push(Array, &Copy);   \
    }
    
// Push an element into the array at the specified index
#define darray_push_at(Array, Index, Value) {        \
        typeof(Value) Copy = Value;                  \
        Array = chibi_darray_push_at(Array, Index, &Value); \
    }
    
// Removes an element from the end of the list
// If ValuePtr is valid, then the removed value is copied into it.
#define darray_pop(Array, ValuePtr)          chibi_darray_pop(Array, ValuePtr)
// Removes an element from the specified index
// If ValuePtr is valid, then the removed value is copied into it.
// A ptr to the array is returned.
#define darray_pop_at(Array, Index, ValuePtr) chibi_darray_pop_at(Array, Index, ValuePtr)

void* chibi_darray_init(u64 Capacity, u64 Stride);
void  chibi_darray_free(void* Array);

void* chibi_darray_resize(void* OldArray);

void* chibi_darray_push(void* Array, void* ValuePtr);
void* chibi_darray_push_at(void* Array, u64 Index, void* Destination);

void  chibi_darray_pop(void* Array, void* Destination);
void* chibi_darray_pop_at(void* Array, u64 Index, void* Destination);

u64 chibi_darray_field_get(void* Array, u64 Field);
u64 chibi_darray_field_set(void* Array, u64 Value, u64 Field);

#endif //_DARRAY_Hchibi_darray_init
