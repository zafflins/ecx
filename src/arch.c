#include "_ecx.h"

static u64 ECXArchTemp[255];

u64 ECXCreateArchetype(u32 cap, u32 comps, u64* compv, ECXScene* scene) {
    if (!cap || !comps || !compv || !scene) return ZL_INVALID_HANDLE;

    u64 ah = PoolAllocMemory(NULL, scene->archPool);
    if (ah == ZL_INVALID_HANDLE) return ZL_INVALID_HANDLE;

    // compute comp data size / n fields
    u64 dsize = 0;
    u64 fields = 0;
    ECXArch a = {0};
    FOR_I(0, comps, 1) {
        ECXComp* c = GetPoolData(compv[i], scene->compPool);
        if (!c) {
            PoolFreeMemory(ah, scene->archPool);
            return ZL_INVALID_HANDLE;
        }

        FOR_J(0, c->fields, 1) {
            dsize += cap * c->strides[j];
        } FOR_K(0, ECX_IDENT_MAX, 1) {
            a.ident[k] |= c->ident[k];
        } fields += c->fields;
    }

    // compute total arch metadata size
    u64 msize =
        (sizeof(u32) * cap) +
        (sizeof(u32) * scene->cap) +
        (sizeof(u32) * ECX_COMP_MAX) +
        (sizeof(u64) * comps) +
        (sizeof(u64) * fields) +
        (sizeof(u64) * fields);

    a.dsize = dsize;
    a.tsize = msize + dsize;
    a.src = AllocMemory(a.tsize);
    if (!a.src) {
        PoolFreeMemory(ah, scene->archPool);
        return ZL_INVALID_HANDLE;
    }

    a.data = (u32*)((u8*)a.src);
    a.smap = (u32*)((u8*)a.data + dsize);
    a.emap = (u32*)((u8*)a.smap + sizeof(u32) * cap);
    a.cmap = (u32*)((u8*)a.emap + sizeof(u32) * scene->cap);
    a.compv = (u64*)((u8*)a.cmap + sizeof(u32) * ECX_COMP_MAX);
    a.offsets = (u64*)((u8*)a.compv + sizeof(u64) * comps);
    a.strides = (u64*)((u8*)a.offsets + sizeof(u64) * fields);

    u32 base = 0;
    u32 offset = 0;
    FOR_I(0, comps, 1) {
        ECXComp* c = GetPoolData(compv[i], scene->compPool);

        a.cmap[c->id] = base; // comp base into a.offsets
        a.compv[i] = compv[i];
        FOR_J(0, c->fields, 1) {
            a.offsets[base] = offset; // comp per-field offset into a.data
            a.strides[base] = c->strides[j];

            offset += cap * c->strides[j];
            base++;
        }
    }

    a.cap = cap;
    a.comps = comps;
    a.fields = fields;
    a.lid = ArrayUsed(scene->archetypes);
    a.id = GetPoolIndex(ah,scene->archPool);

    SetPoolData(ah, &a, scene->archPool);
    PushArray(&ah, scene->archetypes);

    // update queries
    FOR_I(0, ArrayUsed(scene->queries), 1) {
        ECXQuery* q = GetPoolData(scene->queries[i], scene->queryPool);
        if (ECXArchInQuery(&a, q)) {
            PushArray(&ah, q->archv);
            q->arches++;
        }
    }

    return ah;
}

void ECXDestroyArchetype(u64 arch, ECXScene* scene) {
    if (!arch || arch == ZL_INVALID_HANDLE || !scene) return;

    ECXArch* a = GetPoolData(arch, scene->archPool);
    if (!a) return;

    // update queries
    FOR_I(0, ArrayUsed(scene->queries), 1) {
        ECXQuery* q = GetPoolData(scene->queries[i], scene->queryPool);
        if (!q) continue;

        if (ECXArchInQuery(a, q)) {
            FOR_J(0, q->arches, 1) {
                if (q->archv[j] == arch) {
                    PopSwapArray(j, q->archv);
                    q->arches--;
                }
            }
        }
    }

    PopSwapArray(a->lid, scene->archetypes);
    PoolFreeMemory(arch, scene->archPool);
    FreeMemory(a->src);
}


