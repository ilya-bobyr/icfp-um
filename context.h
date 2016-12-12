#ifndef __CONTEXT__H
#define __CONTEXT__H

#include "platter.h"
#include "array.h"

#include "exceptions/invalidArrayIndex.h"
#include "exceptions/invalidOperatorFormat.h"

#include <boost/utility.hpp>

#include <array>
#include <vector>
#include <iosfwd>

class memoryManager;
class array;

/*
 * This class instance represents a universal machine context: a set of 
 * registers, an execution finger position and a collection of arrays of patters 
 * allocated at the moment.
 *
 * It also holds references to the input and output streams used by the machine.
 *
 * All the allocations are performed using the specified memory manager.
 */
class context: boost::noncopyable
{
public:
    context(memoryManager & mm, std::istream & is, std::ostream & os,
            array * zeroArray);

    /*
     * Executes the universal machine until it exits or something fails.
     *
     * May throw an exception if the machine enters an invalid state.
     */
    void run() throw(exceptions::invalidArrayIndex, 
                     exceptions::invalidOperatorFormat);

private:
    memoryManager & _mm;

    std::istream & _is;
    std::ostream & _os;

    typedef std::array<platter, 8> _registers_type;
    _registers_type _registers;

    /*
     * When array 0 is loaded from another array instead of copying native code 
     * and jump table both are transfered into array 0.  If both source array 
     * and array 0 are not modified before another array is duplicated into 
     * array 0 then native code and jump table are moved back into the 
     * originating array.  This should save a lot of copying.
     *
     * 0 value means that no back transfer should occur.  For example, when the 
     * source array is deallocated we break this connection.
     */
    size_t _array0Source;

    typedef std::vector<array *> _arrays_type;
    _arrays_type _arrays;

    /*
     * To speed up new array index generation this is the smallest array index 
     * that is not used at the moment.
     */
    size_t _minEmptyArrayIndex;

    /*
     * Calculates finger position based on a native code return address.  Finds 
     * index of a platter that set this return address.
     *
     * It is essentially an index of a platter that contains instructions at 
     * this address except for the case when `returnAddress' is a first byte of 
     * a patter native code.  In this case previous platter index is returned.
     *
     * The first byte of the second platter native code will return index for 
     * the first platter.  But the second byte of the second platter will return 
     * index for the second platter.  Any other addresses return index of a 
     * platter that generated native code at that address.
     */
    size_t fingerPositionFor(void * returnAddress);

    /*
     * Generates native instructions for the specified platter.
     *
     * Returns number of bytes written into `to'.
     * `to' should have enough space to hold all the instructions for this 
     * platter.
     * If `to' is a nullptr just returns the number of bytes required to 
     * represent this platter.
     */
    size_t codeFor(const platter & p, char * to);

    /*
     * Generates native instructions for an out of bound execution stub.
     *
     * Retuns number of bytes written into `to'.
     * `to' should have enough space to hold all the instructions.
     * If `to' is a nullptr just returns the number of bytes required for the 
     * stub. 
     */
    size_t codeForOOBStub(char * to);

    /*
     * Constructs a block of native code and a corresponding jump table for 
     * platters in the specified array.
     */
    void generateNativeCode(array & a);

    /* operator callbacks helpers */

    /* Allocates new array and returns its index */
    size_t allocation(size_t size);

    /*
     * Throws invalidArrayIndex if an array with index `index' was not 
     *        previously allocated or was already abandoned.
     */
    void abandonment(size_t index) throw(exceptions::invalidArrayIndex);

    /* Outputs v into _os. */
    void output(unsigned char v);

    /*
     * Reads the next character from _is and returns it or returns ~0 if the 
     * stream is in an error or EOF states.
     */
    unsigned int input();

    /*
     * Duplicates array index into array 0 and prepares it for execution of 
     * native code.
     *
     * Throws invalidArrayIndex if `index' is 0 or is an index of an array that 
     * is not allocated.
     */
    void loadProgram(size_t index) throw(exceptions::invalidArrayIndex);
};

#endif /* __CONTEXT__H */
