#include "memoryManager.h"

#include <cpput/assertcommon.h>


namespace test {

    void memoryManager::allocTestHelper(size_t s)
    {
        void * p = mm.alloc(s);

        CPPUT_ASSERT(p != nullptr, "Allocation complete");

        for (size_t i = 0; i < s; ++i)
            CPPUT_ASSERT_EQUAL(0, reinterpret_cast<char *>(p)[i]);

        /*
         * Put in some values.
         */
        for (size_t i = 0; i < s; ++i)
            reinterpret_cast<unsigned char *>(p)[i] =
                static_cast<unsigned char>(i + 0x37);

        mm.release(p);
    }


    CPPUT_FIXTURE_TEST(memoryManager, testConstruction)
    {
        CPPUT_ASSERT(true, "Memory manager was constructed");
    }

    CPPUT_FIXTURE_TEST(memoryManager, testAllocation)
    {
        /*
         * Allocate chunks of different sizes.
         * Attempt to cover chunk size changes.
         */

        for (size_t s = 1; s < 70; ++s)
            allocTestHelper(s);

        for (size_t s = 1020; s < 1060; ++s)
            allocTestHelper(s);
    }

    CPPUT_FIXTURE_TEST(memoryManager, testLargeAllocation)
    {
        /* Allocate big chunk of memory. */

        allocTestHelper(2 * 1024 * 1024);
    }

}
