#ifndef __MEMORY_MANAGER__H
#define __MEMORY_MANAGER__H

#include "exceptions/systemError.h"

#include <boost/utility.hpp>

#include <vector>
#include <Stdexcept>


/*
 * As we will be allocating and deallocating a lot of small chunks of memory we 
 * will use a custom memory manager.
 *
 * It will allocate large chunks of memory, split them into pieces and provide 
 * those upon request.
 */
class memoryManager: boost::noncopyable
{
public:
    memoryManager();
    ~memoryManager();

    void * alloc(size_t size, bool zero = true);
    void release(void * p);

private:
    struct _freeChunk;
    struct _allocedChunk;

    static const size_t _allocationSize = 1024 * 1024;
    static const size_t _minChunkSize = 32;
    static const size_t _maxChunkSize = _allocationSize;

    /*
     * All the blocks of memory allocated so far, every one of size 
     * _allocationSize. 
     */
    typedef std::vector<void *> _blocks_type;
    _blocks_type _blocks;

    /*
     * Pointers to the first free chunk in a single linked list of chunks of 
     * size specified in the corresponding entry in _allChunkSizes.
     * Zero means that corresponding list is empty at the moment.  Use 
     * prepareNewBlock(...) to allocate new list of free chunks of an 
     * appropriate size.
     */
    typedef std::vector<_freeChunk *> _allChunks_type;
    _allChunks_type _allChunks;

    /*
     * Sizes of chunks in the corresponding lists pointed to by _allChunks.
     * Sorted in ascending order.
     * First entry is _minChunkSize.  Last entry is at least as large as 
     * _maxChunkSize.
     */
    typedef std::vector<size_t> _allChunkSizes_type;
    _allChunkSizes_type _allChunkSizes;

    static const _allChunks_type::size_type _bigChunkIndex
        = ~static_cast<_allChunks_type::size_type>(0);

    _freeChunk * prepareNewBlock(size_t chunkSize);

    /*
     * Makes sure that _allChunkSizes is a multiple of the system page size.  
     * Throws logic_error when the above assertion is not true.
     */
    void checkAllocationSize() throw(std::logic_error);

    /*
     * Memory is always zeroed out, so zero value is ignored.
     * Returns a block of memory at least of the specified size.
     */
    void * allocHelper(size_t size, bool zero) throw(exceptions::systemError);

    /* Releases a block of memory. */
    void releaseHelper(void * p) throw(exceptions::systemError);
};

#endif /* __MEMORY_MANAGER__H */
