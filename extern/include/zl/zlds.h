#pragma once

#include <zl/zlstd.h>

ZL_API ptr CreateArray(u32 slots, u32 stride);
ZL_API ptr ResizeArray(u32 slots, ptr arr);
ZL_API void DestroyArray(ptr arr);
ZL_API void ClearArray(ptr arr);
ZL_API u8 ArrayFull(ptr arr);
ZL_API u32 ArrayUsed(ptr arr);
ZL_API u32 ArraySlots(ptr arr);
ZL_API ptr GetArray(u32 slot, ptr arr);
ZL_API void RemArray(u32 slot, ptr arr);
ZL_API void PopArray(ptr value, ptr arr);
ZL_API void PushArray(ptr value, ptr arr);
ZL_API ptr SliceArray(u32 src, u32 end, ptr arr);
ZL_API void SetArray(u32 slot, ptr value, ptr arr);
ZL_API void PullArray(u32 slot, ptr value, ptr arr);
ZL_API void PopSwapArray(u32 slot, ptr arr);


ZL_API ptr CreateMap(u32 slots, u32 stride);
ZL_API ptr ResizeMap(u32 slots, ptr map);
ZL_API void DestroyMap(ptr map);
ZL_API void SetMap(char* key, ptr value, ptr map);
ZL_API void RemMap(char* key, ptr map);
ZL_API ptr GetMap(char* key, ptr map);
ZL_API void IterMapValues(void (*fn)(ptr), ptr map);
ZL_API void IterMapKeys(void (*fn)(char*), ptr map);

ZL_API ptr CreatePool(u32 slots, u32 stride);
ZL_API u64 PoolAllocMemory(ptr data, ptr pool);
ZL_API void PoolFreeMemory(u64 handle, ptr pool);
ZL_API void DestroyPool(ptr pool);

ZL_API void SetPoolData(u64 handle, ptr data, ptr pool);
ZL_API u32 GetPoolVersion(u64 handle, ptr pool);
ZL_API u32 GetPoolIndex(u64 handle, ptr pool);
ZL_API ptr GetPoolData(u64 handle, ptr pool);
