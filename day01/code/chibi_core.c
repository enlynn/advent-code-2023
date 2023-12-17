#include "chibi_core.h"
#include "platform.h"

#include <assert.h>

void report_assertion_failure(const char* Condition, const char* File, int Line, const char* Message)
{
    logger_log(log_level_error, File, Line, "Assertion Failure: %s. Message: %s.", Condition, Message);
}

void assert_debug_break()
{
    //platform_debug_break();
}

typedef struct 
{
    int MinLogLevel;
    int LogMode;
} logger_context;

var_global logger_context* gState = NULL;

s64 logger_get_mem_requirements()
{
    return sizeof(logger_context);
}

void logger_initialize(void* Context)
{
    cassert(Context);

    gState = Context;
    mem_zero(gState, sizeof(logger_context));

    gState->MinLogLevel = log_level_debug;
    gState->LogMode     = log_mode_console;

    // Primarily for Windows - Visual Studio has a special debug console 
#ifdef DEBUG_BUILD
    gState->LogMode |= log_mode_debug_console;
#endif
}

void logger_shutdown()
{
    gState = NULL;
}

void logger_set_min_log_level(int MinLogLevel)
{
    gState->MinLogLevel = MinLogLevel;
}

void logger_set_log_mode(int LogModeBitmask)
{
    gState->LogMode = LogModeBitmask;
}

void logger_log(int LogLevel, const char* File, int Line, const char* Fmt, ...)
{
    if (LogLevel < gState->MinLogLevel) return;

    const char* LogLevelStrings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };

    // Get the necessary size for the Log Header and the Log Message

    // ThreadId [LogLevel] File:Line: 
    const char* LogHeader = "%d\t[%s]\t %s:%d: ";
    int LogHeaderSize = string_format(NULL, 0, LogHeader, 0, LogLevelStrings[LogLevel], File, Line); 

    va_list Args;
    va_start(Args, Fmt);
    int LogSize = string_vformat(NULL, 0, Fmt, Args);
    va_end(Args);

    // Allocate the log memory, and fill out the log 
    // Format: Header: Message\n
    int TotalLogSize = LogHeaderSize + LogSize + 1;
    cassert(TotalLogSize < 2047);

    char Message[2048];
    mem_zero(Message, sizeof(Message));

    LogHeaderSize = string_format(Message, 2048, LogHeader, 0, LogLevelStrings[LogLevel], File, Line); 
    
    va_start(Args, Fmt);
    string_vformat(Message + LogHeaderSize, 2048, Fmt, Args);
    va_end(Args);

    Message[TotalLogSize - 1] = '\n';
    Message[TotalLogSize]     = 0;

    // Write to the console

    if ((gState->LogMode & log_mode_debug_console) != 0)
        platform_log_to_debug_console(Message, TotalLogSize, LogLevel);
    if ((gState->LogMode & log_mode_console) != 0)
        platform_log_to_console(Message, TotalLogSize, LogLevel);
    if ((gState->LogMode & log_mode_file) != 0)
        platform_log_to_file(Message, TotalLogSize, LogLevel);

    if (LogLevel == log_level_fatal)
        platform_debug_break();
}

#include <string.h>
#include <ctype.h> //isspace
#include <stdio.h>
#include <stdlib.h>

u64 string_len(const char* String)
{
    return strlen(String);
}

void string_concat(
        char* Destination,   u64 DestinationSize,
        const char* SourceA, u64 SourceASize,
        const char* SourceB, u64 SourceBSize)
{
    if (SourceASize + SourceBSize > DestinationSize)
    {
        //TODO: log
        return;
    }

#if 0
    for (u64 Index = 0; Index < SourceASize; ++Index)
        *Destination++ = *SourceA++;

    for (u64 Index = 0; Index < SourceBSize; ++Index)
        *Destination++ = *SourceB++;

    *Destination++ = 0;
#else 
    string_ncopy(Destination, SourceA, SourceASize);
    string_ncopy(Destination + SourceASize, SourceB, SourceBSize);
#endif
}

