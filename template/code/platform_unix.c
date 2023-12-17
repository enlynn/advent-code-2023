
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <assert.h> //assert
#include <stdio.h>  //printf

#include "chibi_types.h"
#include "platform.h"
#include "chibi_core.h"


typedef enum 
{ //Readable wrapper for mode_t
                                      //Linux Name
    file_mode_owner_read    = 1 << 8, //S_IRUSR
    file_mode_owner_write   = 1 << 7, //S_IWUSR
    file_mode_owner_execute = 1 << 6, //S_IXUSR

    file_mode_group_read    = 1 << 5, //S_IRGRP
    file_mode_group_write   = 1 << 4, //S_IWGRP
    file_mode_group_execute = 1 << 3, //S_IXGRP

    file_mode_other_read    = 1 << 2, //S_IROTH
    file_mode_other_write   = 1 << 1, //S_IWOTH
    file_mode_other_execute = 1 << 0, //S_IXOTH

    file_mode_owner_rwx = 
        file_mode_owner_read |
        file_mode_owner_write |
        file_mode_owner_execute,  //S_IRWXU

    file_mode_group_rwx = 
        file_mode_group_read |
        file_mode_group_write |
        file_mode_group_execute,  //S_IRWXG

    file_mode_other_rwx = 
        file_mode_other_read |
        file_mode_other_write |
        file_mode_other_execute,  //S_IRWXO

    file_mode_all = 
        file_mode_owner_rwx |
        file_mode_group_rwx |
        file_mode_other_rwx,      //0777

    // NOTE(enlynn): Not sure why these are needed, but including
    // for completeness.
    file_mode_set_usr_id   = 0004000, // set user id on execution
    file_mode_set_grp_id   = 0002000, // set group id on execution
    file_mode_save_swp_txt = 0001000, // save swapped text even after use
} unix_file_mode_mask;

var_global const char* cDataEnv   = "XDG_DATA_HOME";    
var_global const char* cConfigEnv = "XDG_CONFIG_HOME";
var_global const char* cCacheEnv  = "XDG_CACHE_HOME";
var_global const char* cHomeEnv   = "HOME";

fn_internal char*
unix_build_home_dir(const char* LocalPath)
{
    const char* HomeDir =  getenv(cHomeEnv);
    if (!HomeDir) return NULL;
    
    u64 HomeLen = string_len(HomeDir);
    u64 LocalLen = string_len(LocalPath);
    u64 DesiredStringLen = HomeLen + LocalLen + 1;
    char* Result = mem_alloc(char, DesiredStringLen);

    string_concat(
            Result,    DesiredStringLen, 
            HomeDir,   HomeLen, 
            LocalPath, LocalLen);

    return Result;
}

char* platform_get_config_dir()
{
    char* Result = NULL;
    
    const char* ConfigDir = getenv(cConfigEnv);
    if (ConfigDir)
    {
        Result = string_duplicate(ConfigDir);
    }

    if (!Result)
    { // Failed to get the config env variable, let's try the home variable
        Result = unix_build_home_dir("/.config/chibi-tech");
    }

    if (!Result)
    { // Failed to get the home variable, let's just use the local path then...
        Result = string_duplicate(".");
    }

    if (Result && Result[string_len(Result) - 1] != '/')
    { // let's append /
        char* Interim = Result; 
        int Len = string_len(Interim);

        Result = mem_alloc(char, Len + 2);
        string_concat(
                Result,  Len + 2,
                Interim, Len,
                "/",     1);

        mem_free(Interim);
    }

    return Result;
}

char* platform_get_data_dir()
{
    char* Result = NULL;
    
    const char *DataDir = getenv(cDataEnv);
    if (DataDir)
    {
        Result = string_duplicate(DataDir);
    }

    if (!Result)
    { // Failed to get the config env variable, let's try the home variable
        Result = unix_build_home_dir("/.local/chibi-tech");
    }

    if (!Result)
    { // Failed to get the home variable, let's just use the local path then...
        Result = platform_get_config_dir();
    }

    if (Result && Result[string_len(Result) - 1] != '/')
    { // let's append /
        char* Interim = Result; 
        int Len = string_len(Interim);

        Result = mem_alloc(char, Len + 2);
        string_concat(
                Result,  Len + 2,
                Interim, Len,
                "/",     1);

        mem_free(Interim);
    }

    return Result;
}

