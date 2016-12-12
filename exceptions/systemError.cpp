#include "systemError.h"

#include "../utils.h"

using namespace std;

namespace exceptions {

    systemError::systemError() throw():
        _errorCode(errorCode_notSet)
    { }

    systemError::systemError(constructorTypes value) throw():
        _errorCode(GetLastError())
    {
        /*
         * As the base is a virtual base type it's constructor call may be 
         * moved to a more derived constructor and thus can not be used 
         * reliably.  Also it is guaranteed to be already called as the 
         * execution will reach this point as systemError derives from base.
         */
        msg(systemErrorText(_errorCode));
    }

    systemError::systemError(DWORD errorCode) throw():
        _errorCode(errorCode)
    {
        /* See comment in the constructor above. */
        msg(systemErrorText(_errorCode));
    }

    systemError::systemError(const wstring & msg) throw():
        _errorCode(errorCode_notSet)
    {
        /* 
         * It is arguable if it would be better to call the base constructor 
         * in the member initializers list so it would be removed if the 
         * class is inherited.  Also see comment in the constructor above.
         */
        this->msg(msg);
    }

    systemError::systemError(const wstring & contextMsg, DWORD errorCode) 
        throw():
        _contextMsg(contextMsg), _errorCode(errorCode)
    {
        updateMsg();
    }

    systemError::systemError(const wstring & contextMsg, constructorTypes value) 
        throw():
        _contextMsg(contextMsg), _errorCode(GetLastError())
    {
        updateMsg();
    }

    DWORD systemError::errorCode() const throw()
    {
        return _errorCode;
    }

    void systemError::errorCode(DWORD v) throw()
    {
        if (_errorCode == v)
            return;

        _errorCode = v;

        updateMsg();
    }

    const wstring systemError::contextMsg() const throw()
    {
        return _contextMsg;
    }

    void systemError::contextMsg(const wstring & contextMsg) throw()
    {
        if (_contextMsg == contextMsg)
            return;

        _contextMsg = contextMsg;

        updateMsg();
    }

    void systemError::errorCode_getLast() throw()
    {
        errorCode(GetLastError());
    }

    const char * systemError::what() const throw()
    {
        if (_errorCode == errorCode_notSet && _contextMsg.empty())
            return base::what();

        if (_what != 0)
            return _what;

        /* New _what size. */
        size_t what_size = 0;

        /*
         * System error message for _errorCode in English unless _errorCode 
         * is errorCode_notSet. 
         */
        wstring usMsg;

        if (_errorCode != errorCode_notSet)
        {
            usMsg = systemErrorText
                (_errorCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));

            what_size = usMsg.size() + 1;
        }

        if (!_contextMsg.empty())
            what_size += _contextMsg.size() + 1;

        _what = new char[what_size];

        /* Position to write the next part of the message. */
        char * p = _what;

        if (!_contextMsg.empty())
        {
            use_facet< ctype<wchar_t> >(locale()).
                narrow(_contextMsg.data(), 
                       _contextMsg.data() + _contextMsg.size(),
                       '?', p);

            if (_errorCode != errorCode_notSet)
            {
                p += _contextMsg.size();
                *p = '\n';
                ++p;
            }
        }

        if (_errorCode != errorCode_notSet)
        {
            use_facet< ctype<wchar_t> >(locale()).
                narrow(usMsg.data(), usMsg.data() + usMsg.size(),
                       '?', p);
        }

        _what[what_size] = '\0';

        return _what;
    }

    void systemError::updateMsg() throw()
    {
        if (_contextMsg.empty() && _errorCode == errorCode_notSet)
        {
            msg(L"");
            return;
        }

        if (_contextMsg.empty())
            msg(systemErrorText(_errorCode));
        else if (_errorCode == errorCode_notSet)
            msg(_contextMsg);
        else
        {
            std::wostringstream ss;
            ss << _contextMsg << std::endl
                << systemErrorText(_errorCode);

            msg(ss.str());
        }
    }
}
