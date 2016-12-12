#include "scrollReader.h"

#include <istream>
#include <vector>

#include <stdlib.h>

#include "memoryManager.h"
#include "array.h"
#include "platter.h"


using namespace std;


array * scrollReader::readLegacy(memoryManager & mm,
                                 istream & scroll, size_t size)
    throw(invalid_argument, runtime_error)
{
    static_assert(sizeof(unsigned int) == sizeof(platter),
                  "unsigned int is used to hold platter values");
    static_assert(sizeof(unsigned int) == 4,
                  "unsigned int is read as 4 bytes");

    if (size % 4 != 0)
        throw invalid_argument("size is not a multiple of 4");

    array * res = array::create(mm, size / 4);

    scroll.read(reinterpret_cast<char*>(res->platters()), size);
    if (!scroll)
    {
        res->destroy(mm);

        throw runtime_error("scroll does not have enough characters");
    }

    platter * platters = res->platters();
    for (size_t i = 0; i < size / 4; ++i)
        platters[i] = _byteswap_ulong(platters[i]);

    return res;
}
