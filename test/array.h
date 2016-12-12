#ifndef __TEST__ARRAY__H
#define __TEST__ARRAY__H

#include <cpput/testing.h>

#include "../memoryManager.h"
#include "../array.h"

namespace test
{

    struct array: CppUT::TestCase
    {
        memoryManager mm;

        void fillWith(::array & a, unsigned int v);

        void checkAllEqual(const ::array & a, unsigned int v);

        /*
         * Fill array with a predefined pattern that can be checked via 
         * checkPattern(...) call.
         */
        void fillPattern(::array & a);

        /*
         * Checks that array has values generated by fillPattern(...). 
         */
        void checkPattern(const ::array & a);
    };

}

#endif /* __TEST__ARRAY__H */