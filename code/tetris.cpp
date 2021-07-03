#include "tetris.h"

#include "tetris_random.cpp"
#include "tetris_rendering.cpp"

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

internal s32
GetRotateOffset(s32 X, s32 Y, s32 R)
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
            s32 PieceIndex = GetRotateOffset(X, Y, Rotation);
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

internal s32
GetRandomTetromino()
{
    s32 Result = RandomRange_s32(0, ArrayCount(Tetrominoes) - 1);
    return Result;
}

internal void
DrawTetromino(game_offscreen_buffer *Buffer, v2 P, r32 TileSize, r32 TilePadding, s32 Piece, s32 Rotation)
{
    v2 TileHalfSize = V2(TileSize/2, TileSize/2);
    r32 TotalSize = TileSize + TilePadding;
    
    r32 OriginalX = P.X;
    for(s32 Y = 0; Y < 4; ++Y)
    {
        for(s32 X = 0; X < 4; ++X)
        {
            s32 Index = GetRotateOffset(X, Y, Rotation);
            
            char *At = Tetrominoes[Piece] + Index;
            if(*At == '0')
            {
                DrawRectangle(Buffer, P, TileHalfSize, Color(0.0f, 0.0f, 0.5f));
            }
            
            P.X += TotalSize;
        }
        P.Y += TotalSize;
        P.X = OriginalX;
    }
}

internal void
ChangeGameLevel(game_state *GameState, game_level GameLevel)
{
    switch(GameLevel)
    {
        case GameLevel_Menu:
        {
            game_level_menu_state *LevelState = &GameState->LevelState.Menu;
            
            LevelState->TypeA = true;
            LevelState->TypeB = false;
        } break;
        
        case GameLevel_TypeA:
        {
            game_level_type_a_state *LevelState = &GameState->LevelState.TypeA;
            GameState->NextLine = 0;
            
            LevelState->Score = 0;
            LevelState->TotalLines = 0;
            LevelState->X = CENTER_X;
            LevelState->Y = 1;
            LevelState->Rotation = 0;
            LevelState->DropCounter = 0;
            LevelState->DropSpeed = 10;
            LevelState->Piece = GetRandomTetromino();
            LevelState->NextPiece = GetRandomTetromino();
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
            GameState->GameOver = false;
        } break;
        
        InvalidDefaultCase;
    }
    
    GameState->Level = GameLevel;
}

internal void
UpdateGameMenu(game_state *GameState, game_input *Input, game_offscreen_buffer *Buffer)
{
    game_level_menu_state *LevelState = &GameState->LevelState.Menu;
    
    for (s32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        
        if(Controller->MoveLeft.EndedDown)
        {
            LevelState->TypeA = true;
            LevelState->TypeB = false;
        }
        if(Controller->MoveRight.EndedDown)
        {
            LevelState->TypeA = false;
            LevelState->TypeB = true;
        }
        
        if(Controller->Start.EndedDown && Controller->Start.HalfTransitionCount == 1)
        {
            if(LevelState->TypeA)
            {
                ChangeGameLevel(GameState, GameLevel_TypeA);
            }
            else if(LevelState->TypeB)
            {
                ChangeGameLevel(GameState, GameLevel_TypeB);
            }
        }
    }
    
    // NOTE(kstandbridge): Clear screen
    DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width/2, Buffer->Height/2), Color(0.5f, 0.0f, 0.5f));
    
    color BorderColor = Color(1, 1, 1);
    color SelectedBorderColor = Color(1, 1, 0);
    color BackgroundColor = Color(0, 0, 0);
    color TextColor = Color(1, 1, 1);
    color SelectTextColor = Color(1, 1, 0);
    
    DrawRectangle(Buffer, V2(0, -22), V2(36.0f, 10.0f), BorderColor);
    DrawRectangle(Buffer, V2(0, -22), V2(34.0f, 8.0f), BackgroundColor);
    DrawString(Buffer, "GAME TYPE", V2(0, -20), 0.4, TextAlign_Center, TextColor);
    
    DrawRectangle(Buffer, V2(-40, 13), V2(24.0f, 8.0f), (LevelState->TypeA) ? SelectedBorderColor : BorderColor);
    DrawRectangle(Buffer, V2(-40, 13), V2(22.0f, 6.0f), BackgroundColor);
    DrawString(Buffer, "TYPE A", V2(-40, 15), 0.4, TextAlign_Center, (LevelState->TypeA) ? SelectTextColor : TextColor);
    
    DrawRectangle(Buffer, V2(40, 13), V2(24.0f, 8.0f), (LevelState->TypeB) ? SelectedBorderColor : BorderColor);
    DrawRectangle(Buffer, V2(40, 13), V2(22.0f, 6.0f), BackgroundColor);
    DrawString(Buffer, "TYPE B", V2( 40, 15), 0.4, TextAlign_Center, (LevelState->TypeB) ? SelectTextColor : TextColor);
}

internal void
UpdateGameOver(game_state *GameState, game_input *Input)
{
    b32 Exit = false;
    
    for (s32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        
        if(Controller->Start.EndedDown && Controller->Start.HalfTransitionCount == 1)
        {
            Exit = true;
        }
    }
    
    if(Exit)
    {
        ChangeGameLevel(GameState, GameLevel_Menu);
    }
}

