#include "context.h"

#include "../array.h"
#include "../platter.h"

#include <cpput/assertcommon.h>

#include <boost/assert.hpp>


namespace test {

    CPPUT_FIXTURE_TEST(context, testConstruction)
    {
        array * pa = array::create(mm, 10);

        ::context ctx(mm, is, os, pa);
    }

#define GENERAL_OP(IDX, OP, A, B, C)                    \
    BOOST_ASSERT(IDX == nextI);                         \
    a[nextI++] = (OP << 28) | (A << 6) | (B << 3) | C;  \
    /* */

#define OP_CONDITIONAL_MOVE(IDX, A, B, C) \
    GENERAL_OP(IDX, 0, A, B, C)

#define OP_ARRAY_INDEX(IDX, A, B, C) \
    GENERAL_OP(IDX, 1, A, B, C)

#define OP_ARRAY_AMENDMENT(IDX, A, B, C) \
    GENERAL_OP(IDX, 2, A, B, C)

#define OP_ADDITION(IDX, A, B, C) \
    GENERAL_OP(IDX, 3, A, B, C)

#define OP_MULTIPLICATION(IDX, A, B, C) \
    GENERAL_OP(IDX, 4, A, B, C)

#define OP_DIVISION(IDX, A, B, C) \
    GENERAL_OP(IDX, 5, A, B, C)

#define OP_NOT_AND(IDX, A, B, C) \
    GENERAL_OP(IDX, 6, A, B, C)

#define OP_HALT(IDX) \
    GENERAL_OP(IDX, 7, 0, 0, 0)

#define OP_ALLOCATION(IDX, B, C) \
    GENERAL_OP(IDX, 8, 0, B, C)

#define OP_ABANDONMENT(IDX, C) \
    GENERAL_OP(IDX, 9, 0, 0, C)

#define OP_OUTPUT(IDX, C) \
    GENERAL_OP(IDX, 10, 0, 0, C)

#define OP_INPUT(IDX, C) \
    GENERAL_OP(IDX, 11, 0, 0, C)

#define OP_LOAD_PROGRAM(IDX, B, C) \
    GENERAL_OP(IDX, 12, 0, B, C)

#define OP_ORTHOGRAPHY(IDX, A, VAL)             \
    BOOST_ASSERT(IDX == nextI);                 \
    a[nextI++] = (13 << 28) | (A << 25) | VAL;  \
    /* */

/*
 * a xor b = not (a nand b) nand ((a nand a) nand (b nand b))
 *
 * var1 = a nand a
 * var2 = b nand b
 * var1 = var1 nand var2
 * var2 = a nand b
 * var1 = var1 nand var2
 * res = var1 nand var1
 */
#define SYN_6OP_XOR(IDX, A, B, C, VAR1, VAR2)   \
    OP_NOT_AND(IDX    , VAR1, B, B);            \
    OP_NOT_AND(IDX + 1, VAR2, C,    C);         \
    OP_NOT_AND(IDX + 2, VAR1, VAR1, VAR2);      \
    OP_NOT_AND(IDX + 3, VAR2, B,    C);         \
    OP_NOT_AND(IDX + 4, VAR1, VAR1, VAR2);      \
    OP_NOT_AND(IDX + 5, A,    VAR1, VAR1);      \
    /* */

    CPPUT_FIXTURE_TEST(context, testInputOutput)
    {
        array * pa = array::create(mm, 5);
        array & a = *pa;

        size_t nextI = 0;

        OP_INPUT        (0,     1);
        OP_ORTHOGRAPHY  (1,     0, 10);
        OP_ADDITION     (2,     2, 1, 0);
        OP_OUTPUT       (3,     2);
        OP_HALT         (4);

        BOOST_ASSERT(nextI == a.size());


        is.str("A");

        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "K", "Output is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testAmendment)
    {
        array * pa = array::create(mm, 5);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     0, 4);
        OP_ORTHOGRAPHY      (1,     1, 'A');
        OP_ARRAY_AMENDMENT  (2,     2, 0, 1);

        OP_HALT             (3);

        /* Placeholder      (4) */
        nextI++;

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(a.dirty(), "Array is dirty");
        CPPUT_ASSERT(a[4] == 'A', "Array modified correctly");
    }

