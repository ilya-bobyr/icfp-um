#include "nativeCode.h"

#include "memoryManager.h"
#include "jumpTable.h"

#include <boost/assert.hpp>


nativeCode * nativeCode::create(memoryManager & mm, size_t bytes,
                                size_t jumpTableSlotCount)
{
    void * p = mm.alloc(sizeof(nativeCode) + bytes, false);

    /*
     * While there is no nativeCode::operator new(...), if '::' is not specified 
     * here, the compiler will look for nativeCode::operator delete(...) first  
     * (N3376 section 5.3.4 paragraph 19).  As one is actually declared it is 
     * the only one that is checked for a match with the placement new 
     * argument list.  As the argument list does not match it is not called.
     *
     * By explicitly qualifying new here we make compiler look for a matching 
     * delete in the global scope only.
     *
     * MS C++ 10 will give a warning.  It makes sense in a general case as if 
     * the constructor throws it will result in a leak.
     */
    nativeCode * res = ::new(p) nativeCode();

    res->_jumpTable = jumpTable::create(mm, jumpTableSlotCount);

    return res;
}

nativeCode::nativeCode() throw()
    : _jumpTable(nullptr)
{ }

nativeCode::~nativeCode()
{
    BOOST_ASSERT(_jumpTable == nullptr);
}

void nativeCode::destroy(memoryManager & mm)
{
    if (_jumpTable)
    {
        _jumpTable->destroy(mm);
        _jumpTable = nullptr;
    }

    nativeCode::~nativeCode();

    mm.release(this);
}

char * nativeCode::begin()
{
    return reinterpret_cast<char *>(this) + sizeof(nativeCode);
}

jumpTable * nativeCode::jumpTable()
{
    return _jumpTable;
}

const jumpTable * nativeCode::jumpTable() const
{
    return _jumpTable;
}
