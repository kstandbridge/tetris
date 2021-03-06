#pragma once
union v2
{
    struct
    {
        r32 X, Y;
    };
    
    r32 E[2];
};

// TODO(kstandbridge): Consider v2 A = v2{5, 3}; ?
inline v2
V2(r32 X, r32 Y)
{
    v2 Result;
    
    Result.X = X;
    Result.Y = Y;
    
    return Result;
}

inline v2
V2(s32 X, s32 Y)
{
    v2 Result;
    
    Result.X = (r32)X;
    Result.Y = (r32)Y;
    
    return Result;
}


inline v2 
operator*(r32 A, v2 B)
{
    v2 Result;
    
    Result.X = A*B.X;
    Result.Y = A*B.Y;
    
    return Result;
}

inline v2 
operator*(v2 B, r32 A)
{
    v2 Result = A * B;
    
    return Result;
}

inline v2 &
operator*=(v2 &B, r32 A)
{
    B = A * B;
    
    return B;
}

inline v2
operator-(v2 A)
{
    v2 Result;
    
    Result.X = -A.X;
    Result.Y = -A.Y;
    
    
    return Result;
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    
    return Result;
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return A;
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    
    return Result;
}

inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;
    
    return A;
}

inline r32
Square(r32 A)
{
    r32 Result = A * A;
    
    return Result;
}

inline r32
Inner(v2 A, v2 B)
{
    r32 Result = A.X*B.X + A.Y*B.Y;
    
    return Result;
}

inline r32
LengthSq(v2 A)
{
    r32 Result = Inner(A, A);
    
    return Result;
}

struct rectangle2
{
    v2 Min;
    v2 Max;
};

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return Result;
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return Result;
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return Result;
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    
    return Result;
}

inline b32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    b32 Result = ((Test.X >= Rectangle.Min.X) &&
                  (Test.Y >= Rectangle.Min.Y) &&
                  (Test.X < Rectangle.Max.X) &&
                  (Test.Y < Rectangle.Max.Y));
    
    return Result;
}

inline s32
Clamp(s32 Min, s32 Value, s32 Max)
{
    s32 Result;
    
    if(Value < Min) Result = Min;
    else if(Value > Max) Result = Max;
    else Result = Value;
    
    return Result;
}