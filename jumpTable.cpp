#include "jumpTable.h"

#include "memoryManager.h"

jumpTable * jumpTable::create(memoryManager & mm, size_t slotCount)
{
    void * p = mm.alloc(slotCount * sizeof(void *), false);

    /*
     * See nativeCode::craete(...) implementation for an exmplanation why 
     * explicit '::' is required here.
     */
    return ::new (p) jumpTable();
}

jumpTable::~jumpTable()
{ }

void jumpTable::destroy(memoryManager & mm)
{
    jumpTable::~jumpTable();

    mm.release(this);
}

void ** jumpTable::begin()
{
    return reinterpret_cast<void **>(this);
}

void * const * jumpTable::begin() const
{
    return reinterpret_cast<void * const *>(this);
}

void * jumpTable::address(size_t i) const
{
    return begin()[i];
}
