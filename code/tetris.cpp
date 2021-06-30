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

internal s32
RotateTetromino(s32 X, s32 Y, s32 R)
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

internal b32
ValidMove(board_type *Board, s32 Tetromino, s32 Rotation, s32 TestX, s32 TestY)
{
    for(s32 X = 0; X < 4; ++X)
    {
        for(s32 Y = 0; Y < 4; ++Y)
        {
            s32 PieceIndex = RotateTetromino(X, Y, Rotation);
            s32 FieldIndex = (TestY + Y) * TILES_X + (TestX + X);
            
            if(TestX + X >= 0 && TestX + X < TILES_X)
            {
                if(TestY + Y >= 0 && TestY + Y < TILES_Y)
                {
                    if(Tetrominoes[Tetromino][PieceIndex] == '0' && Board[FieldIndex] != 0)
                    {
                        // NOTE(kstandbridge): Fail on first hit
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
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
            GameState->X = TILES_X/2;
            GameState->Y = 1;
            GameState->Rotation = 0;
            GameState->Piece = 0;
            GameState->Lines = PushArray(&MemoryArena, LINE_COUNT, s32);
            GameState->NextLine = 0;
            GameState->Board = PushArray(&MemoryArena, TILES_Y*TILES_X, board_type);
            for(s32 Y = 0; Y < TILES_Y; ++Y)
            {
                for(s32 X = 0; X < TILES_X; ++X)
                {
                    if(X == 0 || X == TILES_X - 1 || Y == TILES_Y - 1)
                    {
                        GameState->Board[Y * TILES_X + X] = BoardType_Wall;
                    }
                    else
                    {
                        GameState->Board[Y * TILES_X + X] = BoardType_Clear;
                    }
                }
            }
        }
    }
    
    
    //////////////////////
    // NOTE(kstandbridge): Handle input
    s32 MoveX = 0;
    s32 MoveY = 0;
    s32 Rotation = 0;
    
    {    
        for (s32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
        {
            game_controller_input *Controller = GetController(Input, ControllerIndex);
            
            //entity ControllingEntity = GetHighEntity(GameState, LowIndex);
            
            if (Controller->IsAnalog)
            {
                // NOTE(kstandbridge): Use analog movement tuning
                // TODO(kstandbridge): Analog movement
                //ddP = v2{ Controller->StickAverageX, -Controller->StickAverageY };
            }
            else
            {
                // NOTE(kstandbridge): Use digital movement tuning
                
                if(Controller->MoveUp.EndedDown && Controller->MoveUp.HalfTransitionCount == 1)
                {
                    MoveY = -1;
                }
                if(Controller->MoveDown.EndedDown && Controller->MoveDown.HalfTransitionCount == 1)
                {
                    MoveY = +1;
                }
                if(Controller->MoveLeft.EndedDown && Controller->MoveLeft.HalfTransitionCount == 1)
                {
                    MoveX = -1;
                }
                if(Controller->MoveRight.EndedDown && Controller->MoveRight.HalfTransitionCount == 1)
                {
                    MoveX = 1;
                }
            }
            
            if(Controller->ActionUp.EndedDown && Controller->ActionUp.HalfTransitionCount == 1)
            {
                GameState->Piece = (GameState->Piece + 1) % ArrayCount(Tetrominoes);
            }
            
            if(Controller->ActionDown.EndedDown && Controller->ActionDown.HalfTransitionCount == 1)
            {
                Rotation = 1;
            }
            
        }
    }
    
    if(ValidMove(GameState->Board, GameState->Piece, GameState->Rotation + Rotation, GameState->X + MoveX, GameState->Y + MoveY))
    {
        GameState->Rotation += Rotation;
        GameState->X += MoveX;
        GameState->Y += MoveY;
    }
    else
    {
        // TODO(kstandbridge): Invalid move sound
    }
    GameState->DropCounter++;
    if(GameState->DropCounter > DROP_TIME)
    {
        // NOTE(kstandbridge): Clear any lines
        for(s32 Index = 0; Index < LINE_COUNT; ++Index)
        {
            if(GameState->Lines[Index] != 0)
            {
                for(s32 X = 1; X < TILES_X - 1; ++X)
                {
                    for(s32 Y = GameState->Lines[Index]; Y > 0; Y--)
                    {
                        GameState->Board[Y * TILES_X + X] = GameState->Board[(Y - 1) * TILES_X + X];
                    }
                    GameState->Board[X] = BoardType_Clear;
                }
                
                GameState->Lines[Index] = 0;
            }
        }
        
        GameState->DropCounter = 0;
        if(ValidMove(GameState->Board, GameState->Piece, GameState->Rotation, GameState->X, GameState->Y + 1))
        {
            GameState->Y++;
        }
        else
        {
            // NOTE(kstandbridge): Lock the piece onto the board
            for(s32 Y = 0; Y < 4; ++Y)
            {
                for(s32 X = 0; X < 4; ++X)
                {
                    if(Tetrominoes[GameState->Piece][RotateTetromino(X, Y, GameState->Rotation)] == '0')
                    {
                        GameState->Board[(GameState->Y + Y) * TILES_X + (GameState->X + X)] = BoardType_Locked;
                    }
                }
            }
            
            // NOTE(kstandbridge): Check for line
            for(s32 Y = 0; Y < 4; ++Y)
            {
                if(GameState->Y + Y < TILES_Y - 1)
                {
                    b32 Line = true;
                    for(s32 X = 1; X < TILES_X - 1; ++X)
                    {
                        Line &= (GameState->Board[(GameState->Y + Y) * TILES_X + X]) != 0;
                    }
                    
                    if(Line)
                    {
                        for(s32 X = 1; X < TILES_X - 1; ++X)
                        {
                            GameState->Board[(GameState->Y + Y) * TILES_X + X] = BoardType_Line;
                        }
                        GameState->DropCounter = -DROP_TIME;
                        GameState->Lines[GameState->NextLine++] = GameState->Y + Y;
                        if(GameState->NextLine >= LINE_COUNT) GameState->NextLine = 0;
                    }
                }
            }
            
            GameState->X = TILES_X/2;
            GameState->Y = 1;
            GameState->Rotation = 0;
            GameState->Piece = 1;
            
            // NOTE(kstandbridge): Newly placed piece is invalid move thus game over
            if(!ValidMove(GameState->Board, GameState->Piece, GameState->Rotation, GameState->X, GameState->Y))
            {
                // TODO(kstandbridge): Game over screen
                Memory->IsInitialized = false;
            }
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
            
            r32 TotalSize = (TileSize) + TilePadding;
            P.X -= (TotalSize)*(TILES_X - 1)/2;
            P.Y -= (TotalSize)*(TILES_Y - 1)/2;
            r32 OriginalX = P.X;
            
            for(s32 Y = 0; Y < TILES_Y; ++Y)
            {
                for(s32 X = 0; X < TILES_X; ++X)
                {
                    board_type BoardType = *(GameState->Board + (Y * TILES_X) + X);
                    switch(BoardType)
                    {
                        
                        case BoardType_Clear:
                        {
                            DrawRectangle(Buffer, P, TileHalfSize, 1.0f, 1.0f, 1.0f);
                        } break;
                        
                        case BoardType_Wall:
                        {
                            DrawRectangle(Buffer, P, TileHalfSize, 0.0f, 0.0f, 0.0f);
                        } break;
                        
                        case BoardType_Locked:
                        {
                            DrawRectangle(Buffer, P, TileHalfSize, 0.5f, 0.5f, 0.5f);
                        } break;
                        
                        case BoardType_Line:
                        {
                            DrawRectangle(Buffer, P, TileHalfSize, 0.0f, 1.0f, 0.0f);
                        } break;
                        
                        InvalidDefaultCase;
                    }
                    
                    P.X += BlockOffset;
                }
                P.Y += BlockOffset;
                P.X = OriginalX;
                
            }
            
            
            // NOTE(kstandbridge): Draw current piece
            
            P = V2(0, 0);
            P.X -= (TotalSize)*(TILES_X - 1)/2;
            P.Y -= (TotalSize)*(TILES_Y - 1)/2;
            P.X += TotalSize*GameState->X;
            P.Y += TotalSize*GameState->Y;
            OriginalX = P.X;
            for(s32 Y = 0; Y < 4; ++Y)
            {
                for(s32 X = 0; X < 4; ++X)
                {
                    s32 Index = RotateTetromino(X, Y, GameState->Rotation);
                    
                    char *At = Tetrominoes[GameState->Piece] + Index;
                    if(*At == '0')
                    {
                        DrawRectangle(Buffer, P, TileHalfSize, 0.0f, 0.0f, 0.5f);
                    }
                    
                    P.X += TotalSize;
                }
                P.Y += TotalSize;
                P.X = OriginalX;
            }
        }
    }
}