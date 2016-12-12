#ifndef __EXCEPTIONS__INVALID_ARRAY_INDEX__H
#define __EXCEPTIONS__INVALID_ARRAY_INDEX__H

#include "base.h"

#include <string>

namespace exceptions
{
    /*
     * This exception is thrown when an operator requests an opeartion on an 
     * array that is not allocated.
     */
    class invalidArrayIndex: virtual public base
    {
        size_t _index;

    public:
        invalidArrayIndex(const std::wstring & msg, size_t index) throw()
            : base(msg), _index(index)
        { }

        const size_t index() const
        {
            return _index;
        }
    };
}

#endif /* __EXCEPTIONS__INVALID_ARRAY_INDEX__H */
