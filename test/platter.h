#ifndef __TEST__PLATTER__H
#define __TEST__PLATTER__H

#include <cpput/testing.h>

#include "../memoryManager.h"

namespace test
{

    struct platter: CppUT::TestCase
    {
        memoryManager mm;
    };

}

#endif /* __TEST__PLATTER__H */
