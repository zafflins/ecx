# ECX API Guide

ECX is a single header C99 entity-component-system (ECS) library.

## Dependencies and Build

ECX is built with [`r3make`](https://github.com/r3shape/r3make), using the
project's [`r3make.json`](r3make.json) build configuration. The configuration
builds the ECX shared library and defines a test executable. To build the project with r3make just run `r3make -c` to build the library.

## Core Concepts

- A **scene** owns entities, component definitions, queries, and systems.
- An **entity** is a handle that can have components bound to it.
- A **component definition** describes an ordered list of fields by byte size.
  Each entity with that component has one value for each field.
- A **query** selects archetypes that contain all included components and none
  of the excluded components.
- A **system** runs callbacks for the archetypes matched by its query.

Handles are `u64` values. Treat them as scene-local identifiers: do not use a
handle after its object or scene has been destroyed. Creation functions return
an invalid handle if creation fails.

## Create a Scene and Use Components

Include the public header and create a scene with the desired entity capacity.
The scene is returned by value and should be passed by address to ECX
functions.

```c
#include <ecx/ecx.h>

typedef struct Position {
    float x;
    float y;
} Position;

int main(void) {
    ECXScene scene = ECXCreateScene(1024);
    if (!scene.entPool) {
        return 1;
    }

    u64 position = ECXCreateComponent((ECXCompDesc){
        .fields = 1,
        .strides = (u64[]){ sizeof(Position) }
    }, &scene);

    u64 entity = ECXCreateEntity(&scene);
    if (position == ECX_INVALID_HANDLE || entity == ECX_INVALID_HANDLE) {
        ECXDestroyScene(&scene);
        return 1;
    }

    ECXBindComponent(entity, position, &scene);

    Position start = { .x = 10.0f, .y = 20.0f };
    ECXSetComponent(entity, position, 0, &start, &scene);

    Position *stored = ECXGetComponent(entity, position, 0, &scene);
    if (stored) {
        stored->x += 1.0f;
    }

    ECXDestroyScene(&scene);
    return 0;
}
```

`ECXCompDesc.fields` is the number of fields, and `strides[field]` is that
field's size in bytes. The descriptor's `strides` array only needs to remain
valid for the call to `ECXCreateComponent`.

`ECXBindComponent` attaches a component definition to an entity.
`ECXUnbindComponent` removes it. `ECXSetComponent` copies the number of bytes
specified by the field's stride from `val`. `ECXGetComponent` returns a mutable
pointer to the field or `NULL` when the entity, component, or field is not
available.

Unbind a component from its entities before calling `ECXDestroyComponent`.

The pointer returned by `ECXGetComponent` addresses ECX-owned component data;
it is not a separately allocated value. Treat it as temporary and reacquire it
after structural changes such as binding or unbinding components, and never use
it after destroying the entity or scene.

## Component Views

`ECXCreateComponentView` gathers pointers to every field of one component on an
entity. Its `fieldv` array is allocated for the view and must be released with
`ECXDestroyComponentView`:

```c
ECXCompView view = ECXCreateComponentView(entity, position, &scene);
if (view.fieldv) {
    Position *value = (Position *)view.fieldv[0];
    value->y += 2.0f;
    ECXDestroyComponentView(&view);
}
```

The pointers in `fieldv` refer to ECX-owned data and have the same temporary
lifetime as pointers returned by `ECXGetComponent`.

`ECXProjectComponent` provides an alternative when a typed view is more
convenient. Pass a caller-owned struct containing one pointer per component
field, in field order. Each pointer receives the address of that field's
storage. The struct's pointer types and order must match the component
definition.

## Queries

Create a query from arrays of component handles. `includes` and `excludes` are
the lengths of their respective arrays; either can be zero.

```c
u64 query = ECXCreateQuery((ECXQueryDesc){
    .includes = 1,
    .excludes = 1,
    .includev = (u64[]){ position },
    .excludev = (u64[]){ hidden }
}, &scene);
```

This query matches archetypes with `position` and without `hidden`. ECX
reuses an existing query when its include and exclude sets are identical, so
component order does not affect query identity. `ECXCreateQueryView` returns a
view of the query's matching archetype handles. The view is borrowed: do not
modify or free its `archv` array, and do not use it after destroying the query
or scene.

## Systems

A system requires a valid query and a `main` callback. `init` and `exit` are
optional. Each callback receives the user pointer, a view of one matching
archetype, and the scene. For each matching archetype, ECX invokes `init` (if
provided), then `main`, then `exit` (if provided).

```c
static void Update(void *user, const ECXArchView *arch, const ECXScene *scene) {
    (void)user;
    (void)arch;
    (void)scene;
    /* Process this matching archetype. */
}

ECXSysDesc desc = {
    .user = NULL,
    .query = query,
    .main = Update
};
u64 system = ECXCreateSystem(desc, &scene);
ECXRunSystem(system, &scene);
```

`ECXArchView` describes the current archetype and exposes its entity count,
component/field metadata, and data pointer. `ECXGetArchViewField` can retrieve a
field's data from a component handle and field index. The returned data belongs
to the scene; do not retain its pointer across structural changes or scene
destruction.

## Scene Snapshots

`ECXCompileScene(path, &scene)` creates an in-memory `ECXBinScene` snapshot and,
when `path` is non-`NULL`, writes it to that path. The returned snapshot owns
memory and must be released with `ECXDestroyCompiledScene`.

`ECXDecompileScene(path, &scene)` reads a snapshot from a file and restores a
scene into the supplied `ECXScene`. Its returned `ECXBinScene` also owns memory
and must be passed to `ECXDestroyCompiledScene` when no longer needed. Snapshot
files use ECX's current format; they should not be assumed to be portable
between incompatible ECX versions or platforms. Queries and systems are
runtime objects and are not restored from a snapshot; recreate them after
loading if needed.

## API Index

| Area | Functions |
| --- | --- |
| Scene lifecycle | `ECXCreateScene`, `ECXDestroyScene` |
| Entities | `ECXCreateEntity`, `ECXDestroyEntity` |
| Components | `ECXCreateComponent`, `ECXDestroyComponent`, `ECXBindComponent`, `ECXUnbindComponent`, `ECXGetComponent`, `ECXSetComponent` |
| Component access | `ECXCreateComponentView`, `ECXDestroyComponentView`, `ECXProjectComponent` |
| Queries | `ECXCreateQuery`, `ECXDestroyQuery`, `ECXCreateQueryView` |
| Systems | `ECXCreateSystem`, `ECXDestroySystem`, `ECXRunSystem` |
| Archetype access | `ECXGetArchViewField` |
| Snapshots | `ECXCompileScene`, `ECXDecompileScene`, `ECXDestroyCompiledScene` |
