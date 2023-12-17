#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "chibi_types.h"

//
// General Platform code
//

void platform_debug_break();

// TODO(enlynn): For the log functions, pass the foreground and background colors instead.
void platform_log_to_console(const char* Buffer, u32 BufferSize, int LogLevel);
void platform_log_to_debug_console(const char* Buffer, u32 BufferSize, int LogLevel);
void platform_log_to_file(const char* Buffer, u32 BufferSize, int LogLevel);
void platform_setup_segfault_handler();

//
// File I/O 
//

typedef enum 
{
    file_type_unknown,
    file_type_file,
    file_type_directory,
} file_type;

typedef enum
{
    file_io_none,
    file_io_wrong_file_type,
    file_io_file_not_found,
    file_io_failed_to_open,
    file_io_failed_to_read,
    file_io_failed_to_write,
} file_io_error;

typedef struct 
{
    file_io_error Error;
    void*         FileData;
    u64           FileSize;
} file_io_read_result;

// Retrieves a platform dependent local directory for the engine. Will allocate
// the path, it is the responsibility of the caller to free the string.
//
// Linux: $HOME/.local/share/chibi-tech 
// Windows: %APP_DATA%/chibi-tech/
char* platform_get_data_dir();
char* platform_get_config_dir();
char* platform_get_cache_dir();

bool platform_file_exists(const char* Filepath);
void platform_mkdir(const char* Filepath);

file_io_read_result platform_read_entire_file(const char* Filepath);
file_io_error platform_write_entire_file(const char* Filepath, void* FileData, u64 NumBytesToWrite, bool Append);

void* platform_load_library(const char* Library);
void  platform_unload_library(void* Library);
void* platform_load_function(void* Library, const char* FunctionName);

//
// Memory 
//

u32 platform_get_page_size();
void platform_virtual_free(void* Ptr, u64 AllocationSize);
void* platform_virtual_reserve_memory(u64 Size);
void platform_virtual_map_to_physical(void* BasePtr, u64 Offset, u64 PageRange);

#endif //_PLATFORM_H_
