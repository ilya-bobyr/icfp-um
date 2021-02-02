#include "context.h"

#include "jumpTable.h"
#include "nativeCode.h"

#include <algorithm>
#include <type_traits>
#include <istream>
#include <ostream>
#include <stdexcept>

#include <boost/assert.hpp>


using namespace std;


namespace
{

    /*
     * When native code returns it is expected to put one of these values into 
     * eax to indicate the reason of the return.
     */
    namespace nativeCodeReturnValue
    {
        enum value
        {
            /*
             * ebx: error code
             *   0 - normal termination
             *   1 - invalid operator
             *   2 - execution beyond array size
             *
             * ecx: additional value only valid for the following ebx values
             *   1 - ecx is a value of the plater that was not decoded
             */
            halt            = 1,

            /*
             * ebx: size of the new array
             */
            allocation      = 2,

            /*
             * ebx: an index of an array to abandon
             */
            abandonment     = 3,

            /*
             * ebx: value to output (should be [0, 255])
             */
            output          = 4,

            /*
             * Upon return into the native code:
             *      eax should contain a character code that was read [0, 255] 
             *          or ~0 for EOF
             */
            input           = 5,

            /*
             * ebx: an index of an array to be copied into array 0
             * ecx: an index to set the execution finger to
             */
            loadProgram     = 6,

            /*
             * This code is returned when execution hits a "recompile stub".  
             * Array 0 native code should be regenerated and execution should be 
             * restarted from the same platter.
             *
             * Recompile stubs are inserted by array amendment operations that 
             * amend array 0 itself.  This way we are trying to avoid extra 
             * recompilations if array 0 is modified in more than one spot 
             * before the modified code is actually executed.
             *
             * Execution finger position is calculated by doing a search in the 
             * jumpTable for the nativeCode before recompilation.  It is not 
             * very efficient but reduces stub size.  As stub may replace any 
             * other operator its size is effectively the minimum size for 
             * native code blocks generated for other operators.  As 
             * recompilation of a generated native code should not happen often 
             * this seems like a reasonable optimization.
             */
            recompile       = 7,
        };
    };

    namespace haltReturnCodes
    {
        enum value
        {
            normalTermination   = 0,
            invalidOperator     = 1,
            outOfBoundExecution = 2,
        };
    };
}


/*
 * === context ===
 */

context::context(memoryManager & mm, istream & is, ostream & os, ::array * zeroArray)
    : _mm(mm)
    , _is(is)
    , _os(os)
    , _array0Source(0)
    , _minEmptyArrayIndex(1)
{
    if (!zeroArray)
        throw invalid_argument("zeroArray should not be a null pointer");

    _arrays.push_back(zeroArray);
}

#pragma warning( push )
/*
 * C4731: frame pointer register 'ebp' modified by inline assembly code
 *
 * This is intentional.  ebp is modified to be used by the generated native code 
 * but then it should be restored to the original value.
 *
 * With vc10 this warning seems to work only at a function level.
 */
#pragma warning( disable: 4731 )