internal void
UpdateGame(game_state *GameState, game_input *Input)
{
    game_level_type_a_state *LevelState = &GameState->LevelState.TypeA;
    s32 MoveX = 0;
    s32 MoveY = 0;
    s32 Rotation = 0;
    
    for (s32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        
        if (Controller->IsAnalog)
        {
            // TODO(kstandbridge): Analog movement
            //ddP = v2{ Controller->StickAverageX, -Controller->StickAverageY };
        }
        else
        {
            // NOTE(kstandbridge): Use digital movement tuning
            
            if(Controller->MoveDown.EndedDown)
            {
                MoveY = 1;
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
        
        if(Controller->ActionDown.EndedDown && Controller->ActionDown.HalfTransitionCount == 1)
        {
            Rotation = 1;
        }
        
    }
    
    if(ValidMove(GameState->Board, LevelState->Piece, LevelState->Rotation + Rotation, LevelState->X + MoveX, LevelState->Y))
    {
        LevelState->Rotation += Rotation;
        LevelState->X += MoveX;
    }
    else
    {
        if(Rotation)
        {
            // NOTE(kstandbridge): Move 2 spaces automatically on rotates next to wall/object
            for(MoveX = -2; MoveX < 3; ++MoveX)
            {
                if(ValidMove(GameState->Board, LevelState->Piece, LevelState->Rotation + Rotation, LevelState->X + MoveX, LevelState->Y))
                {
                    LevelState->Rotation += Rotation;
                    LevelState->X += MoveX;
                    break;
                }
            }
        }
        else
        {
            // TODO(kstandbridge): Invalid move sound
        }
    }
    
    LevelState->DropCounter -= MoveY*10;
    LevelState->DropCounter -= LevelState->DropSpeed * Input->dtForFrame;
    if(LevelState->DropCounter < 0)
    {
        // NOTE(kstandbridge): Clear any lines
        s32 LineScore = 0;
        for(s32 Index = 0; Index < LINE_COUNT; ++Index)
        {
            if(GameState->Lines[Index] != 0)
            {
                ++LineScore;
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
        if(LineScore)
        {
            LevelState->Score += (1 << LineScore) * 100;
        }
        LevelState->TotalLines += LineScore;
        LevelState->DropSpeed += LineScore;
        LevelState->DropCounter = DROP_TIME;
        if(ValidMove(GameState->Board, LevelState->Piece, LevelState->Rotation, LevelState->X, LevelState->Y + 1))
        {
            LevelState->Y++;
        }
        else
        {
            LevelState->Score += 25;
            
            // NOTE(kstandbridge): Lock the piece onto the board
            for(s32 Y = 0; Y < 4; ++Y)
            {
                for(s32 X = 0; X < 4; ++X)
                {
                    if(Tetrominoes[LevelState->Piece][GetRotateOffset(X, Y, LevelState->Rotation)] == '0')
                    {
                        GameState->Board[(LevelState->Y + Y) * TILES_X + (LevelState->X + X)] = BoardType_Locked;
                    }
                }
            }
            
            // NOTE(kstandbridge): Check for line
            GameState->NextLine = 0;
            for(s32 Y = 0; Y < 4; ++Y)
            {
                if(LevelState->Y + Y < TILES_Y - 1)
                {
                    b32 Line = true;
                    for(s32 X = 1; X < TILES_X - 1; ++X)
                    {
                        Line &= (GameState->Board[(LevelState->Y + Y) * TILES_X + X]) != 0;
                    }
                    
                    if(Line)
                    {
                        for(s32 X = 1; X < TILES_X - 1; ++X)
                        {
                            GameState->Board[(LevelState->Y + Y) * TILES_X + X] = BoardType_Line;
                        }
                        LevelState->DropCounter = DROP_TIME;
                        GameState->Lines[GameState->NextLine++] = LevelState->Y + Y;
                        if(GameState->NextLine >= LINE_COUNT) GameState->NextLine = 0;
                    }
                }
            }
            
            LevelState->X = CENTER_X;
            LevelState->Y = 1;
            LevelState->Rotation = 0;
            LevelState->Piece = LevelState->NextPiece;
            LevelState->NextPiece = GetRandomTetromino();
            LevelState->DropCounter = DROP_TIME;
            // NOTE(kstandbridge): Newly placed piece is invalid move thus game over
            if(!ValidMove(GameState->Board, LevelState->Piece, LevelState->Rotation, LevelState->X, LevelState->Y))
            {
                // TODO(kstandbridge): Game over screen
                GameState->GameOver = true;
            }
        }
    }
}

internal void
RenderGame(game_state *GameState, game_offscreen_buffer *Buffer)
{
    game_level_type_a_state *LevelState = &GameState->LevelState.TypeA;
    
    // NOTE(kstandbridge): Clear screen
    DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width/2, Buffer->Height/2), Color(0.5f, 0.0f, 0.5f));
    
    // NOTE(kstandbridge): Draw game board
    v2 P = V2(0, 0);
    r32 TileSize = 5.0f;
    r32 TilePadding = 0.3f;
    v2 TileHalfSize = V2(TileSize/2, TileSize/2);
    r32 BlockOffset = TileSize + TilePadding;
    
    r32 TotalSize = (TileSize) + TilePadding;
    {
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
                        DrawRectangle(Buffer, P, TileHalfSize, Color(1.0f, 1.0f, 1.0f));
                    } break;
                    
                    case BoardType_Wall:
                    {
                        DrawRectangle(Buffer, P, TileHalfSize, Color(0.0f, 0.0f, 0.0f));
                    } break;
                    
                    case BoardType_Locked:
                    {
                        DrawRectangle(Buffer, P, TileHalfSize, Color(0.5f, 0.5f, 0.5f));
                    } break;
                    
                    case BoardType_Line:
                    {
                        DrawRectangle(Buffer, P, TileHalfSize, Color(0.0f, 1.0f, 0.0f));
                    } break;
                    
                    InvalidDefaultCase;
                }
                
                P.X += BlockOffset;
            }
            P.Y += BlockOffset;
            P.X = OriginalX;
            
        }
    }
    
    P = V2(0, 0);
    P.X -= (TotalSize)*(TILES_X - 1)/2;
    P.Y -= (TotalSize)*(TILES_Y - 1)/2;
    P.X += TotalSize*LevelState->X;
    P.Y += TotalSize*LevelState->Y;
    
    // NOTE(kstandbridge): Draw current piece
    DrawTetromino(Buffer, P, TileSize, TilePadding, LevelState->Piece, LevelState->Rotation);
    
    // NOTE(kstandbridge): Draw HUD
    {
        DrawRectangle(Buffer, V2(56, -17), V2(20, 20), Color(0, 0, 0));
        char NumberBuffer[16];
        P = V2(40, -30);
        DrawString(Buffer, "SCORE", P, 0.4, TextAlign_Left, Color(1, 1, 1));
        P.Y += 10.0f;
        _snprintf_s(NumberBuffer, sizeof(NumberBuffer), "%06d", LevelState->Score);
        DrawString(Buffer, NumberBuffer, P, 0.4, TextAlign_Left, Color(1, 1, 1));
        
        DrawRectangle(Buffer, V2(52, 22), V2(15, 17), Color(0, 0, 0));
        P.Y += 10.0f;
        DrawString(Buffer, "LINES", P, 0.4, TextAlign_Left, Color(1, 1, 1));
        P.Y += 10.0f;
        _snprintf_s(NumberBuffer, sizeof(NumberBuffer), "%03d", LevelState->TotalLines);
        DrawString(Buffer, NumberBuffer, P, 0.4, TextAlign_Left, Color(1, 1, 1));
        P.Y += 12.0f;
        DrawString(Buffer, "NEXT", P, 0.4, TextAlign_Left, Color(1, 1, 1));
        DrawTetromino(Buffer, V2(40, 20), TileSize, TilePadding, LevelState->NextPiece, 0);
        
        P = V2(-80, -40);
        _snprintf_s(NumberBuffer, sizeof(NumberBuffer), "COUNTER %02.f", LevelState->DropCounter);
        DrawString(Buffer, NumberBuffer, P, 0.2, TextAlign_Left, Color(1, 1, 1));
        P.Y += 5.0f;
        _snprintf_s(NumberBuffer, sizeof(NumberBuffer), "SPEED %d", LevelState->DropSpeed);
        DrawString(Buffer, NumberBuffer, P, 0.2, TextAlign_Left, Color(1, 1, 1));
        
        
    }
}

