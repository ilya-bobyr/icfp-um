#ifndef __TEST__SCROLL_READER__H
#define __TEST__SCROLL_READER__H

#include <cpput/testing.h>

#include "../memoryManager.h"

namespace test
{

    struct scrollReader: CppUT::TestCase
    {
        memoryManager mm;
    };

}

#endif /* __TEST__SCROLL_READER__H */
