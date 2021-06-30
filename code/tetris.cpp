#include "tetris.h"

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    s16 ToneVolume = 1000;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;
    
    s16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        // TODO(kstandbridge): Draw this out for people
        
#if 0
        r32 SineValue = sinf(GameState->tSine);
        s16 SampleValue = (s16)(SineValue * ToneVolume);
#else
        s16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        
#if 0
        GameState->tSine += 2.0f * Pi32 * 1.0f / (r32)WavePeriod;
        if (GameState->tSine > 2.0f * Pi32)
        {
            GameState->tSine -= 2.0f * Pi32;
        }
#endif
    }
}

void 
GameGetSoundSamples(thread_context *Thread, game_memory *Memory, game_sound_output_buffer *SoundBuffer)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}

global_variable r32 GlobalScale = 0.01f; // NOTE(kstandbridge): Hard coded

internal void
DrawRectangle(game_offscreen_buffer *Buffer, v2 P, v2 HalfSize, r32 R, r32 G, r32 B)
{
    HalfSize *= (Buffer->Width/1.77f)*GlobalScale;
    P *= (Buffer->Width/1.77f)*GlobalScale;
    
    P += V2(Buffer->Width*0.5f,Buffer->Height*0.5f); 
    
    s32 MinX = Clamp(0, RoundReal32ToInt32(P.X - HalfSize.X), Buffer->Width);
    s32 MinY = Clamp(0, RoundReal32ToInt32(P.Y - HalfSize.Y), Buffer->Height);
    s32 MaxX = Clamp(MinX, RoundReal32ToInt32(P.X + HalfSize.X), Buffer->Width);
    s32 MaxY = Clamp(MinY, RoundReal32ToInt32(P.Y + HalfSize.Y), Buffer->Height);
    
    // BIT PATTERN: 0x AA RR GG BB
    u32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                 (RoundReal32ToUInt32(G * 255.0f) << 8) |
                 (RoundReal32ToUInt32(B * 255.0f) << 0));
    
    u8 *Row = ((u8 *)Buffer->Memory +
               MinX * Buffer->BytesPerPixel +
               MinY * Buffer->Pitch);
    
    for (s32 Y = MinY; Y < MaxY; ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for (s32 X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        
        Row += Buffer->Pitch;
    }
}

inline s32
Rotate(s32 X, s32 Y, s32 R)
{
    switch(R % 4)
    {
        case 0: return Y * 4 + X;        // 0   degrees
        case 1: return 12 + Y - (X * 4); // 90  degrees
        case 2: return 15 - (Y * 4) - X; // 180 degrees
        case 3: return 3 - Y + (X * 4);  // 270 degrees
    }
    return 0;
}

void
GameUpdateAndRender(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           ArrayCount(Input->Controllers[0].Buttons));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    memory_arena MemoryArena;
    InitializeArena(&MemoryArena, 
                    Memory->PermanentStorageSize - sizeof(game_state),
                    (u8 *)Memory->PermanentStorage + sizeof(game_state));
    
    //////////////////////
    // NOTE(kstandbridge): Initalize game
    {    
        if (!Memory->IsInitialized)
        {
            Memory->IsInitialized = true;
            GameState->P = V2(0, 0);
            GameState->Board = PushArray(&MemoryArena, TILES_Y*TILES_X, s32);
        }
    }
    
    
    //////////////////////
    // NOTE(kstandbridge): Handle input
    {    
        for (s32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
        {
            game_controller_input *Controller = GetController(Input, ControllerIndex);
            
            //entity ControllingEntity = GetHighEntity(GameState, LowIndex);
            v2 ddP = {};
            
            if (Controller->IsAnalog)
            {
                // NOTE(kstandbridge): Use analog movement tuning
                ddP = v2{ Controller->StickAverageX, -Controller->StickAverageY };
            }
            else
            {
                // NOTE(kstandbridge): Use digital movement tuning
                
                if(Controller->MoveUp.EndedDown)
                {
                    ddP.Y = -1.0f;
                }
                if(Controller->MoveDown.EndedDown)
                {
                    ddP.Y = 1.0f;
                }
                if(Controller->MoveLeft.EndedDown)
                {
                    ddP.X = -1.0f;
                }
                if(Controller->MoveRight.EndedDown)
                {
                    ddP.X = 1.0f;
                }
            }
            
            if(Controller->ActionUp.EndedDown && Controller->ActionUp.HalfTransitionCount == 1)
            {
                GameState->Piece = (GameState->Piece + 1) % ArrayCount(Tetrominoes);
            }
            
            if(Controller->ActionDown.EndedDown && Controller->ActionDown.HalfTransitionCount == 1)
            {
                GameState->Rotation++;
            }
            
            r32 Speed = 100.0f;
            GameState->P += ddP*Input->dtForFrame*Speed;
        }
    }
    
    //////////////////////
    // NOTE(kstandbridge): Render
    {
        // NOTE(kstandbridge): Clear screen
        DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width/2, Buffer->Height/2), 0.5f, 0.0f, 0.5f);
        
        // NOTE(kstandbridge): Draw game board
        {
            
            v2 P = V2(0, 0);
            r32 TileSize = 5.0f;
            r32 TilePadding = 0.3f;
            v2 TileHalfSize = V2(TileSize/2, TileSize/2);
            r32 BlockOffset = TileSize + TilePadding;
            
            P.X -= ((TileSize) + TilePadding)*(TILES_X - 1)/2;
            P.Y -= ((TileSize) + TilePadding)*(TILES_Y - 1)/2;
            r32 OriginalX = P.X;
            
            for(s32 Y = 0; Y < TILES_Y; ++Y)
            {
                for(s32 X = 0; X < TILES_X; ++X)
                {
                    if(*(GameState->Board + (Y * TILES_X) + X) == 0)
                    {
                        DrawRectangle(Buffer, P, TileHalfSize, 1.0f, 1.0f, 1.0f);
                    }
                    P.X += BlockOffset;
                }
                P.Y += BlockOffset;
                P.X = OriginalX;
                
            }
        }
        
#if 0        
        // NOTE(kstandbridge): Draw tetromino
        r32 HalfSize = 2.5f;
        v2 InputP = GameState->P;
        s32 Piece = GameState->Piece;
        {
            v2 P = InputP - V2(HalfSize, HalfSize)*3;
            r32 OriginalX = P.X;
            r32 OriginalY = P.Y;
            r32 BlockOffset = HalfSize*2.0f + 0.2f;
            
            for(s32 Y = 0; Y < 4; ++Y)
            {
                for(s32 X = 0; X < 4; ++X)
                {
                    s32 Index = Rotate(X, Y, GameState->Rotation);
                    
                    char *At = Tetrominoes[Piece] + Index;
                    if(*At == '0')
                    {
                        DrawRectangle(Buffer, P, V2(HalfSize, HalfSize), 0.0f, 0.0f, 0.0f);
                    }
                    else
                    {
                        DrawRectangle(Buffer, P, V2(HalfSize, HalfSize), 1.0f, 1.0f, 1.0f);
                    }
                    P.X += BlockOffset;
                }
                P.X = OriginalX;
                P.Y += BlockOffset;
            }
        }
#endif
        
    }
}