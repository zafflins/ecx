#include "_ecx.h"

typedef struct ECXBinEnt {
    u32 comps;
    u64 ident[4];
} ECXBinEnt;

typedef struct ECXBinComp {
    u32 fields;
    u64* strides;
    u64 ident[4];
} ECXBinComp;

typedef struct ECXBinArch {
    u32 cap;
    u32 ents;
    u32 comps;
    u32 fields;

    u64 size;
    ptr smap;
    ptr data;
    u64 ident[4];
} ECXBinArch;

#define ECX_VERSION 1
#define ECX_MAGIC (('Z' << 24) | ('X' << 16) | ('V' << 8) | ECX_VERSION)


static void ECXStoreScene(char* path, ECXBinScene* scene, FileBuffer* fb) {
    ECXBinEnt* entData = scene->data.entData;
    ECXBinComp* compData = scene->data.compData;
    ECXBinArch* archData = scene->data.archData;

    WriteFile(sizeof(scene->header), &scene->header, fb);

    FOR_I(0, scene->header.ents, 1) {
        WriteFile(sizeof(ECXBinEnt), &entData[i], fb);
    }

    FOR_I(0, scene->header.comps, 1) {
        struct {
            u32 fields;
            u64 ident[4];
        } record = {
            .fields = compData[i].fields
        };
        WriteMemory(sizeof(record.ident), compData[i].ident, record.ident);

        WriteFile(sizeof(record), &record, fb);
        WriteFile(sizeof(u64) *  record.fields, compData[i].strides, fb);
    }

    FOR_I(0, scene->header.arches, 1) {
        ECXBinArch* a = &archData[i];
        struct {
            u32 cap;
            u32 ents;
            u32 comps;
            u32 fields;

            u64 size;
            u64 ident[4];
        } record = {
            .cap = a->cap,
            .ents = a->ents,
            .size = a->size,
            .comps = a->comps,
            .fields = a->fields,
        };

        WriteMemory(sizeof(record.ident), a->ident, record.ident);

        WriteFile(sizeof(record), &record, fb);
        WriteFile(a->size, a->data, fb);
        WriteFile(sizeof(u32) * a->cap, a->smap, fb);
    }

    SaveFile(path, fb);
}

static void ECXRestoreScene(ECXBinScene* bscene, ECXScene* scene) {
    if (bscene->header.magic != ECX_MAGIC || bscene->header.version != ECX_VERSION) return;

    *scene = ECXCreateScene(bscene->header.cap);
    if (!scene->cap) return;

    u64* emap = AllocMemory(sizeof(u64) * bscene->header.ents);
    u64* cmap = AllocMemory(sizeof(u64) * bscene->header.comps);
    if (!emap || !cmap) {
        FreeMemory(emap);
        FreeMemory(cmap);
        return;
    }

    ECXBinEnt* entData = bscene->data.entData;
    ECXBinComp* compData = bscene->data.compData;
    ECXBinArch* archData = bscene->data.archData;

    FOR_I(0, bscene->header.ents, 1) {
        ECXBinEnt* be = &entData[i];
        u64 eh = ECXCreateEntity(scene);
        ECXEnt* e = GetPoolData(eh, scene->entPool);

        WriteMemory(sizeof(e->ident), be->ident, e->ident);
        e->comps = be->comps;
        emap[e->id] = eh;
    }

    FOR_I(0, bscene->header.comps, 1) {
        ECXBinComp* bc = &compData[i];
        u64 ch = ECXCreateComponent((ECXCompDesc){
            .fields = bc->fields,
            .strides = bc->strides
        }, scene);
        ECXComp* c = GetPoolData(ch, scene->compPool);

        WriteMemory(sizeof(c->ident), bc->ident, c->ident);
        cmap[c->id] = ch;
    }

    u64 comps[ECX_COMP_MAX];
    FOR_I(0, bscene->header.arches, 1) {
        ECXBinArch* ba = &archData[i];

        u32 n = 0;
        FOR_J(0, bscene->header.comps, 1) {
            u64 ch = cmap[j];
            ECXComp* c = GetPoolData(ch, scene->compPool);

            u8 match = 1;
            FOR_K(0, ECX_IDENT_MAX, 1) {
                if ((ba->ident[k] & c->ident[k]) != c->ident[k]) {
                    match = 0;
                    break;
                }
            } if (match) comps[n++] = ch;
        }

        u64 ah = ECXCreateArchetype(ba->cap, ba->comps, comps, scene);
        ECXArch* a = GetPoolData(ah, scene->archPool);

        WriteMemory(ba->size, ba->data, a->data);
        WriteMemory(ba->cap * sizeof(u32), ba->smap, a->smap);

        a->ents = ba->ents;
        FOR_U(0, a->ents, 1) {
            u32 ei = a->smap[u];
            u64 eh = emap[ei];

            ECXEnt* e = GetPoolData(eh, scene->entPool);

            e->arch = ah;
            a->emap[ei] = u;
        }
    }

    FreeMemory(emap);
    FreeMemory(cmap);
}


