#ifndef __EXCEPTIONS__INVALID_OPERATOR_FORMAT__H
#define __EXCEPTIONS__INVALID_OPERATOR_FORMAT__H

#include "base.h"

#include <string>

namespace exceptions {

    /*
     * This exception is thrown when a platter is been transformed into an 
     * operator that does not have a valid format.
     */
    class invalidOperatorFormat: virtual public base
    {
    public:
        invalidOperatorFormat(const std::wstring & msg) throw()
            : base(msg)
        { }
    };
}

#endif /* __EXCEPTIONS__INVALID_OPERATOR_FORMAT__H */
