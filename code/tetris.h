#pragma once

#include "tetris_platform.h"

struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
void*
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return Result;
}

#include "tetris_math.h"
#include "tetris_intrinsics.h"


struct game_state
{
    v2 P;
    s32 Piece;
};

char *Tetrominoes[][4] =
{
    
    "..0.",
    "..0.",
    "..0.",
    "..0.",
    
    "..0.",
    ".00.",
    "..0.",
    "....",
    
    "....",
    ".00.",
    ".00.",
    "....",
    
    "..0.",
    ".00.",
    ".0..",
    "....",
    
    ".0..",
    ".00.",
    "..0.",
    "....",
    
    ".0..",
    ".0..",
    ".00.",
    "....",
    
    "..0.",
    "..0.",
    ".00.",
    "....",
    
};
