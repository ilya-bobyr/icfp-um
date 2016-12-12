#ifndef __SCROLL_READER__H
#define __SCROLL_READER__H

#include <stdexcept>
#include <iosfwd>

class memoryManager;
class array;

/*
 * Reads a "program" scroll of platters into an array.  It would probably used 
 * as a 0 array for a um context.
 *
 * Supports legacy "unsigned 8-bit character" scrolls.
 */
class scrollReader
{
public:
    /*
     * `size' is a number of  "unsigned 8-bit characters" to read from the 
     * scroll stream.
     *
     * Throws invalid_argument if sizeHint % 4 != 0.
     * Throws runtime_error is scroll contains number of characters that is not 
     *        a multiple of 4.
     */
    static array * readLegacy(memoryManager & mm,
                              std::istream & scroll, size_t size)
        throw(std::invalid_argument, std::runtime_error);
};

#endif /* __SCROLL_READER__H */