    CPPUT_FIXTURE_TEST(context, testIndex)
    {
        array * pa = array::create(mm, 5);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     1, 4);
        OP_ARRAY_INDEX      (1,     2, 0, 1);

        OP_OUTPUT           (2,     2);

        OP_HALT             (3);

        /* Value to output  (4) */
        a[nextI++] = 'B';

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "B", "Output is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testAmendmentAndIndex)
    {
        array * pa = array::create(mm, 7);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     0, 6);
        OP_ORTHOGRAPHY      (1,     1, 'P');
        OP_ARRAY_AMENDMENT  (2,     2, 0, 1);

        OP_ARRAY_INDEX      (3,     3, 2, 0);

        OP_OUTPUT           (4,     3);

        OP_HALT             (5);

        /* Placeholder      (6) */
        nextI++;

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "P", "Output is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testAllocateAndFill)
    {
        array * pa = array::create(mm, 36);
        array & a = *pa;

        size_t nextI = 0;

        /*
         * Registers:
         * 0 - allocated array index
         * 1 - allocated array size
         * 2 - fill value
         * 3 - fill value increment
         * 4 - loop counter/fill index
         * 5 - var1
         * 6 - var2
         * 7 - loop start address
         */

        /* New array length */
        OP_INPUT            (0,     1);
        OP_ALLOCATION       (1,     0, 1);

        /* Value to fill new array with */
        OP_INPUT            (2,     2);

        /* Fill value increment */
        OP_INPUT            (3,     3);

        /* === Fill array with values === */

        /* Loop start address */
        OP_ORTHOGRAPHY      (4,     7, 5);

        /* Counter is in register 4 */

        /* `0[`4] = `2 */
        OP_ARRAY_AMENDMENT  (5,     0, 4, 2);

        OP_ADDITION         (6,     2, 2, 3);
        OP_ORTHOGRAPHY      (7,     5, 1);
        OP_ADDITION         (8,     4, 4, 5);

        /* `5 = `1 xor `4 */
        SYN_6OP_XOR         (9,     5, 1, 4, /* */ 5, 6);

        /* Loop end address */
        OP_ORTHOGRAPHY      (15,    6, 19);

        /* `6 = `7 if `5 != 0 */
        OP_CONDITIONAL_MOVE (16,    6, 7, 5);
        OP_ORTHOGRAPHY      (17,    5, 0);
        OP_LOAD_PROGRAM     (18,    5, 6);

        /* === Output generated array === */

        /*
         * Registers:
         * 0 - allocated array index
         * 1 - allocated array size
         * 2 - loop counter
         * 3 - loop counter increment
         * 4 - loop start address
         * 5 - var1
         * 6 - var2
         * 7 - var3
         */

        OP_ORTHOGRAPHY      (19,    2, 0);
        OP_ORTHOGRAPHY      (20,    3, 1);
        OP_ORTHOGRAPHY      (21,    4, 22);

        OP_ARRAY_INDEX      (22,    5, 0, 2);
        OP_OUTPUT           (23,    5);
        OP_ADDITION         (24,    2, 2, 3);

        /* `5 = xor(`1, `2) */
        SYN_6OP_XOR         (25,    5, 1, 2, /* */ 5, 6);

        /* Loop end address */
        OP_ORTHOGRAPHY      (31,    6, 35);

        /* `6 = `4 if `5 != 0 */
        OP_CONDITIONAL_MOVE (32,    6, 4, 5);

        OP_ORTHOGRAPHY      (33,    5, 0);
        OP_LOAD_PROGRAM     (34,    5, 6);

        OP_HALT             (35);

        BOOST_ASSERT(nextI == a.size());


        is.str("\x1A" /* Array length is 20 */
               "a"    /* Initial constant to use */
               "\x01" /* Increase by 1 */);

        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "abcdefghijklmnopqrstuvwxyz",
                     "ABC generated");
    }

    CPPUT_FIXTURE_TEST(context, testAbandonment)
    {
        array * pa = array::create(mm, 28);
        array & a = *pa;

        size_t nextI = 0;

        /* Allocate first array.  Index in register 1. */
        OP_ORTHOGRAPHY      (0,     0, 10);
        OP_ALLOCATION       (1,     1, 0);

        /* Fill in a constant */
        OP_ORTHOGRAPHY      (2,     5, 7);
        OP_ORTHOGRAPHY      (3,     6, 'P');
        OP_ARRAY_AMENDMENT  (4,     1, 5, 6);

        /* Allocate second array.  Index in register 2. */
        OP_ORTHOGRAPHY      (5,     0, 5);
        OP_ALLOCATION       (6,     2, 0);

        /* Fill in a constant */
        OP_ORTHOGRAPHY      (7,     5, 4);
        OP_ORTHOGRAPHY      (8,     6, 'a');
        OP_ARRAY_AMENDMENT  (9,     2, 5, 6);

        /* Check first array content */
        OP_ORTHOGRAPHY      (10,    5, 7);
        OP_ARRAY_INDEX      (11,    6, 1, 5);
        OP_OUTPUT           (12,    6);

        OP_ABANDONMENT      (13,    1);

        /* Allocate third array.  Index in register 1. */
        OP_ORTHOGRAPHY      (14,    0, 8);
        OP_ALLOCATION       (15,    1, 0);

        /* Fill in a constant */
        OP_ORTHOGRAPHY      (16,    5, 3);
        OP_ORTHOGRAPHY      (17,    6, 's');
        OP_ARRAY_AMENDMENT  (18,    1, 5, 6);

        /* Check second array content */
        OP_ORTHOGRAPHY      (19,    5, 4);
        OP_ARRAY_INDEX      (20,    6, 2, 5);
        OP_OUTPUT           (21,    6);

        OP_ABANDONMENT      (22,    2);

        /* Check third array content */
        OP_ORTHOGRAPHY      (23,    5, 3);
        OP_ARRAY_INDEX      (24,    6, 1, 5);
        OP_OUTPUT           (25,    6);

        OP_ABANDONMENT      (26, 1);

        OP_HALT             (27);

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "Pas", "Output is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testLoadProgram)
    {
        array * pa = array::create(mm, 24);
        array & a = *pa;

        size_t nextI = 0;

        /*
         * Registers:
         * 0 - allocated array index
         * 1 - allocated array size
         * 2 - source array index = 0
         * 3 - copy from index
         * 4 - loop counter
         * 5 - loop start address
         * 6 - var1
         * 7 - var2
         */

        OP_ORTHOGRAPHY      (0,     1, 5);
        OP_ALLOCATION       (1,     0, 1);
        OP_ORTHOGRAPHY      (2,     3, 19);
        OP_ORTHOGRAPHY      (3,     5, 4);

        OP_ARRAY_INDEX      (4,     6, 2, 3);
        OP_ARRAY_AMENDMENT  (5,     0, 4, 6);

        OP_ORTHOGRAPHY      (6,     6, 1);
        OP_ADDITION         (7,     3, 3, 6);
        OP_ADDITION         (8,     4, 4, 6);

        /* `6 = `1 xor `4 */
        SYN_6OP_XOR         (9,     6, 1, 4, /* */ 6, 7);

        /* Loop end address */
        OP_ORTHOGRAPHY      (15,    7, 18);
        /* `7 = `5 if `6 != 0 */
        OP_CONDITIONAL_MOVE (16,    7, 5, 6);
        OP_LOAD_PROGRAM     (17,    2, 7);

        OP_LOAD_PROGRAM     (18,    0, 2);

        OP_ORTHOGRAPHY      (19,    0, 'O');
        OP_OUTPUT           (20,    0);
        OP_ORTHOGRAPHY      (21,    0, 'K');
        OP_OUTPUT           (22,    0);
        OP_HALT             (23);

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "OK", "Output is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testNativeCodeReuse)
    {
        array * pa = array::create(mm, 56);
        array & a = *pa;

        size_t nextI = 0;

        /*
         * Create first execution array
         *
         * Registers:
         * 0 - allocated array index
         * 1 - allocated array size
         * 2 - source array index = 0
         * 3 - copy from index
         * 4 - loop counter
         * 5 - loop start address
         * 6 - var1
         * 7 - var2
         */

        OP_ORTHOGRAPHY      (0,     1, 8);
        OP_ALLOCATION       (1,     0, 1);
        OP_ORTHOGRAPHY      (2,     3, 38);
        OP_ORTHOGRAPHY      (3,     5, 4);

        OP_ARRAY_INDEX      (4,     6, 2, 3);
        OP_ARRAY_AMENDMENT  (5,     0, 4, 6);

        OP_ORTHOGRAPHY      (6,     6, 1);
        OP_ADDITION         (7,     3, 3, 6);
        OP_ADDITION         (8,     4, 4, 6);

        /* `6 = `1 xor `4 */
        SYN_6OP_XOR         (9,     6, 1, 4, /* */ 6, 7);

        /* Loop end address */
        OP_ORTHOGRAPHY      (15,    7, 18);
        /* `7 = `5 if `6 != 0 */
        OP_CONDITIONAL_MOVE (16,    7, 5, 6);
        OP_LOAD_PROGRAM     (17,    2, 7);

        /*
         * Create second execution array
         *
         * Registers as above
         */

        OP_ORTHOGRAPHY      (18,    1, 10);
        OP_ALLOCATION       (19,    0, 1);
        OP_ORTHOGRAPHY      (20,    3, 46);
        OP_ORTHOGRAPHY      (21,    4, 0);
        OP_ORTHOGRAPHY      (22,    5, 23);

        OP_ARRAY_INDEX      (23,    6, 2, 3);
        OP_ARRAY_AMENDMENT  (24,    0, 4, 6);

        OP_ORTHOGRAPHY      (25,    6, 1);
        OP_ADDITION         (26,    3, 3, 6);
        OP_ADDITION         (27,    4, 4, 6);

        /* `6 = `1 xor `4 */
        SYN_6OP_XOR         (28,    6, 1, 4, /* */ 6, 7);

        /* Loop end address */
        OP_ORTHOGRAPHY      (34,    7, 37);
        /* `7 = `5 if `6 != 0 */
        OP_CONDITIONAL_MOVE (35,    7, 5, 6);
        OP_LOAD_PROGRAM     (36,    2, 7);

        OP_LOAD_PROGRAM     (37,    0, 2);

        /*
         * First execution array content
         */

        OP_ORTHOGRAPHY      (38,    0, '2');
        OP_OUTPUT           (39,    0);

        OP_ORTHOGRAPHY      (40,    1, 2);
        OP_ORTHOGRAPHY      (41,    2, 5);
        OP_LOAD_PROGRAM     (42,    1, 2);

        OP_ORTHOGRAPHY      (43,    0, '4');
        OP_OUTPUT           (44,    0);

        OP_HALT             (45);

        /*
         * Second execution array content
         */

        OP_ORTHOGRAPHY      (46,    0, '1');
        OP_OUTPUT           (47,    0);

        OP_ORTHOGRAPHY      (48,    1, 1);
        OP_ORTHOGRAPHY      (49,    2, 0);
        OP_LOAD_PROGRAM     (50,    1, 2);

        OP_ORTHOGRAPHY      (51,    0, '3');
        OP_OUTPUT           (52,    0);

        OP_ORTHOGRAPHY      (53,    1, 1);
        OP_ORTHOGRAPHY      (54,    2, 5);
        OP_LOAD_PROGRAM     (55,    1, 2);

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "1234", "Output is as expected");
    }

    /* In this test stub completely covers old instruction. */
    CPPUT_FIXTURE_TEST(context, testSelfModifyingCode1)
    {
        array * pa = array::create(mm, 10);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     0, 'O');
        OP_OUTPUT           (1,     0);

        OP_ORTHOGRAPHY      (2,     1, 9);
        OP_ARRAY_INDEX      (3,     2, 7, 1);

        OP_ORTHOGRAPHY      (4,     1, 6);
        OP_ARRAY_AMENDMENT  (5,     7, 1, 2);

        OP_ORTHOGRAPHY      (6,     0, '*');
        OP_OUTPUT           (7,     0);

        OP_HALT             (8);

        OP_ORTHOGRAPHY      (9,     0, 'K');

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "OK", "Output is as expected");
    }

    /* In this test new code has different size. */
    CPPUT_FIXTURE_TEST(context, testSelfModifyingCode2)
    {
        array * pa = array::create(mm, 23);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     0, 12);
        OP_ORTHOGRAPHY      (1,     1, 32);

        OP_ORTHOGRAPHY      (2,     3, 18);
        OP_ARRAY_INDEX      (3,     4, 2, 3);
        OP_ARRAY_AMENDMENT  (4,     2, 5, 4);

        OP_ORTHOGRAPHY      (5,     3, 19);
        OP_ARRAY_INDEX      (6,     4, 2, 3);
        OP_ORTHOGRAPHY      (7,     5, 1);
        OP_ARRAY_AMENDMENT  (8,     2, 5, 4);

        OP_ORTHOGRAPHY      (9,     3, 20);
        OP_ARRAY_INDEX      (10,    4, 2, 3);
        OP_ORTHOGRAPHY      (11,    5, 2);
        OP_ARRAY_AMENDMENT  (12,    2, 5, 4);

        OP_ORTHOGRAPHY      (13,    3, 21);
        OP_ARRAY_INDEX      (14,    4, 2, 3);
        OP_ORTHOGRAPHY      (15,    5, 3);
        OP_ARRAY_AMENDMENT  (16,    2, 5, 4);

        OP_LOAD_PROGRAM     (17,    2, 2);

        OP_MULTIPLICATION   (18,    3, 0, 1);
        OP_ORTHOGRAPHY      (19,    5, 22);
        OP_ARRAY_AMENDMENT  (20,    2, 5, 3);
        OP_HALT             (21);

        /* Placeholder      (22) */
        nextI++;

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(a[22] == 384, "Value is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testExecutionAfterArrayEnd)
    {
        array * pa = array::create(mm, 6);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     0, 'O');
        OP_OUTPUT           (1,     0);

        OP_ORTHOGRAPHY      (2,     0, 'O');
        OP_OUTPUT           (3,     0);

        OP_ORTHOGRAPHY      (4,     0, 'B');
        OP_OUTPUT           (5,     0);

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "OOB\n"
                     "Execution beyound array length\n",
                     "Output is as expected");
    }

    CPPUT_FIXTURE_TEST(context, testInvalidOperator)
    {
        array * pa = array::create(mm, 5);
        array & a = *pa;

        size_t nextI = 0;

        OP_ORTHOGRAPHY      (0,     0, 'I');
        OP_OUTPUT           (1,     0);

        OP_ORTHOGRAPHY      (2,     0, 'O');
        OP_OUTPUT           (3,     0);

        /* Invalid operator (4) */
        a[nextI++] = 0xE1234567;

        BOOST_ASSERT(nextI == a.size());


        ::context ctx(mm, is, os, pa);

        ctx.run();

        CPPUT_ASSERT(os.str() == "IO\n"
                     "Invalid operator: 0xE1234567\n",
                     "Output is as expected");
    }

#undef GENERAL_OP
#undef OP_CONDITIONAL_MOVE
#undef OP_ARRAY_INDEX
#undef OP_ARRAY_AMENDMENT
#undef OP_ADDITION
#undef OP_MULTIPLICATION
#undef OP_DIVISION
#undef OP_NOT_AND
#undef OP_HALT
#undef OP_ALLOCATION
#undef OP_ABANDONMENT
#undef OP_OUTPUT
#undef OP_INPUT
#undef OP_LOAD_PROGRAM
#undef OP_ORTHOGRAPHY
#undef SYN_6OP_XOR

}