bool string_compare(
        const char* SourceA, u64 SourceASize,
        const char* SourceB, u64 SourceBSize)
{
    if (SourceASize != SourceBSize) 
        return false;

    for (s64 Index = 0; Index < SourceASize; ++Index)
    {
        if (*SourceA++ != *SourceB++) 
            return false;
    }

    return true;
}

// duplicate and ncopy allocate memory
char* string_duplicate(const char* SourceString)
{
    u64 StringLen = string_len(SourceString);

    char* Result = mem_alloc(char, StringLen + 1);
    mem_copy(Result, SourceString, StringLen);
    Result[StringLen] = 0;
    return Result;
}

void string_ncopy(char* Destination, const char* SourceA, u64 NumCharsToCopy)
{
#if 0
    for (u64 Index = 0; Index < NumCharsToCopy; ++Index)
    {
        *Destination++ = *SourceA++;
    }

    *Destination++ = 0;
#else 
    strncpy(Destination, SourceA, NumCharsToCopy);
    Destination[NumCharsToCopy] = 0;
#endif
}

int string_format(char* DestinationBuffer, u64 DestinationBufferSize, const char* Fmt, ...)
{
    va_list Args;
    va_start(Args, Fmt);
    int Result = string_vformat(DestinationBuffer, DestinationBufferSize, Fmt, Args);
    va_end(Args);
    return Result;
}

int string_vformat(char* Buffer, u64 BufferSize, const char* Fmt, va_list Args)
{
    return vsnprintf(Buffer, BufferSize, Fmt, Args);
}

char* string_trim(char* Source)
{
    // Trim preceeding whitespace
    while (isspace((unsigned char)*Source))
    {
        Source++;
    }

    if (*Source)
    {
        char* Iter = Source;
        while(*Iter) { ++Iter; } // find the end of the string 

        // TODO(enlynn): Test trailing whitespace, i am suspicious this won't actually work.
        // now trim the trailing whitespace
        while(isspace((unsigned char)*Iter) || *Iter == '\r') { --Iter; }

        // Insert a new null terminator
        Iter[1] = 0;
    }

    return Source;
}

u64 string_read_to_delim(char* Destination, char* Source, char Delimiter)
{
    char* Iter = Source;
    while(*Iter && (*Iter != Delimiter))
    {
        *Destination++ = *Iter++;
    }

    u64 CharsRead = Iter - Source;
    *Destination++ = 0;

    return CharsRead;
}

bool string_to_f32(char* Str, f32* OutF32)
{
    if (Str)
    {
        *OutF32 = 0.0f;
        s32 Result = sscanf(Str, "%f", OutF32);
        return Result != -1;
    }
    return false;
}

bool string_to_int(char* Str, s32* OutS32)
{
    if (Str)
    {
        *OutS32 = 0.0f;
        s32 Result = sscanf(Str, "%i", OutS32);
        return Result != -1;
    }
    return false;
}

bool string_uint(char* Str, u32* OutU32)
{
    if (Str)
    {
        *OutU32 = 0.0f;
        s32 Result = sscanf(Str, "%u", OutU32);
        return Result != -1;
    }
    return false;
}

void* chibi_memory_alloc(u64 Size) { return malloc(Size); }
void  chibi_memory_free(void* Memory) { free(Memory); }

void  
chibi_memory_set(void* Memory, int Byte, u64 Size)
{
    u8* Iter = (u8*)Memory;
    while (Size--) { *Iter++ = Byte; } 
}

void  
chibi_memory_copy(void* Destination, void* Source, u64 CopySize)
{
    memcpy(Destination, Source, CopySize);
}

bool chibi_memory_cmp(void* Left, void* Right, u64 Size)
{
    return memcmp(Left, Right, Size) == 0;
}
