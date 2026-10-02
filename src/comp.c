#include "_ecx.h"

u64 ECXCreateComponent(ECXCompDesc desc, ECXScene* scene) {
    if (!scene) return ZL_INVALID_HANDLE;

    u64 ch = PoolAllocMemory(NULL, scene->compPool);
    if (ch == ZL_INVALID_HANDLE) return ZL_INVALID_HANDLE;

    FOR(u8, f, 0, desc.fields, 1) {
        if (!desc.strides[f]) {
            PoolFreeMemory(ch, scene->compPool);
            return ZL_INVALID_HANDLE;
        }
    }

    ECXComp c = {0};
    c.strides = AllocMemory(sizeof(u64) * desc.fields);
    if (!c.strides) {
        PoolFreeMemory(ch, scene->compPool);
        return ZL_INVALID_HANDLE;
    } WriteMemory(sizeof(u64) * desc.fields, desc.strides, c.strides);

    c.fields = desc.fields;

    u32 id = GetPoolIndex(ch, scene->compPool);
    u32 chunk = id / 64;
    u32 bit = id % 64;

    c.id = id;
    c.ident[chunk] |= (1ULL << bit);
    c.lid = ArrayUsed(scene->components);

    SetPoolData(ch, &c, scene->compPool);
    PushArray(&ch, scene->components);
    return ch;
}

// TODO: defer deletion for entity/archetype cleanup
void ECXDestroyComponent(u64 comp, ECXScene* scene) {
    if (!comp || comp == ZL_INVALID_HANDLE || !scene) return;

    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!c) return;

    PopSwapArray(c->lid, scene->components);
    PoolFreeMemory(comp, scene->compPool);
    FreeMemory(c->strides);
}


void ECXBindComponent(u64 ent, u64 comp, ECXScene* scene) {
    if (!ent || ent == ZL_INVALID_HANDLE || !comp || comp == ZL_INVALID_HANDLE || !scene) return;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!e || !c) return;

    FOR_I(0, ECX_IDENT_MAX, 1) {
        e->ident[i] |= c->ident[i];
    } ECXMigrateArchetype(ent, scene);
}

ptr ECXGetComponent(u64 ent, u64 comp, u64 field, ECXScene* scene) {
    if (!ent || ent == ZL_INVALID_HANDLE || !comp || comp == ZL_INVALID_HANDLE || !scene) return NULL;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!e || !c) return NULL;

    ECXArch* a = GetPoolData(e->arch, scene->archPool);
    if (!a || !ECXCompInArch(a, c) || field >= c->fields) return NULL;

    u32 slot = a->smap[e->id];
    if (slot >= a->ents) return NULL;

    u32 base = a->cmap[c->id];
    return (u8*)a->data + a->offsets[base + field] + slot * a->strides[base + field];
}

void ECXSetComponent(u64 ent, u64 comp, u64 field, ptr val, ECXScene* scene) {
    if (!ent || ent == ZL_INVALID_HANDLE || !comp || comp == ZL_INVALID_HANDLE || !scene) return;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!e || !c) return;

    ECXArch* a = GetPoolData(e->arch, scene->archPool);
    if (!a || !ECXCompInArch(a, c) || field >= c->fields) return;

    u32 slot = a->smap[e->id];
    if (slot >= a->ents) return;

    u32 base = a->cmap[c->id];
    ptr dst = (u8*)a->data + a->offsets[base + field] + slot * a->strides[base + field];
    WriteMemory(a->strides[base + field], val, dst);
}

void ECXUnbindComponent(u64 ent, u64 comp, ECXScene* scene) {
    if (!ent || ent == ZL_INVALID_HANDLE || !comp || comp == ZL_INVALID_HANDLE || !scene) return;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!e || !c) return;
    FOR_I(0, ECX_IDENT_MAX, 1) {
        e->ident[i] &= ~c->ident[i];
    } ECXMigrateArchetype(ent, scene);
}


ECXCompView ECXCreateComponentView(u64 ent, u64 comp, ECXScene* scene) {
    ECXCompView v = {0};
    if (!ent || !comp || ent == ZL_INVALID_HANDLE || comp == ZL_INVALID_HANDLE || !scene) return v;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!e || !c) return v;

    ECXArch* a = GetPoolData(e->arch, scene->archPool);
    if (!a || !ECXCompInArch(a, c)) return v;

    ptr* fieldv = AllocMemory(sizeof(ptr) * c->fields);
    if (!fieldv) return v;

    u32 slot = a->smap[e->id];
    u32 base = a->cmap[c->id];
    FOR_I(0, c->fields, 1) {
        u32 offset = a->offsets[base + i];
        u32 stride = a->strides[base + i];
        fieldv[i] = (u8*)a->data + offset + slot * stride;
    }

    v.fields = c->fields;
    v.fieldv = fieldv;
    v.handle = comp;
    return v;
}

void ECXDestroyComponentView(ECXCompView* view) {
    FreeMemory(view->fieldv);
    *view = (ECXCompView){0};
}

void ECXProjectComponent(u64 ent, u64 comp, ptr proj, ECXScene* scene) {
    if (!ent || !comp || ent == ZL_INVALID_HANDLE || comp == ZL_INVALID_HANDLE || !proj || !scene) return;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    ECXComp* c = GetPoolData(comp, scene->compPool);
    if (!e || !c) return;

    ECXArch* a = GetPoolData(e->arch, scene->archPool);
    if (!a || !ECXCompInArch(a, c)) return;

    ptr* fields = (ptr*)proj;
    u32 slot = a->smap[e->id];
    u32 base = a->cmap[c->id];
    FOR_I(0, c->fields, 1) {
        u32 offset = a->offsets[base + i];
        u32 stride = a->strides[base + i];
        fields[i] = (u8*)a->data + offset + slot * stride;
    }
}
