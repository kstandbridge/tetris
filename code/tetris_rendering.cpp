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

#include "tetris_letters.h"

inline s32
GetLetterIndex(char C)
{
    s32 Result;
    
    if (C >= '0' && C <= '9')
        //if (C > 47 && C < 58)
    {
        Result = C - 48 + 26;
    }
    else if (C == '.')
    {
        Result = 'Z'-'A'+11;
    }
    else if (C == '/')
    {
        Result = 'Z'-'A'+12;
    }
    else
    {
        Result = C-'A';
    }
    
    return Result;
}

enum text_align
{
    TextAlign_Left,
    TextAlign_Right,
    TextAlign_Center,
};

// NOTE(kstandbridge): This is not 100%
internal r32
GetWordAlignOffset(s32 text_align, char *Word, r32 Size) 
{
    Assert(*Word);
    if (text_align == TextAlign_Left) return 0.f;
    
    r32 Result = 0.f;
    r32 BlockOffsetX = Size*1.6f*2.f; // NOTE(kstandbridge): Copy'n'Paste
    
    for (char *At = Word; *At && *At != '\\'; At++) 
    {
        if (*At == ' ')
        {
            Result += BlockOffsetX*4 + Size*2.f;
            continue;
        }
        
        s32 LetterIndex = GetLetterIndex(*At);
        Result += BlockOffsetX*LetterSpacings[LetterIndex] + Size*2.f;
    }
    
    Result -=  Size*2.f;
    
    if (text_align == TextAlign_Right) return -Result;
    else if (text_align == TextAlign_Center) return -Result*.5f;
    
    InvalidCodePath;
    return 0.f;
}

internal void
DrawString(game_offscreen_buffer *Buffer, char *String, v2 P, r32 Size, text_align TextAlign, r32 R, r32 G, r32 B) 
{
    r32 Firstx = P.X;
    P.X += GetWordAlignOffset(TextAlign, String, Size);
    
    r32 OriginalX = P.X;
    r32 OriginalY = P.Y;
    v2 HalfSize = {Size*1.6f, Size};
    r32 BlockOffsetX = Size*1.6f*2.f; // NOTE(kstandbridge): Copy'n'Paste
    
    
    for (char *LetterAt = String; *LetterAt; LetterAt++) 
    {
        if (*LetterAt == ' ')
        {
            P.X += BlockOffsetX*4 + Size*2.f;
            OriginalX = P.X;
            continue;
        } 
        else if (*LetterAt == '\\') 
        {
            P.X = Firstx;
            P.X += GetWordAlignOffset(TextAlign, LetterAt+1, Size);
            OriginalX = P.X;
            P.Y -= Size*(TEXT_HEIGHT+2)*2.5f;
            OriginalY = P.Y;
            continue;
        }
        
        s32 LetterIndex = GetLetterIndex(*LetterAt);
        char **Letter = &Letters[LetterIndex][0];
        
        for (s32 Index = TEXT_HEIGHT; Index > 0; --Index) 
        {
            char *At = Letter[Index - 1];
            while(*At) 
            {
                if (*At++ != ' ') 
                {
                    DrawRectangle(Buffer, P, HalfSize, R, G, B);
                }
                P.X += BlockOffsetX;
            }
            P.Y -= Size*2.f;
            if (Index != 0) P.X = OriginalX;
        }
        
        P.X = OriginalX;
        P.Y = OriginalY;
        P.X += BlockOffsetX*LetterSpacings[LetterIndex] + Size*2.f;
        OriginalX = P.X;
    }
}

// TODO(kstandbridge): Remove stdio
#include <stdio.h>

internal void
DrawNumber(game_offscreen_buffer *Buffer, u32 Number, v2 P, r32 Size, text_align TextAlign, r32 R, r32 G, r32 B) 
{
    char NumberBuffer[16];
    _snprintf_s(NumberBuffer, sizeof(NumberBuffer), "%06d", Number);
    DrawString(Buffer, NumberBuffer, P, Size, TextAlign, R, G, B);
}