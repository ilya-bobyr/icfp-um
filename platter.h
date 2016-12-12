#ifndef __PLATTER__H
#define __PLATTER__H

#include "exceptions/invalidOperatorFormat.h"

#include <istream>
#include <limits>


class platter
{
public:

    static_assert(std::numeric_limits<unsigned int>::digits == 32,
                  "platters is written under assumptions that unsigned int is "
                  "a 32 bit integer");

    /* Default value is 0. */
    platter();
    platter(unsigned int v);

    /*
     * Reads 4 bytes from the stream and stores them as a platter.  Throws 
     * invalidOperatorFormat if there are less than 4 bytes available in the 
     * stream.
     */
    platter(std::istream & is) throw(exceptions::invalidOperatorFormat);

    platter(const platter & rhs);

    platter & operator=(unsigned int v);

    struct operator_
    {
        enum value
        {
            conditionalMove = 0,
            arrayIndex      = 1,
            arrayAmendment  = 2,
            addition        = 3,
            multiplication  = 4,
            division        = 5,
            notAnd          = 6,
            halt            = 7,
            allocation      = 8,
            abandonment     = 9,
            output          = 10,
            input           = 11,
            loadProgram     = 12,
            orthography     = 13
        };

    private:
        /* This struct is just a container for value. */
        operator_();
    };

    /*
     * Not all operators have values for all the A, B, C and value arguments.  
     * If a register or value argument is not relevant for an operator it may or 
     * may not be modified.  Its value should not be relied upon.
     */
    operator_::value decode
        (unsigned int & A, unsigned int & B, unsigned int & C,
         unsigned int & value) const
        throw(exceptions::invalidOperatorFormat);

    operator unsigned int() const;

private:
    unsigned int _v;
};

#endif /* __PLATTER__H */
