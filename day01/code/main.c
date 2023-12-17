// -------------------------------------------------------------------
// Headers

#include "chibi_types.h"
#include "platform.h"
#include "chibi_core.h"
#include "darray.h"

// -------------------------------------------------------------------
// Implementation

//
// Find the first and last character
// Combine them in to a single number (i.e. 1 and 7 -> 17)
// Find the sum of all lines
// 

fn_inline bool is_digit(char Val)               { return Val >= '0' && Val <= '9';   }
fn_inline bool is_line_end(char Val)            { return Val == '\r' || Val == '\n'; }
fn_inline int  make_number(int Tens, int Zeros) { return (Tens * 10) + Zeros;        }
fn_inline int  char_to_digit(char Val)          { return Val - '0';                  }

// If not word digit, return -1
// else returns the digit
fn_inline int is_word_digit(char* Digit, int Length)
{
    switch(Digit[0])
    {
        case 'z': //zero
        {
            if (Length < 4) return -1;
            if (string_compare(Digit + 1, 3, "ero", 3))
                return 0;
        } break;

        case 'o': //one
        {
            if (Length < 3) return -1;
            if (string_compare(Digit + 1, 2, "ne", 2))
                return 1;
        } break;
        
        case 't': //two, three
        {
            if (Length < 3) return -1;
            if (string_compare(Digit + 1, 2, "wo", 2))
                return 2;

            if (Length < 5) return -1;
            if (string_compare(Digit + 1, 4, "hree", 4))
                return 3;
        } break;
        
        case 'f': //four, five
        {
            if (Length < 4) return -1;
            if (string_compare(Digit + 1, 3, "our", 3))
                return 4;
            if (string_compare(Digit + 1, 3, "ive", 3))
                return 5;
        } break;
        
        case 's': //six, seven
        {
            if (Length < 3) return -1;
            if (string_compare(Digit + 1, 2, "ix", 2))
                return 6;

            if (Length < 5) return -1;
            if (string_compare(Digit + 1, 4, "even", 4))
                return 7;
        } break;
        
        case 'e': //eight
        {
            if (Length < 5) return -1;
            if (string_compare(Digit + 1, 4, "ight", 4))
                return 8;
        } break;
        
        case 'n': //nine
        {
            if (Length < 4) return -1;
            if (string_compare(Digit + 1, 3, "ine", 3))
                return 9;
        } break;
        
        default: return -1;
    }
    return -1;
}

// Returns the start of the next line
fn_inline char* get_line(char* Start, char* StrEnd, char** OutLineEnd, int* OutLineLen)
{
    char* Iter = Start;
    while (Iter < StrEnd)
    {
        if (is_line_end(*Iter))
        {
            *OutLineEnd  = Start;
            *OutLineLen = Iter - Start;
            break;
        }

        Iter += 1;
    }

    if (Iter == StrEnd)
    { // This was the last line
        *OutLineEnd = Start;
        *OutLineLen = Iter - Start;
    }
    else 
    { // Lets go ahead and consume the line endings for the next line
        while (Iter < StrEnd)
        {
            if (!is_line_end(*Iter)) break;
            Iter += 1;
        }
    }

    return Iter;
}

fn_inline int parse_line(char* Start, int Length)
{
    int Tens  = 0;
    int Zeros = 0;

    bool FoundTens  = false;
    bool FoundZeros = false;

    int FirstBackIndex = Length - 1;
    ForRange(int, i, Length)
    {
        int FrontIndex = i;
        int BackIndex  = FirstBackIndex - i;

        char Front = Start[FrontIndex];
        char Back  = Start[BackIndex];

        if (is_digit(Front) && !FoundTens)
        {
            FoundTens = true;
            Tens = char_to_digit(Front);
        }

        if (is_digit(Back) && !FoundZeros)
        {
            FoundZeros = true;
            Zeros = char_to_digit(Back);
        }

        // Found front and back, we are done
        if (FoundTens && FoundZeros) break;
    }

    return make_number(Tens, Zeros);
}