void
GameUpdateAndRender(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           ArrayCount(Input->Controllers[0].Buttons));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    if(!GlobalRandomState)
    {
        GlobalRandomState = (u32)Memory->GetTimeStamp();
    }
    
    //////////////////////
    // NOTE(kstandbridge): Initalize game
    
    if (!Memory->IsInitialized)
    {
        Memory->IsInitialized = true;
        
        memory_arena MemoryArena;
        InitializeArena(&MemoryArena, 
                        Memory->PermanentStorageSize - sizeof(game_state),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        GameState->Lines = PushArray(&MemoryArena, LINE_COUNT, s32);
        GameState->Board = PushArray(&MemoryArena, TILES_Y*TILES_X, board_type);
        ChangeGameLevel(GameState, GameLevel_Menu);
        
    }
    
    
    switch(GameState->Level)
    {
        case GameLevel_Menu:
        {
            UpdateGameMenu(GameState, Input, Buffer);
            
        } break;
        
        case GameLevel_TypeA:
        {
            if(GameState->GameOver)
            {
                UpdateGameOver(GameState, Input);
                
                RenderGame(GameState, Buffer);
                
                DrawRectangle(Buffer, V2(0, 0), V2(31.7f, 6.0f), Color(0, 0, 0));
                DrawString(Buffer, "GAME OVER", V2(0, 2), 0.4, TextAlign_Center, Color(1, 1, 1));
            }
            else
            {
                UpdateGame(GameState, Input);
                
                RenderGame(GameState, Buffer);
            }
            
        } break;
    }
    
}