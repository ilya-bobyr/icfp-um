#include "scrollReader.h"

#include "../scrollReader.h"
#include "../array.h"
#include "../platter.h"

#include <cpput/assertcommon.h>

#include <string>
#include <sstream>

using namespace std;


namespace test {

    CPPUT_FIXTURE_TEST(scrollReader, testReadLegacy)
    {
        const char scrollStr[] =
            "\x12\x34\x56\x78"
            "\x78\x56\x00\x12"
            "\x00\xaa\xcc\x33";

        /*
         * `- 1' is for the automatically added zero byte at the end of the 
         * string literal.
         */
        string scroll(scrollStr, scrollStr + sizeof(scrollStr) - 1);

        istringstream iss(scroll);

        array * pa = ::scrollReader::readLegacy(mm, iss, scroll.size());
        array & a = *pa;

        CPPUT_ASSERT(a.size() == 3, "Array size is 3");
        CPPUT_ASSERT(a[0] == 0x12345678u, "First platter read correctly");
        CPPUT_ASSERT(a[1] == 0x78560012u, "Second platter read correctly");
        CPPUT_ASSERT(a[2] == 0x00aacc33u, "Third platter read correctly");
    }

}
