#include "array.h"

#include "../platter.h"

#include <cpput/assertcommon.h>


namespace test {

    CPPUT_FIXTURE_TEST(array, testConstruction)
    {
        ::array * a = ::array::create(mm, 10);

        CPPUT_ASSERT(a != nullptr, "Allocation complete");
        CPPUT_ASSERT(a->platters() != nullptr,   "platters() is not null");
        CPPUT_ASSERT(a->jumpTable() == nullptr,  "jumpTable() is null");
        CPPUT_ASSERT(a->nativeCode() == nullptr, "nativeCode() is null");

        a->destroy(mm);
    }

    CPPUT_FIXTURE_TEST(array, testContent)
    {
        for (size_t s = 20; s < 70; s += 3)
        {
            ::array * a = ::array::create(mm, s);

            fillPattern(*a);
            checkPattern(*a);

            a->destroy(mm);
        }
    }


    CPPUT_FIXTURE_TEST(array, testClone)
    {
        for (size_t s = 20; s < 30 * 1024; s = s * 2)
        {
            ::array * a = ::array::create(mm, s);

            fillPattern(*a);

            ::array * c = a->clone(mm);

            fillWith(*a, 0x1271B318);

            a->destroy(mm);

            checkPattern(*c);

            c->destroy(mm);
        }
    }

    CPPUT_FIXTURE_TEST(array, testDirtyFlag)
    {
        ::array * a = ::array::create(mm, 10);

        CPPUT_ASSERT_EQUAL(false, a->dirty());

        a->dirty(true);

        CPPUT_ASSERT_EQUAL(true, a->dirty());

        a->destroy(mm);
    }

    /*
     * === Helpers ===
     */

    void array::fillWith(::array & a, unsigned int v)
    {
        size_t s = a.size();

        for (size_t i = 0; i < s; ++i)
            a[i] = v;
    }

    void array::checkAllEqual(const ::array & a, unsigned int v)
    {
        size_t s = a.size();

        for (size_t i = 0; i < s; ++i)
            CPPUT_ASSERT_EQUAL(v, a[i]);
    }

    void array::fillPattern(::array & a)
    {
        size_t s = a.size();

        for (size_t i = 0; i < s; ++i)
        {
            CPPUT_ASSERT_EQUAL(0, a[i]);

            size_t v = i & 0xFF;
            v = 0x55AACC33 | v | (v << 8) | (v << 16) | (v << 24);
            a[i] = v;
        }
    }

    void array::checkPattern(const ::array & a)
    {
        size_t s = a.size();

        for (size_t i = 0; i < s; ++i)
        {
            size_t v = i & 0xFF;
            v = 0x55AACC33 | v | (v << 8) | (v << 16) | (v << 24);

            CPPUT_ASSERT_EQUAL(v, a[i]);
        }
    }

}
