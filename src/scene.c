#include "_ecx.h"

ECXScene ECXCreateScene(u32 capacity) {
    ECXScene s = {0};
    if (!capacity) return s;

    s.entities = CreateArray(capacity, sizeof(u64));
    s.systems = CreateArray(ECX_SYS_MAX, sizeof(u64));
    s.queries = CreateArray(ECX_QUERY_MAX, sizeof(u64));
    s.components = CreateArray(ECX_COMP_MAX, sizeof(u64));
    s.archetypes = CreateArray(ECX_ARCH_MAX, sizeof(u64));
    if (!s.systems
    ||  !s.queries
    ||  !s.entities
    ||  !s.components
    ||  !s.archetypes) {
        printf("FAILED TO CREATE SCENE: ARRAY ALLOC FAILED\n");
        ECXDestroyScene(&s);
    }

    s.entPool = CreatePool(capacity, sizeof(ECXEnt));
    s.sysPool = CreatePool(ECX_SYS_MAX, sizeof(ECXSys));
    s.compPool =CreatePool(ECX_COMP_MAX, sizeof(ECXComp));
    s.archPool = CreatePool(ECX_ARCH_MAX, sizeof(ECXArch));
    s.queryPool = CreatePool(ECX_QUERY_MAX, sizeof(ECXQuery));
    if (!s.entPool
    ||  !s.sysPool
    ||  !s.compPool
    ||  !s.archPool
    ||  !s.queryPool) {
        printf("FAILED TO CREATE SCENE: POOL ALLOC FAILED\n");
        ECXDestroyScene(&s);
    }

    s.cap = capacity;
    return s;
}

void ECXDestroyScene(ECXScene* scene) {
    if (!scene) return;

    FOR_I(0, ArrayUsed(scene->components), 1) {
        ECXDestroyComponent(scene->components[i], scene);
    } FOR_I(0, ArrayUsed(scene->archetypes), 1) {
        ECXDestroyArchetype(scene->archetypes[i], scene);
    } FOR_I(0, ArrayUsed(scene->queries), 1) {
        ECXDestroyQuery(scene->queries[i], scene);
    }

    DestroyArray(scene->systems);
    DestroyArray(scene->queries);
    DestroyArray(scene->entities);
    DestroyArray(scene->components);
    DestroyArray(scene->archetypes);

    DestroyPool(scene->entPool);
    DestroyPool(scene->sysPool);
    DestroyPool(scene->compPool);
    DestroyPool(scene->archPool);
    DestroyPool(scene->queryPool);

    *scene = (ECXScene){0};
}
