#ifndef __TEST__CONTEXT__H
#define __TEST__CONTEXT__H

#include <cpput/testing.h>

#include "../memoryManager.h"
#include "../context.h"

#include <sstream>


namespace test {

    struct context: CppUT::TestCase
    {
        std::istringstream is;
        std::ostringstream os;

        memoryManager mm;
    };

}

#endif /* __TEST__CONTEXT__H */
