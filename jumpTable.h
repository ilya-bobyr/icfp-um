#ifndef __JUMP_TABLE__H
#define __JUMP_TABLE__H

#include <boost/utility.hpp>

class memoryManager;

/*
 * Contains addresses of instructions in a nativeCode block that coresponds to 
 * platters in an array.  Has the same size as the source array.
 *
 * Instances of this class are created by nativeCode::create(...).
 */
class jumpTable: boost::noncopyable
{
private:
    friend class context;
    friend class nativeCode;

    static jumpTable * create(memoryManager & mm, size_t slotCount);

private:
    ~jumpTable();
    /* Delete via destroy(...) call. */
    void operator delete(void * p);

public:
    void destroy(memoryManager & mm);

    void ** begin();
    void * const * begin() const;

    /*
     * Returns an address of the first instruction to execute when control 
     * finger points to platter i.
     */
    void * address(size_t i) const;

private:
    /*
     * Actual jump table goes here.  It is allocated by the 
     * generateNativeCode(...) call.
     */
};

#endif /* __JUMP_TABLE__H */
