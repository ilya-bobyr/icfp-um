#include "utils.h"

#include <sstream>
#include <iomanip>

using namespace std;

wstring systemErrorText(DWORD errorCode, DWORD langId)
{
    HLOCAL pRes = 0;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER 
                      | FORMAT_MESSAGE_IGNORE_INSERTS
                      | FORMAT_MESSAGE_FROM_SYSTEM,     /* dwFlags */
                      NULL,                             /* lpSource */
                      errorCode,                        /* dwMessageId */
                      langId,                           /* dwLanguageId */
                      reinterpret_cast<LPTSTR>(&pRes),  /* lpBuffer */
                      0,                                /* nSize */
                      0                                 /* Arguments */
                     ) == 0)
    {
	wostringstream ss;
	ss << L"[Could not find a description for error 0x" 
            << hex << setiosflags(ios_base::uppercase) << errorCode << L"]";
	return ss.str();
    }
    else
    {
	wstring res(reinterpret_cast<LPTSTR>(pRes));
	LocalFree(pRes);
	return res;
    }
}

wstring COMErrorText(HRESULT hr)
{
    if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
        hr = HRESULT_CODE(hr);

    HLOCAL pRes = 0;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_IGNORE_INSERTS
                      | FORMAT_MESSAGE_FROM_SYSTEM,     /* dwFlags */
                      NULL,                             /* lpSource */
                      hr,                               /* dwMessageId */
                      0,                                /* dwLanguageId */
                      reinterpret_cast<LPTSTR>(&pRes),  /* lpBuffer */
                      0,                                /* nSize */
                      0                                 /* Arguments */
                     ) == 0)
    {
        wostringstream ss;
        ss << L"[Could not find a description for error 0x" 
	    << hex << setiosflags(ios_base::uppercase) << hr << L"]";
	return ss.str();
    }
    else
    {
	wstring res(reinterpret_cast<LPTSTR>(pRes));
	LocalFree(pRes);
	return res;
    }
}

void quoteAsCommandComponent(wstring & arg)
{
    /* Check if we need to add quotes. */
    if (arg.find_first_of(L" \"") == wstring::npos)
        return;

    /* First we escape all double quotes. */
    wstring::size_type p = 0;
    while (true)
    {
        p = arg.find(L"\"", p);

        if (p == wstring::npos)
            break;

        arg.replace(p, 1, L"\\\"");

        ++p;
    }

    /* 
     * Now quote a backslash if there is any at the end.  Otherwise it will 
     * quote the closing quote we are going to add.  Note that other backslashes 
     * are either already originally escaped or they have literal meaning.
     */
    if (arg.length() > 0 && arg[arg.length() - 1] == L'\\')
        arg += L'\\';

    arg.insert(0, L"\"");
    arg += L'"';
}

/*
 * Creates all the missing directories, making it possible to create a file 
 * with the specified path and an arbitrary name.
 */
void ensurePathExists(const std::wstring & path)
    throw(exceptions::systemError)
{
    wstring p = path;

    /* Replace all forward slashes with backslashes. */
    for (wstring::size_type i = p.find(L'/'); 
         i != wstring::npos; 
         i = p.find(L'/', i))
        p.replace(i, 1, 1, L'\\');

    /* 
     * Index of the next character after previous backslash or of the start 
     * of the path. 
     */
    wstring::size_type i = 0;

    /* An index of the next backslash. */
    wstring::size_type j = p.find(L'\\');

    /* Skip the drive letter if any. */
    if (j == 2 && p[1] == L':')
    {
        i = j;
        j = p.find(L'\\', j + 1);
    }

    /* Quit if there is nothing to do. */
    if (j == wstring::npos
        && i == p.size() - 1)
        return;

    /* Make sure all the path components exist. */
    while (true)
    {
        /* Skip over sequences of backslashes, '.' and '..' entries. */
        if (!(i == j
              || (i + 1 == j
                  && p[i] != L'.')
              || (i + 2 == j
                  && p[i] != L'.'
                  && p[i + 1] != L'.'))
           )
        {
            if (j != wstring::npos)
                p[j] = L'\0';

            BOOL res = CreateDirectoryW
                (p.c_str()   /* lpPathName */, 
                 0           /* lpSecurityAttributes */
                );

            if (!res)
            {
                DWORD errorCode = GetLastError();

                if (errorCode != ERROR_ALREADY_EXISTS)
                    throw exceptions::systemError(errorCode);
            }

            if (j != wstring::npos)
                p[j] = L'\\';
        }

        if (j == wstring::npos)
            break;

        i = j + 1;
        j = p.find(L'\\', i);
    }
}

wstring getDirectory(const wstring & path) throw()
{
    if (path.empty())
        return path;

    wstring::size_type i = path.find_last_of(L"\\/");

    if (i == path.size() - 1)
        return path;

    if (i == wstring::npos)
        return L"";

    return path.substr(0, i);
}
