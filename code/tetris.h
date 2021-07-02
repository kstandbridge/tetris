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


#define TILES_X 12
#define CENTER_X 4ll;
#define TILES_Y 18
#define DROP_TIME 20
#define LINE_COUNT 4

enum board_type
{
    BoardType_Clear,
    BoardType_Wall,
    BoardType_Locked,
    BoardType_Line,
};

struct game_state
{
    s32 Score;
    s32 TotalLines;
    s32 X;
    s32 Y;
    s32 Piece;
    s32 Rotation;
    r32 DropCounter;
    s32 DropSpeed;
    
    s32 *Lines;
    s32 NextLine;
    
    board_type *Board;
    
};

global_variable char *Tetrominoes[] =
{
    "..0...0...0...0.",
    "..0..00...0.....",
    ".....00..00.....",
    "..0..00..0......",
    ".0...00...0.....",
    ".0...0...00.....",
    "..0...0..00.....",
};
