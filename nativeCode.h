#ifndef __NATIVE_CODE__H
#define __NATIVE_CODE__H

#include <boost/utility.hpp>

class memoryManager;
class jumpTable;

/*
 * Represents a block of native code generated based on an array content.
 *
 * Instances of this class are created by context::generateNativeCode(...).
 */
class nativeCode: boost::noncopyable
{
private:
    friend class context;

    static nativeCode * create(memoryManager & mm, size_t bytes,
                               size_t jumpTableSlotCount);

private:
    nativeCode() throw();
    ~nativeCode();
    /* Delete via destroy(...) call. */
    void operator delete(void * p);

public:
    void destroy(memoryManager & mm);

    char * begin();

    class jumpTable * jumpTable();

    const class jumpTable * jumpTable() const;

private:
    class jumpTable * _jumpTable;

    /* Native code goes here. */
};

#endif /* __NATIVE_CODE__H */
