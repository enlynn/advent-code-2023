#include "chibi_types.h"
#include "platform.h"
#include "chibi_core.h"
#include "darray.h"

int main(void)
{
    s64 LoggerSize = logger_get_mem_requirements();
    void* Logger = mem_alloc(byte, LoggerSize);
    logger_initialize(Logger);

    // Now do it...
    log_info("Hello advent of code");

    return 0;
}



// -------------------------------------------------------------------
// Source Code from ther files

#include "chibi_core.c" 
#include "darray.c"
#include "platform_unix.c"