#pragma once

#include <zl/zldefines.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    u64 offset;
    u64 marker;
    u64 total;
    ptr data;
} Arena;

typedef struct { f32 x,y; } Vec2;
typedef struct { f32 x,y,z; } Vec3;
typedef struct { f32 x,y,z,w; } Vec4;
typedef struct { f32 x,y,w,h; } Rect;
typedef struct { f32 m[3][3]; } Mat3;
typedef struct { f32 m[4][4]; } Mat4;

#define PrintVec2(v) printf("Vec3: %0.4f, %0.4f\n", v.x, v.y)
#define PrintVec3(v) printf("Vec3: %0.4f, %0.4f, %0.4f\n", v.x, v.y, v.z)
#define PrintVec4(v) printf("Vec3: %0.4f, %0.4f, %0.4f, %0.4f\n", v.x, v.y, v.z, v.w)

typedef struct { i32 x,y; } Vec2i;
typedef struct { i32 x,y,z; } Vec3i;
typedef struct { i32 x,y,z,w; } Vec4i;
typedef struct { i32 x,y,w,h; } Recti;
typedef struct { i32 m[3][3]; } Mat3i;
typedef struct { i32 m[4][4]; } Mat4i;

#define Vec3ToVec2(v) (Vec2){v.x, v.y}
#define Vec4ToVec3(v) (Vec3){v.x, v.y, v.z}
#define Vec2ToVec3(v, z) (Vec3){v.x, v.y, z}
#define Vec3ToVec4(v, w) (Vec4){v.x, v.y, v.z, w}
#define Vec2ToVec4(v, z, w) (Vec4){v.x, v.y, z, w}

ZL_API ptr AllocMemory(u64 size);
ZL_API Result FreeMemory(ptr memory);
ZL_API ptr ReallocMemory(u64 size, ptr memory);

ZL_API Result SetMemory(u64 size, i32 value, ptr memory);
ZL_API Result ReadMemory(u64 size, ptr source, ptr dest);
ZL_API Result MoveMemory(u64 size, ptr source, ptr dest);
ZL_API Result WriteMemory(u64 size, ptr source, ptr dest);
ZL_API Result CompareMemory(u64 size, ptr mem1, ptr mem2);

ZL_API Arena CreateArena(u64 size);
ZL_API void MarkArena(Arena* arena);
ZL_API void UnmarkArena(Arena* arena);
ZL_API Result DestroyArena(Arena* arena);
ZL_API void ResetArena(u8 marker, Arena* arena);
ZL_API ptr ArenaAllocMemory(u64 size, Arena* arena);

ZL_API f32 CrossVec2(Vec2 a, Vec2 b);
ZL_API Vec2 AddVec2(Vec2 a, Vec2 b);
ZL_API Vec2 SubVec2(Vec2 a, Vec2 b);
ZL_API Vec2 MulVec2(Vec2 a, f32 s);
ZL_API Vec2 DivVec2(Vec2 a, f32 s);
ZL_API f32 DotVec2(Vec2 a, Vec2 b);
ZL_API Vec2 NormVec2(Vec2 v);
ZL_API f32 LenSqVec2(Vec2 v);
ZL_API f32 LenVec2(Vec2 v);
ZL_API Vec2 RotVec2(f32 a, Vec2 v);

ZL_API Vec3 CrossVec3(Vec3 a, Vec3 b);
ZL_API Vec3 AddVec3(Vec3 a, Vec3 b);
ZL_API Vec3 SubVec3(Vec3 a, Vec3 b);
ZL_API Vec3 MulVec3(Vec3 a, f32 s);
ZL_API Vec3 DivVec3(Vec3 a, f32 s);
ZL_API f32 DotVec3(Vec3 a, Vec3 b);
ZL_API Vec3 NormVec3(Vec3 v);
ZL_API f32 LenSqVec3(Vec3 v);
ZL_API f32 LenVec3(Vec3 v);
ZL_API Vec3 RotVec3x(f32 a, Vec3 v);
ZL_API Vec3 RotVec3y(f32 a, Vec3 v);
ZL_API Vec3 RotVec3z(f32 a, Vec3 v);
ZL_API void Vec3Basis(Vec3* rot, Vec3** basis);

ZL_API Vec4 AddVec4(Vec4 a, Vec4 b);
ZL_API Vec4 SubVec4(Vec4 a, Vec4 b);
ZL_API Vec4 MulVec4(Vec4 a, f32 s);
ZL_API Vec4 DivVec4(Vec4 a, f32 s);
ZL_API f32 DotVec4(Vec4 a, Vec4 b);
ZL_API Vec4 NormVec4(Vec4 v);
ZL_API f32 LenSqVec4(Vec4 v);
ZL_API f32 LenVec4(Vec4 v);

ZL_API Vec3 MulMat3Vec3(Vec3 v, Mat3 m);
ZL_API Vec4 MulMat4Vec4(Vec4 v, Mat4 m);