void ECXMigrateArchetype(u64 ent, ECXScene* scene) {
    ECXEnt* e = GetPoolData(ent, scene->entPool);
    if (!e) return;

    u64 oarch = e->arch;
    if (e->comps == 0 && oarch != ZL_INVALID_HANDLE) {
        ECXUnbindArchetype(ent, oarch, scene);
        return;
    }

    // resolve existing arch if possible
    u64 arch = ZL_INVALID_HANDLE;
    FOR_I(0, ArrayUsed(scene->archetypes), 1) {
        ECXArch* a = GetPoolData(scene->archetypes[i], scene->archPool);
        if (!a) continue;

        u8 match = 1;
        FOR_J(0, ECX_IDENT_MAX, 1) {
            if (e->ident[j] != a->ident[j]) {
                match = 0;
                break;
            }
        } if (match) {
            arch = scene->archetypes[i];
            break;
        }
    }

    // create new arch if none match the new identity
    if (arch == ZL_INVALID_HANDLE) {
        u32 comps = 0;
        FOR_I(0, ArrayUsed(scene->components), 1) {
            ECXComp* c = GetPoolData(scene->components[i], scene->compPool);
            if (!c) continue;

            u8 match = 1;
            FOR_J(0, ECX_IDENT_MAX, 1) {
                if ((e->ident[j] & c->ident[j]) != c->ident[j]) {
                    match = 0;
                    break;
                }
            } if (match) ECXArchTemp[comps++] = scene->components[i];
        }

        arch = ECXCreateArchetype(scene->cap, comps, ECXArchTemp, scene);
        SetMemory(sizeof(u64) * comps, 0, ECXArchTemp);

        if (arch == ZL_INVALID_HANDLE) return;
    } if (oarch == arch) return;

    if (oarch != ZL_INVALID_HANDLE) {
        ECXArch* oa = GetPoolData(oarch, scene->archPool);
        ECXArch* na = GetPoolData(arch, scene->archPool);
        if (!oa || !na) return;

        // resolve common comps and migrate data
        u32 nslot = na->ents;
        u32 oslot = oa->emap[e->id];
        FOR_I(0, oa->comps, 1) {
            ECXComp* c = GetPoolData(oa->compv[i], scene->compPool);
            if (!c || !ECXCompInArch(na, c)) continue;

            u32 obase = oa->cmap[c->id];
            u32 nbase = na->cmap[c->id];
            FOR_K(0, c->fields, 1) {
                ptr src = (u8*)oa->data + oa->offsets[obase + k] + oslot * oa->strides[obase + k];
                ptr dst = (u8*)na->data + na->offsets[nbase + k] + nslot * na->strides[nbase + k];
                WriteMemory(oa->strides[obase + k], src, dst);
            }
        } ECXUnbindArchetype(ent, oarch, scene);
    } ECXBindArchetype(ent, arch, scene);
}

void ECXBindArchetype(u64 ent, u64 arch, ECXScene* scene) {
    if (!ent || !arch || ent == ZL_INVALID_HANDLE || arch == ZL_INVALID_HANDLE || !scene) return;

    ECXArch* a = GetPoolData(arch, scene->archPool);
    if (a->ents >= a->cap) return;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    e->comps = a->comps;
    e->arch = arch;

    a->smap[a->ents] = e->id;
    a->emap[e->id] = a->ents++;
}

void ECXUnbindArchetype(u64 ent, u64 arch, ECXScene* scene) {
    if (!ent || !arch || ent == ZL_INVALID_HANDLE || arch == ZL_INVALID_HANDLE || !scene) return;

    ECXArch* a = GetPoolData(arch, scene->archPool);
    ECXEnt* e = GetPoolData(ent, scene->entPool);
    if (!a || !e || !a->ents) return;

    u32 did = e->id;
    u32 lslot = a->ents - 1;
    u32 dslot = a->emap[did];
    if (dslot == ECX_INVALID) return;

    // pop swap field data
    FOR_I(0, a->fields, 1) {
        u64 stride = a->strides[i];
        ptr base = (u8*)a->data + a->offsets[i];

        ptr dead = (u8*)base + dslot * stride;
        ptr last = (u8*)base + lslot * stride;

        WriteMemory(stride, last, dead);
    }

    u32 moved = a->smap[lslot];
    a->smap[dslot] = moved;
    a->emap[moved] = dslot;
    a->ents--;

    e->arch = ZL_INVALID_HANDLE;
    a->emap[did] = ECX_INVALID;
    a->smap[lslot] = ECX_INVALID;
}

ECXArchView ECXCreateArchView(u64 arch, ECXScene* scene) {
    ECXArchView v = {0};
    if (!arch || arch == ZL_INVALID_HANDLE || !scene) return v;

    ECXArch* a = GetPoolData(arch, scene->archPool);
    if (!a) return v;

    v.handle = arch;
    v.ents = a->ents;
    v.comps = a->comps;
    v.fields = a->fields;

    v.data = a->data;
    v.cmap = a->cmap;
    v.offsets = a->offsets;
    v.strides = a->strides;

    return v;
}

ptr ECXGetArchViewField(u64 comp, u64 field, const ECXArchView* view, const ECXScene* scene) {
    if (!comp || comp == ECX_INVALID ||!view || !scene) return NULL;

    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!c || field >= c->fields) return NULL;

    return (u8*)view->data + view->offsets[view->cmap[c->id] + field];
}
