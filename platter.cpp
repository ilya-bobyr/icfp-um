#include "platter.h"


using namespace std;


platter::platter()
    : _v(0)
{ }

platter::platter(unsigned int v)
    : _v(v)
{ }

platter::platter(istream & is)
    throw(exceptions::invalidOperatorFormat)
    : _v(0)
{
    for (size_t i = 0; i < 4; ++i)
    {
        istream::int_type c = is.get();

        if (!is)
            throw exceptions::invalidOperatorFormat
                (L"Not enough bytes in the input stream to form a compleate "
                 L"platter");

        _v |= c << (8 * (3 - i));
    }
}

platter::platter(const platter & rhs)
    : _v(rhs._v)
{ }

platter & platter::operator=(unsigned int v)
{
    _v = v;
    return *this;
}

platter::operator_::value platter::decode
    (unsigned int & A, unsigned int & B, unsigned int & C,
     unsigned int & value) const
    throw(exceptions::invalidOperatorFormat)
{
    unsigned char operatorNumber = _v >> 28;

    if (operatorNumber < 13)
    {
        A = (_v >> 6) & 0x7;
        B = (_v >> 3) & 0x7;
        C =  _v       & 0x7;
    }
    else if (operatorNumber == 13)
    {
        A = (_v >> 25) & 0x7;
        value = _v & ((1 << 25) - 1);
    }
    else
        throw exceptions::invalidOperatorFormat
            (L"Operator number field contains an unexpected value");

    return static_cast<operator_::value>(operatorNumber);
}

platter::operator unsigned int() const
{
    return _v;
}
