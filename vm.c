#include "vm.h"

void vm_collect_garbage(vm_t *vm) {
    mark(vm);
    trace(vm);
    sweep(vm);
}

void sweep(vm_t *vm) {
    if (!vm) return;
    for (int i = 0; i < vm->objects->count; i++) {
        object_t *obj = vm->objects->data[i];
        if (obj->is_marked) obj->is_marked = false;
        else {
            object_free(obj);
            vm->objects->data[i] = NULL;
        }
    }
    stack_remove_nulls(vm->objects);
}

void trace(vm_t *vm) {
    stack_t *gray_objects = stack_new(8);
    if (!gray_objects) return;

    for (int i = 0; i < vm->objects->count; i++) {
        object_t *obj = vm->objects->data[i];
        if (obj->is_marked) stack_push(gray_objects, obj);
    }

    while (gray_objects->count > 0) {
        object_t *obj = stack_pop(gray_objects);
        trace_blacken_object(gray_objects, obj);
    }

    stack_free(gray_objects);
}

void trace_blacken_object(stack_t *gray_objects, object_t *obj) {
    if (obj->kind == ARRAY) {
        for (int i = 0; i < obj->data.v_array.size; i++) {
            trace_mark_object(gray_objects, array_get(obj, i));
        }
    }
}

void trace_mark_object(stack_t *gray_objects, object_t *obj) {
    if (!obj || obj->is_marked) return;
    obj->is_marked = true;
    stack_push(gray_objects, obj);
}

void mark(vm_t *vm) {
    if (!vm) return;
    for (int i = 0; i < vm->frames->count; i++) {
        const frame_t *frame = vm->frames->data[i];
        for (int j = 0; j < frame->references->count; j++) {
            object_t *obj = frame->references->data[j];
            obj->is_marked = true;
        }
    }
}

void frame_reference_object(frame_t *frame, object_t *obj) {
    if (!frame || !obj) return;
    stack_push(frame->references, obj);
}

void vm_track_object(vm_t *vm, object_t *obj) {
    if (!vm || !obj) return;
    stack_push(vm->objects, obj);
}

void vm_frame_push(vm_t *vm, frame_t *frame) {
    if (vm == NULL || frame == NULL) return;
    stack_push(vm->frames, frame);
}

frame_t *vm_new_frame(vm_t *vm) {
    if (vm == NULL) return NULL;

    frame_t *frame = malloc(sizeof(frame_t));
    if (frame == NULL) return NULL;

    frame->references = stack_new(8);
    if (frame->references == NULL) {
        free(frame);
        return NULL;
    }

    vm_frame_push(vm, frame);
    return frame;
}

void frame_free(frame_t *frame) {
    if (frame == NULL) return;
    stack_free(frame->references);
    free(frame);
}

vm_t *vm_new() {
    vm_t *vm = malloc(sizeof(vm_t));
    if (vm == NULL) return NULL;
    vm->frames = stack_new(8);
    vm->objects = stack_new(8);
    return vm;
}

void vm_free(vm_t *vm) {
    if (vm == NULL) return;
    for (int i = 0; i < vm->frames->count; i++) {
        frame_free(vm->frames->data[i]);
    }
    stack_free(vm->frames);
    for (int i = 0; i < vm->objects->count; i++) {
        object_free(vm->objects->data[i]);
    }
    stack_free(vm->objects);
    free(vm);
}

frame_t *vm_frame_pop(vm_t *vm) {
    return stack_pop(vm->frames);
}