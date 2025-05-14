#pragma once
#include <stdbool.h>

#include "stack.h"
#include "vm.h"

typedef struct VirtualMachine vm_t;
typedef struct Object object_t;
void object_free(object_t *obj);

typedef struct {
    size_t size;
    object_t **elements;
} array_t;

typedef enum ObjectKind {
    INTEGER,
    FLOAT,
    STRING,
    ARRAY,
  } object_kind_t;

typedef union ObjectData {
    int v_int;
    float v_float;
    char *v_string;
    array_t v_array;
} object_data_t;

typedef struct Object {
    bool is_marked;
    object_kind_t kind;
    object_data_t data;
} object_t;

object_t *new_integer(vm_t *vm, int value);
object_t *new_float(vm_t *vm, float value);
object_t *new_string(vm_t *vm, const char *value);
object_t *new_array(vm_t *vm, size_t size);

bool array_set(const object_t *array, size_t index, object_t *value);
object_t *array_get(object_t *array, size_t index);