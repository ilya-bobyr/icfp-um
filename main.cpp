#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <io.h>
#include <fcntl.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "memoryManager.h"
#include "scrollReader.h"
#include "array.h"
#include "context.h"


using namespace std;
using namespace boost;
using namespace boost::filesystem;


void usage(ostream & os);

int main(int argc, const char * argv[])
{
    if (argc != 2)
    {
        usage(cerr);
        return 2;
    }

    if (_setmode(_fileno(stdin), _O_BINARY) == -1)
    {
        cerr << "Error: _setmode(stdin, BINARY) failed: " << errno << endl;
        return 2;
    }

    if (_setmode(_fileno(stdout), _O_BINARY) == -1)
    {
        cerr << "Error: _setmode(stdout, BINARY) failed: " << errno << endl;
        return 2;
    }

    try
    {
        path scrollPath(argv[1]);

        if (!exists(scrollPath))
        {
            cerr << "Error: Scroll '" << scrollPath << "' not found." << endl;
            usage(cerr);
            return 2;
        }

        if (!is_regular_file(scrollPath))
        {
            cerr << "Error: Scroll '" << scrollPath << "' is not a regular "
                        "\"file\"." << endl;
            usage(cerr);
            return 2;
        }

        uintmax_t scrollSize = file_size(scrollPath);
        if (scrollSize == static_cast<uintmax_t>(-1))
        {
            cerr << "Error: Failed to obtains '" << scrollPath << "' size."
                << endl;
            usage(cerr);
            return 2;
        }

        filesystem::ifstream scroll;
        /*
         * Without ios::binary MS Standard Library implementation will treat 1A 
         * as an EOF byte.  And, probably, '\r\n' is transformed into '\n'?
         */
        scroll.open(scrollPath, ios::in | ios::binary);
        if (!scroll.is_open())
        {
            cerr << "Error: Failed to open '" << scrollPath << "'." << endl;
            usage(cerr);
            return 2;
        }

        memoryManager mm;

        ::array * zeroArray =
            scrollReader::readLegacy(mm, scroll, 
                                     static_cast<size_t>(scrollSize));

        context ctx(mm, cin, cout, zeroArray);
        ctx.run();
    }
    catch (const std::exception & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 2;
    }

    return 0;
}

void usage(ostream & os)
{
    os << "Usage:" << endl
        << "    um <\"program\" scroll file name>" << endl;
}
