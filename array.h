#ifndef __ARRAY__H
#define __ARRAY__H

#include <boost/utility.hpp>

class memoryManager;
class platter;
class jumpTable;
class nativeCode;

/*
 * Represents a um array along with all the supplementary data that would allow 
 * the array to be executed as a native code.
 */
class array: boost::noncopyable
{
private:
    /*
     * Instances of this class should be created via create(...) call.
     */
    array();

private:
    /*
     * scrollReader uses the array(size_t) constructor directly as it does not 
     * need to zero initialize all the memory as the create(...) call does.
     */
    friend class scrollReader;

    explicit array(size_t size) throw();
    ~array() throw();

    /* Delete via destroy(...) call. */
    void operator delete(void * p);

public:
    /*
     * size is the number of platters this array will hold.
     *
     * Memory is allocated using the specified memory manager.
     */
    static array * create(memoryManager & mm, size_t size);

    void destroy(memoryManager & mm) throw();

    /*
     * Copies all the patters but not the native code block.
     */
    array * clone(memoryManager & mm);

    size_t size() const;

    /*
     * This array was modified after native code for it was generated (if ever).  
     * Before using it as a zero array its native code should be regenerated.
     */
    bool dirty() const;
    void dirty(bool v);

    const platter * platters() const;
    platter * platters();

    const platter & operator[](size_t i) const;
    platter & operator[](size_t i);

    class jumpTable * jumpTable();
    const class jumpTable * jumpTable() const;

    class nativeCode * nativeCode();
    const class nativeCode * nativeCode() const;

private:
    static const size_t _plattersOffset;

private:
    /*
     * This create(...) is used by both public create(...) and clone(...).  
     * Clone can do with uninitialized memory thus saving on zeroing.
     */
    static array * createInt(memoryManager & mm, size_t size, bool zero);

    /*
     * context::generateNativeCode(...) fills in _jumpTable and _nativeCode 
     * directly.
     */
    friend class context;

    /* Number of platters in this array. */
    size_t _size;

    struct flag
    {
        enum value
        {
            /* dirty() value */
            dirty = 0x1
        };

    private:
        /* This struct is just a container for value. */
        flag();
    };
    /* This field might be accessed and modified from generated native code. */
    volatile size_t /* flag */ _flags;

    /*
     * If this array was ever used as a 0 array it should have a native code 
     * block associated with it.
     */
    class nativeCode * _nativeCode;

    /* Actual array of platters comes after the header. */
};

#endif /* __ARRAY__H */
