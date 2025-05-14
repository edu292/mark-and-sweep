#include <string.h>
#include "object.h"

void object_free(object_t *obj) {
    switch (obj->kind) {
        case STRING:
            free(obj->data.v_string);
            break;
        case ARRAY: {
            free(obj->data.v_array.elements);
            break;
        }
    }
    free(obj);
}

object_t *_new_object(vm_t *vm) {
    object_t *obj = calloc(1, sizeof(object_t));
    if (obj == NULL) {
        return NULL;
    }
    obj->is_marked = false;
    vm_track_object(vm, obj);
    return obj;
}

object_t *new_array(vm_t *vm, const size_t size) {
    object_t *obj = _new_object(vm);
    if (obj == NULL) {
        return NULL;
    }

    object_t **elements = calloc(size, sizeof(object_t *));
    if (elements == NULL) {
        free(obj);
        return NULL;
    }

    obj->kind = ARRAY;
    obj->data.v_array = (array_t){.size = size, .elements = elements};

    return obj;
}

object_t *new_integer(vm_t *vm, const int value) {
    object_t *obj = _new_object(vm);
    if (obj == NULL) {
        return NULL;
    }

    obj->kind = INTEGER;
    obj->data.v_int = value;

    return obj;
}

object_t *new_float(vm_t *vm, float value) {
    object_t *obj = _new_object(vm);
    if (obj == NULL) {
        return NULL;
    }

    obj->kind = FLOAT;
    obj->data.v_float = value;
    return obj;
}

object_t *new_string(vm_t *vm, const char *value) {
    object_t *obj = _new_object(vm);
    if (obj == NULL) {
        return NULL;
    }

    int len = strlen(value);
    char *dst = malloc(len + 1);
    if (dst == NULL) {
        free(obj);
        return NULL;
    }

    strcpy(dst, value);

    obj->kind = STRING;
    obj->data.v_string = dst;
    return obj;
}

bool array_set(const object_t *array, const size_t index, object_t *value) {
    if (array == NULL || value == NULL) {
        return false;
    }

    if (array->kind != ARRAY) {
        return false;
    }

    if (index >= array->data.v_array.size) {
        return false;
    }
    array->data.v_array.elements[index] = value;
    return true;
}

object_t *array_get(object_t *array, size_t index) {
    if (array == NULL) {
        return NULL;
    }

    if (array->kind != ARRAY) {
        return NULL;
    }

    if (index >= array->data.v_array.size) {
        return NULL;
    }

    return array->data.v_array.elements[index];
}
