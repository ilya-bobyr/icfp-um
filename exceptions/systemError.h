#ifndef __EXCEPTIONS__SYSTEM_ERROR__H
#define __EXCEPTIONS__SYSTEM_ERROR__H

#include "../windows.h"

#include <string>
#include <locale>
#include <sstream>
#include <iomanip>

#include "base.h"

namespace exceptions {

    /*
     * This exception is thrown when an OS call returns an unexpected error.
     *
     * The class is designed to be usable both as a most derived by itself and 
     * as a virtual base for a more concrete class.
     */
    class systemError: virtual public base
    {
    public:
	/*
	 * If errorCode() is equal to errorCode_notSet then this class behaves 
	 * just as the base class does.
	 */
	static const DWORD errorCode_notSet = ~static_cast<DWORD>(0);

    protected:
        std::wstring _contextMsg;
	DWORD _errorCode;

    public:
	/*
	 * This constructor eases usage of this class as a virtual base.  In any 
	 * other case consider using other constructors that perform 
	 * full-fledged initialization as part of construction.
	 */
	systemError() throw();

	/* 
         * This enumeration is used to distinguish a constructor that 
	 * initializes the exception with the most recent OS error from the 
	 * default constructor that is used to help deal with virtual 
	 * inheritance.
	 */
	enum constructorTypes
	{
            getLast /* Instructs the constructor to obtain the error code via a 
                       GetLastError() call. */
	};

	systemError(constructorTypes value) throw();

	systemError(DWORD errorCode) throw();

	systemError(const std::wstring & msg) throw();

        /*
         * In addition to storing the error code store a message that describes 
         * the error context.  Both are combined in the exception message 
         * returned by msg() with context message preceding the error 
         * description.
         */
        systemError(const std::wstring & contextMsg, DWORD errorCode) throw();

        /*
         * This constructor is similar to the previous one except that is 
         * obtains the error code via GetLastError() call.
         */
        systemError(const std::wstring & contextMsg, constructorTypes value) 
            throw();

	DWORD errorCode() const throw();

	void errorCode(DWORD v) throw();

        const std::wstring contextMsg() const throw();

        void contextMsg(const std::wstring & contextMsg) throw();

	/*
	 * This method initializes the object exactly the same as the 
	 * systemError(getLast) constructor call would.
	 */
	void errorCode_getLast() throw();

	/*
	 * Narrow character string should be created not by narrowing the wide 
	 * character message but by querying the system for a non localized 
	 * variant of the error description and narrowing the result.
	 */
	virtual const char * what() const throw();

    protected:
        /*
         * Updates base::msg after modifications to either _contextMsg or 
         * _errorCode.
         */
        void updateMsg() throw();
    };
}

#endif /* __EXCEPTIONS__SYSTEM_ERROR__H */