char* platform_get_cache_dir()
{
    char* Result = NULL;
    
    const char *CacheDir = getenv(cCacheEnv);
    if (CacheDir)
    {
        Result = string_duplicate(CacheDir);
    }

    if (!Result)
    { // Failed to get the config env variable, let's try the home variable
        Result = unix_build_home_dir("/.cache/chibi-tech");
    }

    if (!Result)
    { // Failed to get the home variable, let's just use the local path then...
        Result = platform_get_config_dir();
    }

    if (Result && Result[string_len(Result) - 1] != '/')
    { // let's append /
        char* Interim = Result; 
        int Len = string_len(Interim);

        Result = mem_alloc(char, Len + 2);
        string_concat(
                Result,  Len + 2,
                Interim, Len,
                "/",     1);

        mem_free(Interim);
    }

    return Result;
}

bool platform_file_exists(const char* Filepath)
{
    struct stat LinuxFile;
    int PathExists = stat(Filepath, &LinuxFile);
    return PathExists != -1;
}

void platform_mkdir(const char* Filepath)
{
    if (!platform_file_exists(Filepath))
    {
        int Result = mkdir(Filepath, file_mode_all);
        if (Result != 0)
            log_error("Failed to create directory: %s", Filepath);
    }
}

u64 unix_get_file_size(const char* Filepath)
{
    // NOTE(enlynn): for now, assume file exists 
    struct stat FileInfo;
    int PathExists = stat(Filepath, &FileInfo);
    return (PathExists != -1) ? FileInfo.st_size : 0;
}

file_io_read_result platform_read_entire_file(const char* Filepath)
{
    file_io_read_result Result = {
        .Error    = file_io_none,
        .FileData = NULL,
    };

    if (platform_file_exists(Filepath))
    {
        u64 FileSize = unix_get_file_size(Filepath);
        if (FileSize == 0)
        {
            Result.Error = file_io_file_not_found;
            goto LBL_ERROR;
        }

        int FileFlags = O_RDONLY;
        int FilePtr = open(Filepath, FileFlags); 
        if (FilePtr == -1)
        {
            Result.Error = file_io_failed_to_open;
            goto LBL_ERROR;
        }
        
        byte* FileData = mem_alloc(byte, FileSize + 1);
        size_t ReadBytes = read(FilePtr, FileData, FileSize); 

        if (ReadBytes == -1)
        {
            Result.Error = file_io_failed_to_read;
            mem_free(FileData);
        }
        else 
        {
            FileData[ReadBytes] = 0;

            Result.FileData = FileData;
            Result.FileSize = FileSize;
        }

        int CloseResult = close(FilePtr);
        cassert_custom(CloseResult != -1, "Failed to close a file opened for reading.");
    }
    else 
    {
        Result.Error = file_io_file_not_found;
    }

LBL_ERROR:
    return Result;
}

file_io_error platform_write_entire_file(const char* Filepath, void* FileData, u64 NumBytesToWrite, bool Append)
{
    file_io_error Result = file_io_none;

    bool FileExists = platform_file_exists(Filepath);
    {
        int FileMode =  FileExists ? 0 : file_mode_all;
        int FileFlags = O_WRONLY | ((Append) ? O_APPEND : O_TRUNC);
        if (!FileExists) FileFlags |= O_CREAT;

        int FilePtr = open(Filepath, FileFlags, FileMode); 
        if (FilePtr == -1)
        {
            Result = file_io_failed_to_open;
            goto LBL_ERROR;
        }
        
        ssize_t WriteResult = write(FilePtr, FileData, NumBytesToWrite);
        if (WriteResult == -1)
        {
            Result = file_io_failed_to_write;
        }

        int CloseResult = close(FilePtr);
        cassert_custom(CloseResult != -1, "Failed to close a file opened for reading.");
    }

LBL_ERROR:
    return Result;
}

void platform_debug_break()
{
    // TODO(enlynn): ideally would popup an error box, but don't know of way that isn't 
    // dependent on Windows and X. It wouldn't work on Wayland. The plan is to place an 
    // error popup in the Editor once the UI is working. 
    assert(false);
}

void platform_log_to_console(const char* Buffer, u32 BufferSize, int LogLevel)
{
    // TODO: fix the print color approach 
    const char* ColorStrings[] = {
        "1;30", "1;34", "1;32", "1;33", "1;31", "0;41",
    };

    // set the console color 
    printf("\033[%sm", ColorStrings[LogLevel]);

    // print the message 
    printf("%s", Buffer);

    // Reset the console color
    printf("\033[0m");
}

