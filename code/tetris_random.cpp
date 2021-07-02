global_variable u32 GlobalRandomState;

inline u32
Random_u32()
{
    u32 Result = GlobalRandomState;
    
    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    GlobalRandomState = Result;
    
    return Result;
}

inline b32
Random_b32()
{
    return Random_u32() % 2;
}

inline b32
RandomChance_b32(s32 Chance)
{
    return Random_u32() % Chance == 2;
}

inline s32
RandomRange_s32(s32 Min, s32 Max) // NOTE(kstandbridge): Inclusive
{
    s32 Range = Max - Min + 1;
    s32 Result = Random_u32() % Range;
    Result += Min;
    return Result;
}