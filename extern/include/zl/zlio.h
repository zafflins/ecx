#pragma once

#include <zl/zlstd.h>

typedef struct FileBuffer {
    u64 size;
    char* data;
    char* cursor;
} FileBuffer;

ZL_API void WriteFile(u64 size, ptr data, FileBuffer* fb);
ZL_API void ReadFile(u64 size, ptr data, FileBuffer* fb);
ZL_API void SaveFile(char* path, FileBuffer* fb);

ZL_API FileBuffer CreateFile(u64 size);
ZL_API void CloseFile(FileBuffer* fb);
ZL_API FileBuffer LoadFile(char* path);
ZL_API u64 SumFileLines(FileBuffer* fb);

ZL_API u64 StrLen(char* str);
ZL_API u8 ChrInStr(char c, char* str);
ZL_API u8 StrInStr(char* sub, char* str);
ZL_API u8 StrIsStr(u64 len, char* a, char* b);
ZL_API u8 PrefixInStr(char* str, char* prefix);
