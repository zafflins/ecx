#include "scene.c"
#include "query.c"
#include "arch.c"
#include "comp.c"
#include "bake.c"
#include "ent.c"

u64 ECXCreateSystem(ECXSysDesc desc, ECXScene* scene) {
    if (!desc.main || !desc.query || desc.query == ZL_INVALID_HANDLE || !scene) return ZL_INVALID_HANDLE;

    u64 sh = PoolAllocMemory(NULL, scene->sysPool);
    if (sh == ZL_INVALID_HANDLE) return ZL_INVALID_HANDLE;

    ECXSys s = {0};

    s.query = desc.query;
    s.user = desc.user;
    s.init = desc.init;
    s.main = desc.main;
    s.exit = desc.exit;
    s.lid = ArrayUsed(scene->systems);
    s.id = GetPoolIndex(sh, scene->sysPool);

    SetPoolData(sh, &s, scene->sysPool);
    PushArray(&sh, scene->systems);

    return sh;
}

void ECXDestroySystem(u64 sys, ECXScene* scene) {
    if (!sys || sys == ZL_INVALID_HANDLE || !scene) return;
    ECXSys* s = GetPoolData(sys, scene->sysPool);
    if (!s) return;

    PopSwapArray(s->lid, scene->systems);
    PoolFreeMemory(sys, scene->sysPool);
}

void ECXRunSystem(u64 sys, ECXScene* scene) {
    if (!sys || sys == ZL_INVALID_HANDLE || !scene) return;
    ECXSys* s = GetPoolData(sys, scene->sysPool);
    if (!s) return;

    ECXQuery* q = GetPoolData(s->query, scene->queryPool);
    if (!q) return;

    ECXQueryView qv = ECXCreateQueryView(s->query, scene);
    FOR_I(0, qv.arches, 1) {
        ECXArchView av = ECXCreateArchView(qv.archv[i], scene);
        if (s->init) s->init(s->user, &av, scene);
        s->main(s->user, &av, scene);
        if (s->exit) s->exit(s->user, &av, scene);
    }
}
