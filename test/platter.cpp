#include "platter.h"

#include "../platter.h"

#include <cpput/assertcommon.h>


namespace test {

    CPPUT_FIXTURE_TEST(platter, testConstruction)
    {
        ::platter p(0x87341256);

        CPPUT_ASSERT(p == 0x87341256,
                     "Platter holds value it was initialized with");
    }

    CPPUT_FIXTURE_TEST(platter, testAssignment)
    {
        ::platter p(0x89712250);

        CPPUT_ASSERT(p == 0x89712250,
                     "Platter holds value it was initialized with");

        p = 0x178212;

        CPPUT_ASSERT(p == 0x178212, "Platter value correctly updated");
    }

    CPPUT_FIXTURE_TEST(platter, testDecode)
    {
        /*
         * Array Amendment
         *
         * Operator: 2
         * A: 7
         * B: 3
         * C: 1
         */
        ::platter p(0x200001D9);

        unsigned int A     = 0;
        unsigned int B     = 0;
        unsigned int C     = 0;
        unsigned int value = 0;

        ::platter::operator_::value op = p.decode(A, B, C, value);
        CPPUT_ASSERT(op == ::platter::operator_::arrayAmendment,
                     "Plater operator is arrayAmendment");
        CPPUT_ASSERT(A == 7, "`A' register index is 7");
        CPPUT_ASSERT(B == 3, "`B' register index is 3");
        CPPUT_ASSERT(C == 1, "`C' register index is 1");

        /*
         * Orthography
         * A: 4
         * value: 14C 759C
         */
        p = 0xD94C759C;
        op = p.decode(A, B, C, value);
        CPPUT_ASSERT(op == ::platter::operator_::orthography,
                     "Plater operator is orthography");
        CPPUT_ASSERT(A == 4, "`A' register index is 4");
        CPPUT_ASSERT(value == 0x14C759C, "`value' is 0x14C759C");
    }

}
