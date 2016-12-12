#ifndef __TEST__MEMORY_MANAGER__H
#define __TEST__MEMORY_MANAGER__H

#include <cpput/testing.h>

#include "../memoryManager.h"

namespace test
{
    struct memoryManager: CppUT::TestCase
    {
        ::memoryManager mm;

        /*
         * A helper that allocates, fills in and then deallocates a chunk of 
         * memory of the specifiend size.
         */
        void allocTestHelper(size_t s);
    };
}

#endif /* __TEST__MEMORY_MANAGER__H */
