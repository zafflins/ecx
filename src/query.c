#include "_ecx.h"


u64 ECXCreateQuery(ECXQueryDesc desc, ECXScene* scene) {
    if (!scene) return ECX_INVALID;

    u64 qh = PoolAllocMemory(NULL, scene->queryPool);
    if(qh == ZL_INVALID_HANDLE) return ECX_INVALID;

    ECXQuery q = {0};
    FOR_I(0, desc.includes, 1) {
        ECXComp* c = GetPoolData(desc.includev[i], scene->compPool);
        if (c) FOR_J(0, 4, 1) q.include[j] |= c->ident[j];
    } FOR_I(0, desc.excludes, 1) {
        ECXComp* c = GetPoolData(desc.excludev[i], scene->compPool);
        if (c) FOR_J(0, 4, 1) q.exclude[j] |= c->ident[j];
    }

    // check for cached query
    FOR_I(0, ArrayUsed(scene->queries), 1) {
        ECXQuery* cq = GetPoolData(scene->queries[i], scene->queryPool);
        if (!cq) continue;

        u8 match = 1;
        FOR_J(0, ECX_IDENT_MAX, 1) {
            if (cq->include[j] != q.include[j]
            ||  cq->exclude[j] != q.exclude[j]) {
                match = 0;
                break;
            }
        } if (match) {
            PoolFreeMemory(qh, scene->queryPool);
            return scene->queries[i];
        }
    }


    // resolve archetypes
    u64* archv = CreateArray(2, sizeof(u64));
    if (!archv) {
        PoolFreeMemory(qh, scene->queryPool);
        return ECX_INVALID;
    }

    FOR_I(0, ArrayUsed(scene->archetypes), 1) {
        ECXArch* a = GetPoolData(scene->archetypes[i], scene->archPool);
        if (!a) continue;

        u8 match = 1;
        FOR_J(0, ECX_IDENT_MAX, 1) {
            if (a->ident[j] & q.exclude[j]
            || (a->ident[j] & q.include[j]) != q.include[j]) {
                match = 0;
                break;
            }
        } if (match) {
            if (ArrayFull(archv)) {
                archv = ResizeArray(ArraySlots(archv)*2, archv);
                if (ArrayFull(archv)) {
                    PoolFreeMemory(qh, scene->queryPool);
                    DestroyArray(archv);
                    return ECX_INVALID;
                }
            } PushArray(&scene->archetypes[i], archv);
        }
    }

    // if (!ArrayUsed(archv)) {
    //     printf("FOUND NO MATCHING ARCHETYPES FOR QUERY\n");
    //     PoolFreeMemory(qh, scene->queryPool);
    //     DestroyArray(archv);
    //     return ECX_INVALID;
    // }

    q.archv = archv;
    q.arches = ArrayUsed(archv);
    q.lid = ArrayUsed(scene->queries);
    q.id = GetPoolIndex(qh, scene->queryPool);

    SetPoolData(qh, &q, scene->queryPool);
    PushArray(&qh, scene->queries);
    return qh;
}

void ECXDestroyQuery(u64 query, ECXScene* scene) {
    if (!query || query == ECX_INVALID || !scene) return;

    ECXQuery* q = GetPoolData(query, scene->queryPool);
    if (!q) return;

    PoolFreeMemory(query, scene->queryPool);
    PopSwapArray(q->lid, scene->queries);
    DestroyArray(q->archv);
}

ECXQueryView ECXCreateQueryView(u64 query, ECXScene* scene) {
    ECXQuery* q = GetPoolData(query, scene->queryPool);
    if (!q) return (ECXQueryView){0};
    return (ECXQueryView) {
        .arches = q->arches,
        .archv = q->archv,
    };
}
