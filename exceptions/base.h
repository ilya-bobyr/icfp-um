#ifndef __EXCEPTIONS__BASE__H
#define __EXCEPTIONS__BASE__H

#include <stdexcept>
#include <string>
#include <locale>

namespace exceptions {

    /*
     * This is the base class for all the exception classes in the application.  
     * It contains a wide character description and can automatically convert 
     * this description to narrow chars if used as an std::exception.
     */
    class base: public std::exception
    {
    protected:
	std::wstring _msg;
	mutable char * _what;

    public:
	/*
	 * This constructor eases usage of this class as a virtual base.  In any 
	 * other case consider using other constructors that perform 
	 * full-fledged initialization as part of construction.
	 */
	base() throw():
	    _what(0)
	{ }

	base(const std::wstring & msg) throw():
	    _msg(msg), _what(0)
	{ }

	base(const base & rhs) throw():
	    _msg(rhs._msg), _what(0)
	{ }

	base & operator=(const base & rhs)
	{
	    if (this == &rhs)
		return *this;

	    clean_what();

	    _msg = rhs._msg;

	    return *this;
	}

	~base()
	{
	    clean_what();
	}

	/* This one is from the std::exception. */
	virtual const char * what() const throw()
	{
	    using namespace std;

	    if (_what != 0) 
		return _what;

	    _what = new char[_msg.size() + 1];
	    use_facet< ctype<wchar_t> >(locale()).
		narrow(_msg.data(), _msg.data() + _msg.size(),
		       '?', _what);
	    _what[_msg.size()] = '\0';

	    return _what;
	}

	void msg(const std::wstring & msg) throw()
	{
	    if (_msg == msg)
		return;

	    clean_what();

	    _msg = msg;
	}

	const std::wstring & msg() const throw()
	{
	    return _msg;
	}

    protected:
	void clean_what() const throw()
	{
	    if (_what != 0) {
		delete[] _what;
		_what = 0;
	    }
	}
    };

}

#endif /* __EXCEPTIONS__BASE__H */
