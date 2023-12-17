#ifndef _CHIBI_CORE_H_
#define _CHIBI_CORE_H_

#include "chibi_types.h"

#if defined(_NO_ASSERTS_)
#  define cassert(Condition)
#  define cassert_custom(Condition, Message)
#else 
#  define cassert(Condition) assert(Condition)
#  define cassert_custom(Condition, Message) \
    if (Condition) {} else {                 \
        log_error(Message);                  \
        assert(Condition);                   \
    }
#endif

void report_assertion_failure(const char* Condition, const char* File, int Line, const char* Message);
void assert_debug_break();

typedef enum 
{
    log_mode_unknown = 0x00,
    log_mode_debug_console = 0x01,
    log_mode_console       = 0x02,
    log_mode_file          = 0x04,
} log_mode;

typedef enum 
{
    log_level_trace,
    log_level_debug,
    log_level_info,
    log_level_warn,
    log_level_error,
    log_level_fatal,
} log_level;

#define log_trace(Message, ...) logger_log(log_level_trace, __FILE__, __LINE__, Message, ##__VA_ARGS__) 
#define log_debug(Message, ...) logger_log(log_level_debug, __FILE__, __LINE__, Message, ##__VA_ARGS__) 
#define log_info(Message, ...)  logger_log(log_level_info,  __FILE__, __LINE__, Message, ##__VA_ARGS__) 
#define log_warn(Message, ...)  logger_log(log_level_warn,  __FILE__, __LINE__, Message, ##__VA_ARGS__) 
#define log_error(Message, ...) logger_log(log_level_error,  __FILE__, __LINE__, Message, ##__VA_ARGS__) 
#define log_fatal(Message, ...) logger_log(log_level_fatal,  __FILE__, __LINE__, Message, ##__VA_ARGS__) 

s64 logger_get_mem_requirements();
void logger_initialize(void* Context);
void logger_shutdown();

void logger_set_min_log_level(int MinLogLevel);
void logger_set_log_mode(int LogModeBitmask);
void logger_log(int LogLevel, const char* File, int Line, const char* Fmt, ...);

u64 string_len(const char* String);
void string_concat(
        char* Destination,   u64 DestinationSize,
        const char* SourceA, u64 SourceASize,
        const char* SourceB, u64 SourceBSize);

bool string_compare(
        const char* SourceA, u64 SourceASize,
        const char* SourceB, u64 SourceBSize);

// duplicate and ncopy allocate memory
char* string_duplicate(const char* SourceString);

void  string_ncopy(char* Destination, const char* SourceA, u64 NumCharsToCopy);

// format and vformat will return the size of the Destination Buffer required for the format. 
// If querying the size of the buffer, pass NULL for format
int string_format(char* DestinationBuffer, u64 DestinationBufferSize, const char* Fmt, ...);
int string_vformat(char* Buffer, u64 BufferSize, const char* Fmt, va_list Args);

// Trim any whitespace from the Source. Does not allocate memory and modifies the original string.
char* string_trim(char* Source);

u64 string_read_to_delim(char* Destination, char* Source, char Delimiter);

bool string_to_f32(char* Str, f32* OutF32);
bool string_to_int(char* Str, s32* OutS32);
bool string_uint(char* Str, u32* OutU32);

#define mem_alloc(Type, Count)              (Type*)chibi_memory_alloc(sizeof(Type) * (Count))
#define mem_free(Memory)                    chibi_memory_free((void*)Memory)
#define mem_set(Memory, Value, Size)        chibi_memory_set((void*)Memory, Value, Size)
#define mem_zero(Memory, Size)              chibi_memory_set((void*)Memory, 0, Size)
#define mem_copy(Destination, Source, Size) chibi_memory_copy((void*)Destination, (void*)Source, Size)
#define mem_cmp(Left, Right, Size)          chibi_memory_cmp((void*)Left, (void*)Right, Size)

void* chibi_memory_alloc(u64 Size);
void  chibi_memory_free(void* Memory);
void  chibi_memory_set(void* Memory, int Value, u64 Size);
void  chibi_memory_copy(void* Destination, void* Source, u64 Size);
bool  chibi_memory_cmp(void* Left, void* Right, u64 Size);


#endif