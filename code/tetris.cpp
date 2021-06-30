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


internal void
DrawRectangle(game_offscreen_buffer *Buffer, v2 vMin, v2 vMax, r32 R, r32 G, r32 B)
{
    s32 MinX = RoundReal32ToInt32(vMin.X);
    s32 MinY = RoundReal32ToInt32(vMin.Y);
    s32 MaxX = RoundReal32ToInt32(vMax.X);
    s32 MaxY = RoundReal32ToInt32(vMax.Y);
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    // BIT PATTERN: 0x AA RR GG BB
    u32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                 (RoundReal32ToUInt32(G * 255.0f) << 8) |
                 (RoundReal32ToUInt32(B * 255.0f) << 0));
    
    u8 *Row = ((u8 *)Buffer->Memory +
               MinX * Buffer->BytesPerPixel +
               MinY * Buffer->Pitch);
    
    for (int Y = MinY; 
         Y < MaxY; 
         ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for (int X = MinX; 
             X < MaxX; 
             ++X)
        {
            *Pixel++ = Color;
        }
        
        Row += Buffer->Pitch;
    }
}
void
GameUpdateAndRender(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           ArrayCount(Input->Controllers[0].Buttons));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    //////////////////////
    // NOTE(kstandbridge): Initalize game
    {    
        if (!Memory->IsInitialized)
        {
            Memory->IsInitialized = true;
            GameState->P = V2(100, 100);
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
            
            if(Controller->ActionUp.EndedDown)
            {
                //ControllingEntity.High->dZ = 3.0f;
            }
            
            GameState->P += ddP;
        }
    }
    
    //////////////////////
    // NOTE(kstandbridge): Render
    {
        DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width, Buffer->Height), 128, 0, 128);
        
        DrawRectangle(Buffer, GameState->P, GameState->P + V2(100, 100), 64, 0, 64);
    }
    
}