void context::run() throw(exceptions::invalidArrayIndex, 
                          exceptions::invalidOperatorFormat)
{
    ::array * array0 = _arrays[0];
    generateNativeCode(*array0);

    size_t eaxValue = 0;
    void * registers = &_registers[0];

    void * arrays = &_arrays[0];
    class jumpTable * jumpTable = array0->jumpTable();
    void * resumeAt = array0->nativeCode()->begin();

    size_t newFingerPosition;
    size_t returnCode; /* eax */

    while (true)
    {
        size_t value1; /* ebx */
        size_t value2; /* ecx */

        __asm
        {
            pushad

            mov eax, eaxValue
            mov ecx, resumeAt

            mov esi, registers
            mov edi, arrays
            mov ebx, jumpTable

            mov ebp, ebx

            call ecx

            pop edx
            mov ebp, [esp + 8]
            mov returnCode, eax
            mov value1, ebx
            mov value2, ecx
            mov resumeAt, edx

            popad
        }

        switch (returnCode)
        {
            case nativeCodeReturnValue::halt:
                switch (value1)
                {
                    case haltReturnCodes::normalTermination:
                        break;

                    case haltReturnCodes::invalidOperator:
                        _os << endl
                            << "Invalid operator: "
                                "0x" << hex << uppercase << value2 << endl;
                        break;

                    case haltReturnCodes::outOfBoundExecution:
                        _os << endl
                            << "Execution beyound array length" << endl;
                        break;

                    default:
                        _os << endl
                            << "Unexpected halt code: "
                                "0x" << hex << uppercase << value1 << endl;
                }
                return;

            case nativeCodeReturnValue::allocation:
                eaxValue = allocation(value1);
                arrays = &_arrays[0];
                break;

            case nativeCodeReturnValue::abandonment:
                abandonment(value1);
                arrays = &_arrays[0];
                break;

            case nativeCodeReturnValue::output:
                output(static_cast<unsigned char>(value1));
                break;

            case nativeCodeReturnValue::input:
                eaxValue = input();
                break;

            case nativeCodeReturnValue::loadProgram:
                newFingerPosition = value2;

                loadProgram(value1);

                array0 = _arrays[0];

                if (newFingerPosition >= array0->size())
                    throw exceptions::invalidArrayIndex
                        (L"loadProgram index out of range", newFingerPosition);

                jumpTable = array0->jumpTable();
                resumeAt = jumpTable->address(newFingerPosition);
                break;

            case nativeCodeReturnValue::recompile:
                newFingerPosition = fingerPositionFor(resumeAt);

                generateNativeCode(*array0);

                jumpTable = array0->jumpTable();
                resumeAt = jumpTable->address(newFingerPosition);
                break;

            default:
                _os << endl
                    << "Unexpected native code return: "
                                "0x" << hex << uppercase << returnCode << endl;
                return;
        }
    }
}
#pragma warning( pop )

size_t context::fingerPositionFor(void * returnAddress)
{
    ::array & array0 = *_arrays[0];

    BOOST_ASSERT(array0.size() > 0);

    void ** begin = array0.jumpTable()->begin();
    void ** end   = begin + array0.size() - 1;

    void ** platter = lower_bound(begin, end, returnAddress);

    /*
     * As return address should never be the very first byte of an array native 
     * code we should never get begin as a result.
     */
    BOOST_ASSERT(platter > begin);

    return platter - begin - 1;
}

/*
 * --- Code generation helper macros ---
 *
 * Used by codeFor and codeForOOBStub.
 *
 * Expect to, curr and size to be in scope.
 */

/*
 * STR is expected to be a litteral string constant and thus include a trailing 
 * zero bytes that is not copied in the output.
 */
#define EMIT_BYTES(STR)                         \
    if (to)                                     \
    {                                           \
        memcpy(curr, STR, sizeof(STR) - 1);     \
        curr += sizeof(STR) - 1;                \
    }                                           \
    size += sizeof(STR) - 1;                    \
    /* */

#define EMIT_REGISTER_AS_BYTE_DISP(REG)                         \
    if (to)                                                     \
    {                                                           \
        *curr = static_cast<char>(REG * sizeof(unsigned int));  \
        ++curr;                                                 \
    }                                                           \
    ++size;                                                     \
    /* */

#define EMIT_BYTE(VALUE)                                        \
    if (to)                                                     \
    {                                                           \
        *reinterpret_cast<unsigned char *>(curr) = VALUE;       \
        ++curr;                                                 \
    }                                                           \
    ++size;                                                     \
    /* */

#define EMIT_WORD(VALUE)                                        \
    if (to)                                                     \
    {                                                           \
        *reinterpret_cast<unsigned int *>(curr) = VALUE;        \
        curr += sizeof(unsigned int);                           \
    }                                                           \
    size += sizeof(unsigned int);                               \
    /* */

