Universal Machine JIT VM
========================

Another implementation of a Universal Machine VM from the [ICFP of
2006](http://www.boundvariable.org/index.shtml).  This is the fastest
implementation of all I was able to find online.  See [results](/results) for a
quick list of sandmark.umz execution times.

It is Win32 only and 32-bit only.  But the whole idea is to have a JIT code
generation.

Passes the sandmark test and runs the codex.

The next optimization steps would be to match certain code patterns and
generate more efficient native code blocks for them.  I've disassembled the
sandmark a bit and found that the compiler into the UM code generates large
patterns, matching the source language operations.  For example, string
output is a sequence of Orthography operations, followed by a sequence of
Output operations.  This can be represented by a much more concise native
code block that is generated when every platter is compiled independently.
But this is essentially an area of compilers, very broad.  Not sure if I
want to invest this much time into this project :)
