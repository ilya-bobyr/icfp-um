#ifndef __UTILS_H
#define __UTILS_H

#include "windows.h"

#include <string>

#include "exceptions/systemError.h"

/*
 * Returns error text for a specified system error code.  Optional language 
 * specifies error text language.
 */
std::wstring systemErrorText(DWORD errorCode, DWORD langId = 0);

/*
 * Returns error text for a system COM error code.
 */
std::wstring COMErrorText(HRESULT hr);

/*
 * Adds double quotes around the string in case they are needed for the string 
 * to be treated as a single component of a command line by functions simmilar 
 * to CreateProcess.
 *
 * Also escapes all double quotes inside the argument if they are present.  
 * Modifies the passed in string.
 */
void quoteAsCommandComponent(std::wstring & arg);


/*
 * Creates all the missing directories, making it possible to create a file 
 * with the specified path and an arbitrary name.
 */
void ensurePathExists(const std::wstring & path)
    throw(exceptions::systemError);

/*
 * Remove the last component from a path unless the path ends with a slash.  In 
 * the later case the path is returned as is.
 *
 * If the path does not contain a component separator it is treated as been a 
 * file name and an empty string is returned.
 */
std::wstring getDirectory(const std::wstring & path) throw();

#endif /* __UTILS_H */
