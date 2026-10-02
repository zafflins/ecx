#pragma once

#include <zl/zl.h>

#define ECX_INVALID_HANDLE UINT64_MAX

typedef struct ECXCompDesc {
    u32 fields;
    u64* strides;
} ECXCompDesc;

typedef struct ECXCompView {
    u32 fields;
    u64 handle;
    ptr* fieldv;
} ECXCompView;

typedef struct ECXQueryDesc {
    u32 includes;
    u32 excludes;
    u64* includev;
    u64* excludev;
} ECXQueryDesc;

typedef struct ECXQueryView {
    u32 arches;
    u64* archv;
} ECXQueryView;

typedef struct ECXArchView {
    u32 ents;
    u32 comps;
    u32 fields;
    u64 handle;

    ptr data;
    u32* cmap;
    u64* offsets;
    u64* strides;
} ECXArchView;

typedef struct ECXScene {
    u32 cap;
    u32 mask;

    ptr entPool;
    ptr sysPool;
    ptr compPool;
    ptr archPool;
    ptr queryPool;

    u64* systems;
    u64* queries;
    u64* entities;
    u64* components;
    u64* archetypes;
} ECXScene;

typedef struct ECXBinScene {
    struct {
        u64 size;
        u32 magic;
        u32 version;

        u32 cap;
        u32 ents;
        u32 comps;
        u32 arches;
    } header;
    struct {
        void* entData;
        void* compData;
        void* archData;
    } data;
} ECXBinScene;

typedef struct ECXSysDesc {
    ptr user;
    u64 query;
    void (*init)(ptr, const ECXArchView*, const ECXScene*);
    void (*main)(ptr, const ECXArchView*, const ECXScene*);
    void (*exit)(ptr, const ECXArchView*, const ECXScene*);
} ECXSysDesc;

ZL_API ECXScene ECXCreateScene(u32 capacity);
ZL_API void ECXDestroyScene(ECXScene* scene);

ZL_API ECXBinScene ECXCompileScene(char* path, ECXScene* scene);
ZL_API ECXBinScene ECXDecompileScene(char* path, ECXScene* scene);
ZL_API void ECXDestroyCompiledScene(ECXBinScene* scene);

ZL_API u64 ECXCreateEntity(ECXScene* scene);
ZL_API void ECXDestroyEntity(u64 ent, ECXScene* scene);

ZL_API u64 ECXCreateComponent(ECXCompDesc desc, ECXScene* scene);
ZL_API void ECXDestroyComponent(u64 comp, ECXScene* scene);

ZL_API u64 ECXCreateQuery(ECXQueryDesc desc, ECXScene* scene);
ZL_API void ECXDestroyQuery(u64 query, ECXScene* scene);
ZL_API ECXQueryView ECXCreateQueryView(u64 query, ECXScene* scene);

ZL_API u64 ECXCreateSystem(ECXSysDesc desc, ECXScene* scene);
ZL_API void ECXDestroySystem(u64 sys, ECXScene* scene);
ZL_API void ECXRunSystem(u64 sys, ECXScene* scene);

ZL_API void ECXProjectComponent(u64 ent, u64 comp, ptr proj, ECXScene* scene);
ZL_API ECXCompView ECXCreateComponentView(u64 ent, u64 comp, ECXScene* scene);
ZL_API void  ECXDestroyComponentView(ECXCompView* view);

ZL_API void ECXBindComponent(u64 ent, u64 comp, ECXScene* scene);
ZL_API void ECXUnbindComponent(u64 ent, u64 comp, ECXScene* scene);
ZL_API ptr ECXGetComponent(u64 ent, u64 comp, u64 field, ECXScene* scene);
ZL_API void ECXSetComponent(u64 ent, u64 comp, u64 field, ptr val, ECXScene* scene);

ZL_API ptr ECXGetArchViewField(u64 comp, u64 field, const ECXArchView* view, const ECXScene* scene);