size_t context::codeFor(const platter & p, char * to)
{
    /*
     * Native code assumes:
     *
     * ESI - pointer to the registers array [8 32-bit values]
     * EDI - pointer to the collection of array pointers
     * EBP - jump table first entry address
     */

    unsigned int A, B, C, value;

    size_t size = 0;
    char * curr = to;

    size_t jmpSource = 0;

    /*
     * Any instruction should be compiled into at least this many bytes so that 
     * it can always be overwritten by a recompile stub in case the code in the 
     * array 0 will decide to modify itself.
     */
    const size_t recompileStubSize = 7;

    platter::operator_::value op;
    try
    {
        op = p.decode(A, B, C, value);
    }
    catch (const exceptions::invalidOperatorFormat & /* ex */)
    {
        static_assert(nativeCodeReturnValue::halt == 1,
                      "halt value is encoded below.  If it changes "
                      "the value below should be updated.");
        static_assert(haltReturnCodes::invalidOperator == 1,
                      "invalidOperator value is encoded below.  If it changes "
                      "the value below should be update.");

        /* eax: nativeCodeReturnValue::halt */
        EMIT_BYTES("\x31\xC0"               /* xor eax, eax             */
                   "\xB0\x01"               /* mov al, imm8             */
                                   /* imm8: nativeCodeReturnValue::halt */
        /* ecx: 1 - invalid operator */
                   "\x31\xDB"               /* xor ebx, ebx             */
                   "\xB3\x01"               /* mov bl, imm8             */
                              /* imm8: haltReturnCodes::invalidOperator */
        /* edx: Invalid platter value */
                   "\xB9");                 /* mov ecx, imm32           */
        EMIT_WORD(p);
        /* return */
        EMIT_BYTES("\x5A"                   /* pop edx                  */
                   "\xFF\xD2");             /* call edx                 */

        BOOST_ASSERT(size >= recompileStubSize);

        return size;
    }

    switch (op)
    {
        case platter::operator_::conditionalMove:
            /* ecx: C */
            EMIT_BYTES("\x8B\x4E");         /* mov ecx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */

            /* if (C != 0) { */
            EMIT_BYTES("\xE3\x06");         /* jcxz rel8 (6)            */
            jmpSource = size;

            /*     eax: B */
            EMIT_BYTES("\x8B\x46");         /* mov eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /*     A = B */
            EMIT_BYTES("\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */
            /* } */
            BOOST_ASSERT(jmpSource + 6 == size);

            BOOST_ASSERT(size >= recompileStubSize);

            break;
            
        case platter::operator_::arrayIndex:
            /* ebx: B */
            EMIT_BYTES("\x8B\x5E");         /* mov ebx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /* eax: array[B] */
            EMIT_BYTES("\x8B\x04\x9F");     /* mov eax, [edi + ebx * 4] */
            /* ecx: C */
            EMIT_BYTES("\x8B\x4E");         /* mov ecx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */
            /* eax: array[B]->platters()[C] */
            EMIT_BYTES("\x8B\x44\x88");    
                          /* mov eax, [eax + ecx * 4 + disp8]           */
                          /*          [eax + ecx * 4 + <first platter>] */
            EMIT_BYTE(::array::_plattersOffset);     /* <first platter> */
            BOOST_ASSERT(::array::_plattersOffset < 256);

            /* A = array[B]->platters()[C] */
            EMIT_BYTES("\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::arrayAmendment:
            /* ecx: A */
            EMIT_BYTES("\x8B\x4E");         /* mov ecx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*          [esi + A]       */
            /* ebx: B */
            EMIT_BYTES("\x8B\x5E");         /* mov ebx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /* edx: C */
            EMIT_BYTES("\x8B\x56");         /* mov edx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */

            /* eax: array[A] */
            EMIT_BYTES("\x8B\x04\x8F");     /* mov eax, [edi + ecx * 4] */

            /* array[A]->_flags |= dirty */
            EMIT_BYTES("\x83\x88");         /* or [eax + disp32], imm8  */
            EMIT_WORD(static_cast<unsigned int>(offsetof(::array, _flags)));
                                            /*    [eax + array::_flags] */
            EMIT_BYTE(static_cast<unsigned char>(::array::flag::dirty));
                                            /* imm8: array::flag::dirty */

            /* array[A]->platters()[B] = C */
            EMIT_BYTES("\x89\x54\x98");    
                               /* mov [eax + ebx * 4 + disp8], edx      */
                               /*     [eax + ebx * 4 + <first platter>] */
            EMIT_BYTE(::array::_plattersOffset);     /* <first platter> */
            BOOST_ASSERT(::array::_plattersOffset < 256);

            /* if (A == 0) { */
            EMIT_BYTES("\x83\xF9\x00");     /* cmp ecx, imm8 (0)        */
            EMIT_BYTES("\x75\x13");         /* jnz rel8: 19             */
            jmpSource = size;

            /*     eax: jumpTable[B] */
            EMIT_BYTES("\x8B\x44\x9D\x00"); 
                                 /* mov eax, [ebp + ebx * 4 + disp8(0)] */

            /*
             *     *eax = asm {
             *                  xor eax, eax
             *                  mov al, imm8
             *                   ... nativeCodeReturnValue::recompile
             *                  pop edx
             *                  call edx
             *            }
             */

            static_assert(nativeCodeReturnValue::recompile == 7,
                          "recompile is encoded in the code below.  If it "
                          "value changes code below should be updated.");
            /*
             * 31 C0             xor eax, eax
             * B0 07             mov al, imm8 - B0+ al(0)
             *                            nativeCodeReturnValue::recompile
             * 5A                pop edx
             * FF D2             (near abs) call edx
             */

            EMIT_BYTES("\xC7\x00\x31\xC0\xB0\x00"
                                       /* mov [eax], imm32 (0x31C0B000) */

                       "\xC7\x40\x03\x07\x5A\xFF\xD2"
                            /* mov [eax + disp8(3)], imm32 (0x075AFFD2) */

            /*     jmp rel8 (0) */
                       "\xEB\x00");

            /* } */
            BOOST_ASSERT(jmpSource + 19 == size);

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::addition:
            /* eax: B */
            EMIT_BYTES("\x8B\x46");         /* mov eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /* eax: B + C */
            EMIT_BYTES("\x03\x46");         /* add eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */
            /* A = B + C */
            EMIT_BYTES("\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::multiplication:
            /* eax: B */
            EMIT_BYTES("\x8B\x46");         /* mov eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /* eax: B * C */
            EMIT_BYTES("\xF7\x66");       /* mul edx:eax, [esi + disp8] */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*            [esi + C]     */
            /* A = B * C */
            EMIT_BYTES("\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::division:
            /* eax: B */
            EMIT_BYTES("\x8B\x46");         /* mov eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /* edx: 0 */
            EMIT_BYTES("\x31\xD2");         /* xor edx, edx             */
            /* eax: B / C */
            EMIT_BYTES("\xF7\x76");       /* div edx:eax, [esi + disp8] */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*            [esi + C]     */
            /* A = B / C */
            EMIT_BYTES("\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::notAnd:
            /* eax: B */
            EMIT_BYTES("\x8B\x46");         /* mov eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */
            /* eax: B & C */
            EMIT_BYTES("\x23\x46");         /* and eax, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */
            /* eax: ~(B & C) */
            EMIT_BYTES("\xF7\xD0");         /* not eax                  */
            /* A = ~(B & C) */
            EMIT_BYTES("\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::halt:

            static_assert(nativeCodeReturnValue::halt == 1,
                          "halt value is encoded below.  If it changes "
                          "the value below should be updated.");
            static_assert(haltReturnCodes::normalTermination == 0,
                          "normalTermination value is encoded below.  If it "
                          "changes the value below should be update.");

            /* eax: nativeCodeReturnValue::halt */
            EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
                       "\xB0\x01"           /* mov al, imm8             */
                                   /* imm8: nativeCodeReturnValue::halt */
            /* ebx: 0 - normal termination */
                       "\x31\xDB"           /* xor ebx, ebx             */
            /* return */
                       "\x5A"               /* pop edx                  */
                       "\xFF\xD2");         /* call edx                 */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::allocation:

            static_assert(nativeCodeReturnValue::allocation == 2,
                          "allocation value is encoded below.  If it "
                          "changes the value below should be updated.");

            /* eax: nativeCodeReturnValue::allocation */
            EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
                       "\xB0\x02"           /* mov al, imm8             */
                             /* imm8: nativeCodeReturnValue::allocation */
            /* ebx: C */
                       "\x8B\x5E");         /* mov ebx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */

            /* return */
            EMIT_BYTES("\x5A"               /* pop edx                  */
                       "\xFF\xD2"           /* call edx                 */

            /* B: eax (new array index) */
                       "\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*     [esi + B]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::abandonment:

            static_assert(nativeCodeReturnValue::abandonment == 3,
                          "abandonment value is encoded below.  If it "
                          "changes the value below should be updated.");

            /* eax: nativeCodeReturnValue::abandonment */
            EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
                       "\xB0\x03"           /* mov el, imm8             */
                            /* imm8: nativeCodeReturnValue::abandonment */
            /* ebx: C */
                       "\x8B\x5E");         /* mov ebx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */

            /* return */
            EMIT_BYTES("\x5A"               /* pop edx                  */
                       "\xFF\xD2");         /* call edx                 */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::output:

            static_assert(nativeCodeReturnValue::output == 4,
                          "output value is encoded below.  If it "
                          "changes the value below should be updated.");

            /* eax: nativeCodeReturnValue::output */
            EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
                       "\xB0\x04"           /* mov el, imm8             */
                                 /* imm8: nativeCodeReturnValue::output */
            /* ebx: C */
                       "\x8B\x5E");         /* mov ebx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */

            /* return */
            EMIT_BYTES("\x5A"               /* pop edx                  */
                       "\xFF\xD2");         /* call edx                 */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::input:

            static_assert(nativeCodeReturnValue::input == 5,
                          "input value is encoded below.  If it "
                          "changes the value below should be updated.");

            /* eax: nativeCodeReturnValue::input */
            EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
                       "\xB0\x05"           /* mov al, imm8             */
                                  /* imm8: nativeCodeReturnValue::input */

                       "\x5A"               /* pop edx                  */
                       "\xFF\xD2"           /* call edx                 */

            /* C = <input char> */
                       "\x89\x46");         /* mov [esi + disp8], eax   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*     [esi + C]            */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::loadProgram:

            static_assert(nativeCodeReturnValue::loadProgram == 6,
                          "loadProgram value is encoded below.  If it "
                          "changes the value below should be updated.");

            /* ebx: B */
            EMIT_BYTES("\x8B\x5E");         /* mov ebx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(B);  /*          [esi + B]       */

            /* ecx: C */
            EMIT_BYTES("\x8B\x4E");         /* mov ecx, [esi + disp8]   */
            EMIT_REGISTER_AS_BYTE_DISP(C);  /*          [esi + C]       */

            /* if (B == 0) { */
            EMIT_BYTES("\x83\xFB\x00");     /* cmp ebx, imm8 (0)        */
            EMIT_BYTES("\x75\x06");         /* jnz rel8: 6              */
            jmpSource = size;

            /*     eax: jumpTable[C] */
            EMIT_BYTES("\x8B\x44\x8D\x00"
                                 /* mov eax, [ebp + ecx * 4 + disp8(0)] */
            /*     jmp eax */
                       "\xFF\xE0"); /* jmp eax */
            /* } */

            BOOST_ASSERT(jmpSource + 6 == size);

            /* eax: nativeCodeReturnValue::loadProgram */
            EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
                       "\xB0\x06"           /* mov el, imm8             */
                            /* imm8: nativeCodeReturnValue::loadProgram */

            /* return */
                       "\x5A"               /* pop edx                  */
                       "\xFF\xD2");         /* call edx                 */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        case platter::operator_::orthography:
            /* A = value */
            EMIT_BYTES("\xC7\x46");         /* mov [esi + disp8], imm32 */
            EMIT_REGISTER_AS_BYTE_DISP(A);  /*     [esi + A]            */
            EMIT_WORD(value);               /*                    value */

            BOOST_ASSERT(size >= recompileStubSize);

            break;

        default:
            throw logic_error("Unexpected operator");
    }

    return size;
}

size_t context::codeForOOBStub(char * to)
{
    size_t size = 0;
    char * curr = to;

    static_assert(nativeCodeReturnValue::halt == 1,
                  "halt value is encoded below.  If it changes "
                  "the value below should be updated.");
    static_assert(haltReturnCodes::outOfBoundExecution == 2,
                  "outOfBoundExecution value is encoded below.  If it "
                  "changes the value below should be update.");

    /* eax: nativeCodeReturnValue::halt */
    EMIT_BYTES("\x31\xC0"           /* xor eax, eax             */
               "\xB0\x01"           /* mov al, imm8             */
                           /* imm8: nativeCodeReturnValue::halt */
    /* ebx: 2 - out of bound execution */
               "\x31\xDB"           /* xor ebx, ebx             */
               "\xB3\x02"           /* mov bl, imm8             */
                  /* imm8: haltReturnCodes::outOfBoundExecution */
    /* return */
               "\x5A"               /* pop edx                  */
               "\xFF\xD2");         /* call edx                 */

    return size;
}

#undef EMIT_BYTES
#undef EMIT_REGISTER_AS_BYTE_DISP
#undef EMIT_BYTE
#undef EMIT_WORD

void context::generateNativeCode(::array & a)
{
    if (a._nativeCode)
    {
        a._nativeCode->destroy(_mm);
        a._nativeCode = nullptr;
    }

    a.dirty(false);

    /* Precalculate native code size */
    size_t nativeCodeSize = 0;
    for (size_t i = 0, len = a.size(); i < len; ++i)
        nativeCodeSize += codeFor(a[i], nullptr);

    /* Stub to prevent execution beyond array length */
    nativeCodeSize += codeForOOBStub(nullptr);

    a._nativeCode = nativeCode::create(_mm, nativeCodeSize, a.size());

    char * nativeCode = a._nativeCode->begin();
    void ** jumpTable = a._nativeCode->jumpTable()->begin();
    for (size_t i = 0, len = a.size(); i < len; ++i)
    {
        *jumpTable++ = nativeCode;
        nativeCode += codeFor(a[i], nativeCode);
    }

    nativeCode += codeForOOBStub(nativeCode);
}

size_t context::allocation(size_t size)
{
    /* Find next available index. */
    while (_minEmptyArrayIndex < _arrays.size() && _arrays[_minEmptyArrayIndex])
        ++_minEmptyArrayIndex;

    if (_minEmptyArrayIndex == _arrays.size())
    {
        _arrays.push_back(nullptr);
        _minEmptyArrayIndex = _arrays.size() - 1;
    }

    _arrays[_minEmptyArrayIndex] = ::array::create(_mm, size);

    return _minEmptyArrayIndex;
}

void context::abandonment(size_t index)
{
    if (index >= _arrays.size())
        throw exceptions::invalidArrayIndex
            (L"Attempt to an abandon an unallocated array", index);

    if (_array0Source == index)
        _array0Source = 0;

    _arrays[index]->destroy(_mm);
    _arrays[index] = 0;

    if (index < _minEmptyArrayIndex)
        _minEmptyArrayIndex = index;
}

void context::output(unsigned char v)
{
    _os << static_cast<char>(v);

    if (v == '\n')
        _os << flush;
}

unsigned int context::input()
{
    istream::int_type v = _is.get();

    return v == istream::traits_type::eof() ? ~static_cast<unsigned int>(0) : v;
}

void context::loadProgram(size_t index)
{
    if (index == 0)
        throw exceptions::invalidArrayIndex(L"Can not load array 0", 0);

    if (index >= _arrays.size()
        || _arrays[index] == nullptr)
        throw exceptions::invalidArrayIndex
            (L"Attempt to load an array that is not allocated", index);

    ::array * array0 = _arrays[0];

    if (!array0->dirty() && _array0Source != 0)
    {
        ::array * oldSource = _arrays[_array0Source];
        if (!oldSource->dirty())
        {
            oldSource->_nativeCode = array0->_nativeCode;
            array0->_nativeCode = nullptr;
        }
    }

    array0->destroy(_mm);

    ::array * source = _arrays[index];

    if (source->dirty() || !source->_nativeCode)
        generateNativeCode(*source);

    array0 = _arrays[0] = source->clone(_mm);

    array0->_nativeCode = source->_nativeCode;
    source->_nativeCode = nullptr;

    _array0Source = index;
}
