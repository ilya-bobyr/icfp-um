#include "memoryManager.h"

#include <boost/assert.hpp>
#include <boost/foreach.hpp>

#include <cstring>
#include <algorithm>
#include <sstream>

#include "windows.h"

using namespace std;

using namespace exceptions;

/*
 * === memoryManager::_freeChunk ===
 */

struct memoryManager::_freeChunk
{
    _freeChunk * next;
};


/*
 * === memoryManager::_allocedChunk ===
 */

struct memoryManager::_allocedChunk
{
    /*
     * An index in the _allChunks where this chunk will be returned to when it 
     * is freed.
     * Big chunks allocated individualy via allocHelper(...) have _bigChunkIndex 
     * as a value of this field.
     */
    _allChunks_type::size_type index;
};


/*
 * === memoryManager ===
 */

memoryManager::memoryManager()
{
    /*
     * Global checks.  Idealy this would be at the namespace level in a "class 
     * static constructor".
     */

    checkAllocationSize();

    static_assert(sizeof(_freeChunk) <= _minChunkSize,
                  "Smallest chunks should be big enough to contian an empty "
                  "chunk info.");

    static_assert(sizeof(_allocedChunk) < _minChunkSize,
                  "Smallest chunks should be big enough to contian a header "
                  "and something else.");

    /*
     * End of global checks.
     */

    size_t size = 1;

    while (size < _minChunkSize)
        size <<= 1;

    while (size <= _maxChunkSize)
    {
        _allChunkSizes.push_back(size);

        size <<= 1;
    }

    _allChunks.resize(_allChunkSizes.size());
}

memoryManager::~memoryManager()
{
    BOOST_FOREACH (void * p, _blocks)
        releaseHelper(p);

    _blocks.clear();

    _allChunks.clear();
    _allChunkSizes.clear();
}

void * memoryManager::alloc(size_t size, bool zero)
{
    /* Large allocations are forwarded to the default memory allocator. */
    if (size + sizeof(_allocedChunk) > _allChunkSizes.back())
    {
        void * block = allocHelper(size + sizeof(_allocedChunk), zero);

        _allocedChunk * header = reinterpret_cast<_allocedChunk *>(block);

        header->index = _bigChunkIndex;

        return reinterpret_cast<char *>(block) + sizeof(header);
    }

    /*
     * Round size up to the next power of 2 and include _allocedChunk size as we 
     * will add this header to the allocated block.
     */
    _allChunkSizes_type::iterator i =
        lower_bound(_allChunkSizes.begin(), _allChunkSizes.end(),
                    size + sizeof(_allocedChunk));

    size_t chunkSize = *i;

    size_t j = i - _allChunkSizes.begin();
    _freeChunk * freeChunk = _allChunks[j];

    if (!freeChunk)
        freeChunk = prepareNewBlock(chunkSize);

    _allChunks[j] = freeChunk->next;

    _allocedChunk * header =
        reinterpret_cast<_allocedChunk *>(freeChunk);
    header->index = j;

    char * body = reinterpret_cast<char *>(header) + sizeof(header);

    if (zero)
        memset(body, 0, size);

    return body;
}

void memoryManager::release(void * p)
{
    _allocedChunk * allocedChunk = reinterpret_cast<_allocedChunk *>
        (reinterpret_cast<char *>(p) - sizeof(_allocedChunk));

    if (allocedChunk->index == _bigChunkIndex)
    {
        releaseHelper(p);
        return;
    }

    size_t index = allocedChunk->index;
    BOOST_ASSERT(index < _allChunks.size());

    _freeChunk * freeChunk = reinterpret_cast<_freeChunk *>(allocedChunk);

    freeChunk->next = _allChunks[index];
    _allChunks[index] = freeChunk;
}

memoryManager::_freeChunk *
    memoryManager::prepareNewBlock(size_t chunkSize)
{
    void * block = allocHelper(_allocationSize, false);

    _blocks.push_back(block);

    _freeChunk * firstChunk = reinterpret_cast<_freeChunk *>(block);

    size_t count = _allocationSize / chunkSize;
    BOOST_ASSERT(count > 0);

    /* Link all the chunks in the new block. */
    {
        char * p = reinterpret_cast<char *>(firstChunk);
        for ( ; count > 0; p += chunkSize, --count)
        {
            reinterpret_cast<_freeChunk *>(p)->next =
                reinterpret_cast<_freeChunk *>(p + chunkSize);
        }
        /* Last chunk has no next. */
        reinterpret_cast<_freeChunk *>(p - chunkSize)->next = 0;
    }

    return firstChunk;
}

void memoryManager::checkAllocationSize() throw(logic_error)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    if (_allocationSize % si.dwPageSize != 0)
    {
        ostringstream ss;

        ss << "_allocationSize is not a multiple of the system page size: "
            << _allocationSize << " % " << si.dwPageSize << " != 0";

        throw logic_error(ss.str());
    }
}

void * memoryManager::allocHelper(size_t size, bool /* zero */) 
    throw(systemError)
{
    void * p = VirtualAlloc
        (0                        /* lpAddress */,
         size                     /* dwSize */,
         MEM_RESERVE | MEM_COMMIT /* flAllocationType */,
         PAGE_EXECUTE_READWRITE   /* flProtect */
        );

    if (!p)
        throw systemError(systemError::getLast);

    return p;
}

void memoryManager::releaseHelper(void * p) throw(systemError)
{
    BOOL res = VirtualFree(p, 0, MEM_RELEASE);

    if (!res)
        throw systemError(systemError::getLast);
}