ECXBinScene ECXCompileScene(char* path, ECXScene* scene) {
    ECXBinScene s = {0};
    if (!scene
    ||  !scene->entPool
    ||  !scene->sysPool
    ||  !scene->compPool
    ||  !scene->archPool
    ||  !scene->queryPool
    ||  !scene->systems
    ||  !scene->queries
    ||  !scene->entities
    ||  !scene->components
    ||  !scene->archetypes) return s;

    u64 size = sizeof(((ECXBinScene*)0)->header);

    u32 ents = ArrayUsed(scene->entities);
    u32 comps = ArrayUsed(scene->components);
    u32 arches = ArrayUsed(scene->archetypes);

    ECXBinEnt* entData = AllocMemory(ents * sizeof(ECXBinEnt));
    ECXBinComp* compData = AllocMemory(comps * sizeof(ECXBinComp));
    ECXBinArch* archData = AllocMemory(arches * sizeof(ECXBinArch));
    if (!entData || !compData || !archData) {
        ECXDestroyCompiledScene(&s);
        return (ECXBinScene){0};
    }

    size += ents * sizeof(ECXBinEnt);
    size += comps * sizeof(ECXBinComp);
    size += arches * sizeof(ECXBinArch);

    FOR_I(0, arches, 1) {
        ECXArch* a = GetPoolData(scene->archetypes[i], scene->archPool);
        if (!a) continue;

        u64 asize = a->dsize + sizeof(u32) * a->cap;
        ptr data = AllocMemory(asize);
        if (!data) {
            ECXDestroyCompiledScene(&s);
            return (ECXBinScene){0};
        } size += asize;

        ECXBinArch ba = {0};
        ba.cap = a->cap;
        ba.ents = a->ents;
        ba.size = a->dsize;
        ba.comps = a->comps;
        ba.fields = a->fields;

        ba.data = data;
        ba.smap = (u8*)data + a->dsize;

        WriteMemory(a->dsize, a->data, ba.data);
        WriteMemory(sizeof(ba.ident), a->ident, ba.ident);
        WriteMemory(a->cap * sizeof(u32), a->smap, ba.smap);

        archData[i] = ba;
    }

    FOR_I(0, comps, 1) {
        ECXComp* c = GetPoolData(scene->components[i], scene->compPool);
        if (!c) continue;

        u64 csize = c->fields * sizeof(u64);
        ptr data = AllocMemory(csize);
        if (!data) {
            ECXDestroyCompiledScene(&s);
            return (ECXBinScene){0};
        } size += csize;

        ECXBinComp bc = {0};
        bc.strides = data;
        bc.fields = c->fields;

        WriteMemory(csize, c->strides, data);
        WriteMemory(sizeof(bc.ident), c->ident, bc.ident);

        compData[i] = bc;
    }

    FOR_I(0, ents, 1) {
        ECXEnt* e = GetPoolData(scene->entities[i], scene->entPool);
        if (!e) continue;

        ECXBinEnt be = {0};
        be.comps = e->comps;

        WriteMemory(sizeof(be.ident), e->ident, be.ident);

        entData[i] = be;
    }

    s.header.magic = ECX_MAGIC;
    s.header.version = ECX_VERSION;

    s.header.size = size;
    s.header.ents = ents;
    s.header.comps = comps;
    s.header.arches = arches;
    s.header.cap = scene->cap;

    s.data.entData = entData;
    s.data.compData = compData;
    s.data.archData = archData;

    if (path) {
        FileBuffer fb = CreateFile(size);
        if (fb.data) {
            ECXStoreScene(path, &s, &fb);
            CloseFile(&fb);
        }
    }

    return s;
}

