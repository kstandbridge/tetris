#pragma once

/*
* NOTE(kstandbridge):
* 
   * TETRIS_INTERNAL:
 *  0 - Build for public release
 *  1 - Build for developer only
 *
 * TETRIS_SLOW:
 *   0 - No slow code allowed
 *   1 - Slow code welcome
 */

//
// NOTE(kstandbridge): Compilers
//

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO(kstandbridge): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

//
// NOTE(kstandbridge): Types
//

#include <stdint.h>
#include <stddef.h>


typedef size_t memory_index;

typedef int8_t s8;
typedef uint8_t u8;

typedef int16_t s16;
typedef uint16_t u16;

typedef int32_t s32;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;

typedef int64_t s64;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

typedef int32_t b32;

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

#if TETRIS_SLOW
// TODO(kstandbridge): Complete assertion macro
#define Assert(Expression) \
if (!(Expression))     \
{                      \
*(int *)0 = 0;     \
}
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");
#define InvalidDefaultCase default: { Assert(!"InvalidDefaultCase"); }

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(kstandbridge): swap, min, max ... macros???

inline u32
SafeTruncateSizeUInt64(u64 Value)
{
    // TODO(kstandbridge): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return Result;
}


struct thread_context
{
    int Placeholder;
};

/*
 * NOTE(kstandbridge): Services that the platform layer provides to the game
 */

#if TETRIS_INTERNAL
/*
 * IMPORTANT(kstandbridge):
 * These are NOT for doing anything in the shipping game - they are
 * blocking and the write doesn't protect against lost data!
 */
struct debug_read_file_result
{
    u32 ContentsSize;
    void *Contents;
};

typedef void debug_platform_free_file_memory(thread_context *Thread, void *Memory);

typedef debug_read_file_result debug_platform_read_entire_file(thread_context *Thread, char *Filename);

typedef b32 debug_platform_write_entire_file(thread_context *Thread, char *Filename, u32 MemorySize, void *Memory);

typedef s64 get_time_stamp();

#endif

/*
 * NOTE(kstandbridge): Services that the game provides to the platform layer.
 * (this may expand in the future - sound a separate thread, etc.)
 */

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO(kstandbridge): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct game_offscreen_buffer
{
    // NOTE(kstandbridge): Pixels are always 32-bit wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    s16 *Samples;
};

struct game_button_state
{
    int HalfTransitionCount;
    b32 EndedDown;
};

struct game_controller_input
{
    b32 IsConnected;
    b32 IsAnalog;
    r32 StickAverageX;
    r32 StickAverageY;
    
    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;
            
            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;
            
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
            
            game_button_state Back;
            game_button_state Start;
            
            // NOTE(kstandbridge): All buttons must be added above this line
            
            game_button_state Terminator;
        };
    };
};

struct game_input
{
    game_button_state MouseButtons[5];
    s32 MouseX, MouseY, MouseZ;
    
    r32 dtForFrame;
    
    // TODO(kstandbridge): Insert clock value here.
    game_controller_input Controllers[5];
};

struct game_memory
{
    b32 IsInitialized;
    u64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(kstandbridge): REQUIRED to be cleared to zero at startup
    
    u64 TransientStorageSize;
    void *TransientStorage; // NOTE(kstandbridge): REQUIRED to be cleared to zero at startup
    
    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    get_time_stamp *GetTimeStamp;
};

typedef void game_update_and_render(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer);

// NOTE(kstandbridge): At the moment, this has to be a very fast function, it cannot be more than a millisecond or so.
// TODO(kstandbridge): Reduce the pressure on this function's performance by measuring it or asking about it, etc
typedef void game_get_sound_samples(thread_context *Thread, game_memory *Memory, game_sound_output_buffer *SoundBuffer);

inline game_controller_input *
GetController(game_input *Input, int ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return (Result);
}
