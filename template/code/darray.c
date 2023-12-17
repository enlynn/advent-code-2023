#include "darray.h"
#include "chibi_core.h"

#define HEADER_SIZE (sizeof(u64) * darray_field_length)

fn_internal u64* 
ptr_to_header(void* Ptr)
{
    return (u64*)Ptr - darray_field_length;
}

fn_internal void* 
header_to_ptr(u64* Header)
{
    return (void*)(Header + darray_field_length);
}

void* chibi_darray_init(u64 Capacity, u64 Stride)
{
    u64 ArraySize = Capacity * Stride;
    u64* Ptr = (u64*)mem_alloc(u8, HEADER_SIZE + ArraySize);
    
    //Assert(Ptr);
    mem_zero(Ptr, HEADER_SIZE + ArraySize);
    
    Ptr[darray_capacity] = Capacity;
    Ptr[darray_length]   = 0;
    Ptr[darray_stride]   = Stride;
    
    return header_to_ptr(Ptr);
}

void  chibi_darray_free(void* Array)
{
    if (!Array) return;
    void* Header = ptr_to_header(Array);
    mem_free(Header);
}

void* chibi_darray_resize(void* OldArray)
{
    u64 OldCapacity = chibi_darray_field_get(OldArray, darray_capacity);
    u64 OldStride   = chibi_darray_field_get(OldArray, darray_stride);
    u64 OldLength   = chibi_darray_field_get(OldArray, darray_length);
    
    void* NewArray = chibi_darray_init(OldCapacity * DARRAY_DEFAULT_RESIZE_FACTOR, OldStride);
    chibi_darray_field_set(NewArray, OldLength, darray_length);
    
    u64 CopySize = OldStride * OldLength;
    mem_copy(NewArray, OldArray, CopySize);
    
    chibi_darray_free(OldArray);
    
    return NewArray;
}

void* chibi_darray_push(void* Array, void* ValuePtr)
{
    u64 Length = darray_len(Array);
    u64 Stride = chibi_darray_field_get(Array, darray_stride);
    if (Length + 1 >= darray_cap(Array))
    {
        Array = chibi_darray_resize(Array);
    }
    
    u64 Addr = (u64)Array;
    Addr += (Length * Stride);
    mem_copy((void*)Addr, ValuePtr, Stride);
    
    chibi_darray_field_set(Array, Length + 1, darray_length);
    
    return Array;
}

void* 
chibi_darray_push_at(void* Array, u64 Index, void* Destination)
{
    u64 Length = darray_len(Array);
    u64 Stride = chibi_darray_field_get(Array, darray_stride);
    
    if (Index >= Length)
    {
        //LOG_ERROR("Attempting to insert into dynamic array with length (%d) at index (%d).", Length, Index);
        return Array;
    }
    
    if (Length + 1 >= darray_cap(Array))
    {
        Array = chibi_darray_resize(Array);
    }
    
    u64 Addr = (u64)Array;
    
    if (Index != Length - 1)
    { // Make room for the new element
        mem_copy((void*)(Addr + ((Index + 1) * Stride)),
                 (void*)(Addr + (Index * Stride)),
                 Stride * (Length - Index));
    }
    
    // Insert the element
    mem_copy((void*)(Addr + (Index * Stride)), Destination, Stride);
    
    chibi_darray_field_set(Array, Length + 1, darray_length);
    
    return Array;
}

void 
chibi_darray_pop(void* Array, void* Destination)
{
    u64 Length = darray_len(Array);
    u64 Stride = chibi_darray_field_get(Array, darray_stride);
    
    u64 Addr = (u64)Array;
    Addr += ((Length - 1) * Stride);
    
    if (Destination)
    {
        mem_copy(Destination, (void*)Addr, Stride);
    }
    
    chibi_darray_field_set(Array, Length - 1, darray_length);
}

void* 
chibi_darray_pop_at(void* Array, u64 Index, void* Destination)
{
    u64 Length = darray_len(Array);
    u64 Stride = chibi_darray_field_get(Array, darray_stride);
    
    if (Index >= Length)
    {
        //LOG_ERROR("Attempting to pop from dynamic array with length (%d) at index (%d).", Length, Index);
        return Array;
    }
    
    u64 Addr = (u64)Array;
    
    if (Destination)
    {
        mem_copy(Destination, (void*)(Addr + (Index * Stride)), Stride);
    }
    
    if (Index != Length - 1) {
        mem_copy((void*)(Addr + (Index * Stride)),
                 (void*)(Addr + ((Index + 1) * Stride)),
                 Stride * (Length - Index));
    }
    
    chibi_darray_field_set(Array, Length - 1, darray_length);
    
    return Array;
}

u64 
chibi_darray_field_get(void* Array, u64 Field)
{
    u64* Header = ptr_to_header(Array);
    return Header[Field];
}

u64 
chibi_darray_field_set(void* Array, u64 Value, u64 Field)
{
    u64* Header = ptr_to_header(Array);
    Header[Field] = Value;
    return Header[Field];
}