ECXBinScene ECXDecompileScene(char* path, ECXScene* scene) {
    if (!path || !scene) return (ECXBinScene){0};

    FileBuffer fb = LoadFile(path);
    if (!fb.data) return (ECXBinScene){0};

    ECXBinScene bs = {0};
    ReadFile(sizeof(bs.header), &bs.header, &fb);

    if (bs.header.magic != ECX_MAGIC || bs.header.version != ECX_VERSION) {
        CloseFile(&fb);
        return (ECXBinScene){0};
    }

    ECXBinEnt* entData = AllocMemory(bs.header.ents * sizeof(ECXBinEnt));
    ECXBinComp* compData = AllocMemory(bs.header.comps * sizeof(ECXBinComp));
    ECXBinArch* archData = AllocMemory(bs.header.arches * sizeof(ECXBinArch));
    if (!entData || !compData || !archData) {
        CloseFile(&fb);
        return (ECXBinScene){0};
    }

    FOR_I(0, bs.header.ents, 1) {
        ReadFile(sizeof(ECXBinEnt), &entData[i], &fb);
    }

    FOR_I(0, bs.header.comps, 1) {
        struct {
            u32 fields;
            u64 ident[4];
        } record = {0};
        ReadFile(sizeof(record), &record, &fb);

        u64* strides = AllocMemory(sizeof(u64) * record.fields);
        if (!strides) {
            ECXDestroyCompiledScene(&bs);
            return (ECXBinScene){0};
        }

        ReadFile(sizeof(u64) * record.fields, strides, &fb);

        ECXBinComp c = {0};
        c.strides = strides;
        c.fields = record.fields;
        WriteMemory(sizeof(record.ident), record.ident, c.ident);

        compData[i] = c;
    }

    FOR_I(0, bs.header.arches, 1) {
        struct {
            u32 cap;
            u32 ents;
            u32 comps;
            u32 fields;

            u64 size;
            u64 ident[4];
        } record = {0};
        ReadFile(sizeof(record), &record, &fb);

        ptr data = AllocMemory(record.size + record.cap * sizeof(u32));
        if (!data) {
            ECXDestroyCompiledScene(&bs);
            return (ECXBinScene){0};
        }

        ReadFile(record.size + record.cap * sizeof(u32), data, &fb);

        ECXBinArch a = {
            .data = data,
            .cap = record.cap,
            .ents = record.ents,
            .size = record.size,
            .comps = record.comps,
            .fields = record.fields
        };
        a.smap = (u8*)data + record.size;
        WriteMemory(sizeof(record.ident), record.ident, a.ident);

        archData[i] = a;
    }

    bs.data.entData = entData;
    bs.data.compData = compData;
    bs.data.archData = archData;

    ECXRestoreScene(&bs, scene);
    return bs;
}

void ECXDestroyCompiledScene(ECXBinScene* scene) {
    if (!scene) return;

    ECXBinEnt* entData = scene->data.entData;
    ECXBinComp* compData = scene->data.compData;
    ECXBinArch* archData = scene->data.archData;

    FOR_I(0, scene->header.arches, 1) {
        if (archData[i].data) FreeMemory(archData[i].data);
    } FOR_I(0, scene->header.comps, 1) {
        if (compData[i].strides) FreeMemory(compData[i].strides);
    }

    FreeMemory(entData);
    FreeMemory(compData);
    FreeMemory(archData);
    *scene = (ECXBinScene){0};
}
