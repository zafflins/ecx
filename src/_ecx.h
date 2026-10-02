#pragma once

#include <ecx/ecx.h>

#define ECX_INVALID                  0xFFFFFFFF
#define ECX_SCENE_MAX                255
#define ECX_FIELD_MAX                255
#define ECX_QUERY_MAX                1024
#define ECX_SYS_MAX                  1024
#define ECX_ENT_MAX                  5000000
#define ECX_IDENT_MAX                4
#define ECX_COMP_MAX                 64 * ECX_IDENT_MAX
#define ECX_ARCH_MAX                 64 * ECX_IDENT_MAX

typedef struct ECXEnt {
    u32 id;
    u32 lid;
    u32 comps;

    u64 arch;
    u64 handle;
    u64 ident[4];
} ECXEnt;

typedef struct ECXSys {
    u32 id;
    u32 lid;
    ptr user;
    u64 query;
    void (*init)(ptr, const ECXArchView*, const ECXScene*);
    void (*main)(ptr, const ECXArchView*, const ECXScene*);
    void (*exit)(ptr, const ECXArchView*, const ECXScene*);
} ECXSys;

typedef struct ECXComp {
    u32 id;
    u32 lid;
    u32 slots;
    u32 fields;
    u64* strides;
    u64 ident[4];
} ECXComp;

typedef struct ECXArch {
    u32 id;
    u32 lid;
    u32 cap;
    u32 ents;
    u32 comps;
    u32 fields;

    u64 dsize;
    u64 tsize;
    u64 ident[4];

    ptr src;
    ptr data;
    u32* smap;
    u32* emap;
    u32* cmap;
    u64* compv;
    u64* offsets;
    u64* strides;
} ECXArch;

typedef struct ECXQuery {
    u32 id;
    u32 lid;
    u32 arches;
    u64* archv;
    u64 include[4];
    u64 exclude[4];
} ECXQuery;


u8 ECXEntInComp(ECXEnt* e, ECXComp* c) {
    FOR_I(0, ECX_IDENT_MAX, 1) {
        if ((e->ident[i] & c->ident[i]) != c->ident[i])
            return 0;
    } return 1;
} u8 ECXCompInArch(ECXArch* a, ECXComp* c) {
    FOR_I(0, ECX_IDENT_MAX, 1) {
        if ((a->ident[i] & c->ident[i]) != c->ident[i])
            return 0;
    } return 1;
} u8 ECXArchInQuery(ECXArch* a, ECXQuery* q) {
    FOR_I(0, ECX_IDENT_MAX, 1) {
        if ((a->ident[i] & q->include[i]) != q->include[i]
        || a->ident[i] & q->exclude[i]) return 0;
    } return 1;
}

u64 ECXCreateArchetype(u32 cap, u32 comps, u64* compv, ECXScene* scene);
void ECXDestroyArchetype(u64 arch, ECXScene* scene);

void ECXMigrateArchetype(u64 ent, ECXScene* scene);
ECXArchView ECXCreateArchView(u64 arch, ECXScene* scene);
void ECXBindArchetype(u64 ent, u64 arch, ECXScene* scene);
void ECXUnbindArchetype(u64 ent, u64 arch, ECXScene* scene);
