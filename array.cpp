#include "array.h"

#include "memoryManager.h"
#include "platter.h"
#include "jumpTable.h"
#include "nativeCode.h"

#include <boost/assert.hpp>
#include <boost/type_traits/alignment_of.hpp>

#include <cstring>


using namespace boost;

array::array(size_t size) throw()
    : _size(size)
    , _flags(0)
    , _nativeCode(nullptr)
{ }

array::~array() throw()
{
    BOOST_ASSERT(_nativeCode == nullptr);
}

array * array::create(memoryManager & mm, size_t size)
{
    return createInt(mm, size, true);
}

void array::destroy(memoryManager & mm) throw()
{
    if (_nativeCode)
    {
        _nativeCode->destroy(mm);
        _nativeCode = nullptr;
    }

    array::~array();

    mm.release(this);
}

array * array::clone(memoryManager & mm)
{
    array * res = createInt(mm, _size, false);

    memcpy(res->platters(), platters(), _size * sizeof(platter));

    return res;
}

size_t array::size() const
{
    return _size;
}

bool array::dirty() const
{
    return (_flags & flag::dirty) != 0;
}

void array::dirty(bool v)
{
    if (v)
        _flags |= flag::dirty;
    else
        _flags &= ~flag::dirty;
}

const platter * array::platters() const
{
    return reinterpret_cast<const platter *>
        (reinterpret_cast<const char *>(this) + _plattersOffset);
}

platter * array::platters()
{
    return reinterpret_cast<platter *>
        (reinterpret_cast<char *>(this) + _plattersOffset);
}

const platter & array::operator[](size_t i) const
{
    return platters()[i];
}

platter & array::operator[](size_t i)
{
    return platters()[i];
}

jumpTable * array::jumpTable()
{
    return _nativeCode ? _nativeCode->jumpTable() : nullptr;
}

const jumpTable * array::jumpTable() const
{
    return _nativeCode ? _nativeCode->jumpTable() : nullptr;
}

const nativeCode * array::nativeCode() const
{
    return _nativeCode;
}

nativeCode * array::nativeCode()
{
    return _nativeCode;
}

array * array::createInt(memoryManager & mm, size_t size, bool zero)
{
    size_t totalSize = _plattersOffset + size * sizeof(platter);
    void * p = mm.alloc(totalSize, zero);

    /*
     * See nativeCode::craete(...) implementation for an exmplanation why 
     * explicit '::' is required here.
     */
    return ::new(p) array(size);
}

const size_t array::_plattersOffset =
        (sizeof(array) + alignment_of<platter>::value - 1)
         / alignment_of<platter>::value
         * alignment_of<platter>::value;