void platform_log_to_debug_console(const char* Buffer, u32 BufferSize, int LogLevel)
{ // Nothing to do on linux
}

void platform_log_to_file(const char* Buffer, u32 BufferSize, int LogLevel)
{ // TODO(enlynn):
}

u32 platform_get_page_size()
{
    return sysconf(_SC_PAGESIZE);
}

void platform_virtual_free(void* Ptr, u64 Size)
{
    Size = forward_align(Size, platform_get_page_size()); 
    int Result = munmap(Ptr, Size);
    assert(Result == 0);
    //TODO(enlynn): Properly error check failure
}

void* platform_virtual_reserve_memory(u64 Size)
{
    int ProtectionFlags = PROT_NONE;
    int Flags           = MAP_PRIVATE | MAP_ANONYMOUS;
    Size = forward_align(Size, platform_get_page_size()); 

    void* Result = mmap(NULL, Size, ProtectionFlags, Flags, -1, 0);
    cassert(Result != (void*)-1); 
    // TODO(enlynn): Properly error check failure

    return Result;
}

void platform_virtual_map_to_physical(void* BasePtr, u64 Offset, u64 PageRange)
{
    int ProtectionFlags = PROT_READ | PROT_WRITE;

    PageRange = forward_align(PageRange, platform_get_page_size()); 
    Offset    = forward_align(Offset,    platform_get_page_size()); 
    void* Ptr = (byte*)BasePtr + Offset;

    int Result = mprotect(Ptr, PageRange, ProtectionFlags);
    cassert(Result != -1);
    //TODO(enlynn): Properly error check failure
}

void* platform_load_library(const char* Library)
{
    //if (platform_file_exists(Library)) return NULL;
    //
    //NOTE(enlynn): If we prefix the library name with "lib" then can't find the file!

    const int Flags = RTLD_NOW;
    void* Result = dlopen(Library, Flags);
    if (!Result)
    {
        const char* Error = dlerror();
        log_error("Failed to open library %s for reason: %s", Library, Error);
    }

    return Result;
}

void platform_unload_library(void* Library)
{
    if (Library)
        dlclose(Library);
}

void* platform_load_function(void* Library, const char* FunctionName)
{
    if (!Library || !FunctionName)
        return NULL;

    void* Result = dlsym(Library, FunctionName);
    if (!Result)
    {
        const char* Error = dlerror();
        log_error("Failed to load function from a library. Requested Function Name: %s for reason %s", 
                FunctionName, Error);
    }

    return Result;
}

//
// Crash Reporter
//

#define __USE_GNU

#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <fcntl.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>

#if defined(__x86_64__)
#define _PC gregs[REG_RSP]
#define _SP gregs[REG_RIP]
#else
#error architecture not supported
#endif

fn_internal void 
unix_segfault_handler(int sig, siginfo_t *psi, void* ctxarg)
{
    void *trace[10];
    char **messages = (char**)0;
    int i, trace_size = 0;
    mcontext_t *ctxP = &((ucontext_t *) ctxarg)->uc_mcontext;

    trace_size = backtrace(trace, 10);
    /* overwrite sigaction with caller's address */
    trace[1] = (void*)ctxP->_SP;
    messages = backtrace_symbols(trace, trace_size);
    /* skip first stack frame (points here) */
    log_error("Seg Fault Exectuion Path: Signal %d, Family Addr: %p from %p", sig, (void*)ctxP->_PC, (void*)ctxP->_SP);
    for (i = 0; i < trace_size; ++i)
    {
        fprintf(stderr, "\t#%d %s :: ", i, messages[i]);

        /* find first occurence of '(' or ' ' in message[i] and assume
         * everything before that is the file name. (Don't go beyond 0 though
         * (string terminator)*/
        size_t p = 0;
        while(messages[i][p] != '(' && messages[i][p] != ' '
            && messages[i][p] != 0)
        {
          ++p;
        }

        char syscom[256];
        sprintf(syscom,"addr2line %p -e %.*s", trace[i], (int)p, messages[i]);
        //last parameter is the file name of the symbol
        system(syscom);
    }

    exit(1);
}

void platform_setup_segfault_handler()
{
    struct sigaction sig;
    sig.sa_sigaction = (void*)unix_segfault_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_RESTART|SA_SIGINFO;
    sigaction(SIGSEGV, &sig, NULL);
    sigaction(SIGUSR1, &sig, NULL);
}
