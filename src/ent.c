#include "_ecx.h"

u64 ECXCreateEntity(ECXScene* scene) {
    if (!scene) return ZL_INVALID_HANDLE;

    u64 eh = PoolAllocMemory(NULL, scene->entPool);
    if (eh == ZL_INVALID_HANDLE) return ZL_INVALID_HANDLE;

    ECXEnt e = {0};
    e.handle = eh;
    e.arch = ZL_INVALID_HANDLE;
    e.lid = ArrayUsed(scene->entities);
    e.id = GetPoolIndex(eh, scene->entPool);

    SetPoolData(eh, &e, scene->entPool);
    PushArray(&eh, scene->entities);
    return eh;
}

void ECXDestroyEntity(u64 ent, ECXScene* scene) {
    if (!ent || ent == ZL_INVALID_HANDLE) return;

    ECXEnt* e = GetPoolData(ent, scene->entPool);
    if (!e) return;

    PopSwapArray(e->lid, scene->entities);
    PoolFreeMemory(ent, scene->entPool);
}