fn_inline int parse_line_with_words(char* Start, int Length)
{
    int Tens  = 0;
    int Zeros = 0;

    bool FoundTens  = false;
    bool FoundZeros = false;

    int FirstBackIndex = Length - 1;
    ForRange(int, i, Length)
    {
        int FrontIndex = i;
        int BackIndex  = FirstBackIndex - i;

        char Front = Start[FrontIndex];
        char Back  = Start[BackIndex];

        if (!FoundTens)
        {
            if (is_digit(Front))
            {
                FoundTens = true;
                Tens = char_to_digit(Front);
            }
            else
            {
                int Digit = is_word_digit(Start + FrontIndex, Length - i);
                if (Digit > -1)
                {
                    FoundTens = true;
                    Tens = Digit;
                }
            }
        }

        if (!FoundZeros)
        {
            if (is_digit(Back))
            {
                FoundZeros = true;
                Zeros = char_to_digit(Back);
            }
            else
            {
                int Digit = is_word_digit(Start + BackIndex, Length - BackIndex);
                if (Digit > -1)
                {
                    FoundZeros = true;
                    Zeros = Digit;
                }
            }
        }

        // Found front and back, we are done
        if (FoundTens && FoundZeros) break;
    }

    return make_number(Tens, Zeros);
}

int compute_sum(char* Line, char* LineEnd)
{
    int Sum = 0;

    while (Line < LineEnd)
    {
        char* CurrentLine    = NULL;
        int   CurrentLineLen = 0;

        Line = get_line(Line, LineEnd, &CurrentLine, &CurrentLineLen);
        cassert(CurrentLine && CurrentLine > 0);

        int LineNumber = parse_line(CurrentLine, CurrentLineLen);
        //log_debug("Line Number %d", LineNumber);

        Sum += LineNumber;
    }

    return Sum;
}

int compute_sum_extra(char* Line, char* LineEnd)
{
    int Sum = 0;

    while (Line < LineEnd)
    {
        char* CurrentLine    = NULL;
        int   CurrentLineLen = 0;

        Line = get_line(Line, LineEnd, &CurrentLine, &CurrentLineLen);
        cassert(CurrentLine && CurrentLine > 0);

        int LineNumber = parse_line_with_words(CurrentLine, CurrentLineLen);
        //log_debug("Line Number %d", LineNumber);

        Sum += LineNumber;
    }

    return Sum;
}

void run_part1()
{
    char* Line;
    char* InputEnd;
    int Sum = 0;

#if 0
    char* Sample = 
        "1abc2\n"
        "pqr3stu8vwx\n"
        "a1b2c3d4e5f\n"
        "treb7uchet\n\0"
    ;

    int SampleLength = string_len(Sample);

    Line = Sample;
    InputEnd = Line + SampleLength;

    // First example input
    Sum = compute_sum(Line, InputEnd);
    cassert(Sum == 142);
#endif

    // Test the second example input
    file_io_read_result Result = platform_read_entire_file("input_p1.txt");
    cassert(Result.Error == file_io_none);

    clock_t Begin = clock();

    Line = Result.FileData;
    InputEnd = Result.FileData + Result.FileSize;
    Sum = compute_sum(Line, InputEnd);

    clock_t End = clock();

    log_info("FINAL SUM PART 1: %d", Sum);
    log_info("Part 1 Timing: %lf", clock_ms(Begin, End));
}

void run_part2()
{
    char* Line;
    char* InputEnd;
    int Sum = 0;

#if 0
    char* Sample = 
        "two1nine\n"
        "eightwothree\n"
        "abcone2threexyz\n"
        "xtwone3four\n"
        "4nineeightseven2\n"
        "zoneight234\n"
        "7pqrstsixteen\n\0"
    ;

    int SampleLength = string_len(Sample);

    Line = Sample;
    InputEnd = Line + SampleLength;

    // First example input
    Sum = compute_sum_extra(Line, InputEnd);
    log_info("Sample sum: %d", Sum);
    cassert(Sum == 281);
#endif

    // FINAL INPUT

    file_io_read_result Result = platform_read_entire_file("input_p2.txt");
    cassert(Result.Error == file_io_none);

    clock_t Begin = clock();

    Line = Result.FileData;
    InputEnd = Result.FileData + Result.FileSize;
    Sum = compute_sum_extra(Line, InputEnd);

    clock_t End = clock();

    log_info("FINAL SUM PART 2: %d", Sum);
    log_info("Part 2 Timing: %lf", clock_ms(Begin, End));
}

int main(void)
{
    s64 LoggerSize = logger_get_mem_requirements();
    void* Logger = mem_alloc(byte, LoggerSize);
    logger_initialize(Logger);

    run_part1();
    run_part2();

    return 0;
}

// -------------------------------------------------------------------
// Source Code from ther files

#include "chibi_core.c" 
#include "darray.c"
#include "platform_unix.c"