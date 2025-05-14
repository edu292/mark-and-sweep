#include "munit.h"
#include "object.h"
#include "vm.h"

static MunitResult test_garbage_collection(const MunitParameter params[], void *data) {
    // Create a new virtual machine
    vm_t *vm = vm_new();
    munit_assert_not_null(vm);

    // Create a new frame
    frame_t *frame = vm_new_frame(vm);
    munit_assert_not_null(frame);

    // Create objects that should be kept
    object_t *str1 = new_string(vm, "Hello");
    object_t *int1 = new_integer(vm, 42);
    object_t *array1 = new_array(vm, 2);

    // Reference objects in the frame to keep them alive
    frame_reference_object(frame, str1);
    frame_reference_object(frame, int1);
    frame_reference_object(frame, array1);

    // Set array elements
    array_set(array1, 0, str1);
    array_set(array1, 1, int1);

    // Create objects that should be collected (not referenced)
    object_t *unused_str = new_string(vm, "I will be collected");
    object_t *unused_int = new_integer(vm, 100);

    // Store initial object count
    size_t initial_count = vm->objects->count;
    munit_assert_size(initial_count, ==, 5); // 3 referenced + 2 unreferenced objects

    // Trigger garbage collection
    vm_collect_garbage(vm);

    // Check that unused objects were collected
    size_t final_count = vm->objects->count;
    munit_assert_size(final_count, ==, 3); // Only referenced objects should remain

    // Verify that referenced objects are still valid
    munit_assert_string_equal(str1->data.v_string, "Hello");
    munit_assert_int(int1->data.v_int, ==, 42);
    munit_assert_size(array1->data.v_array.size, ==, 2);

    // Verify array contents are preserved
    object_t *array_str = array_get(array1, 0);
    object_t *array_int = array_get(array1, 1);
    munit_assert_string_equal(array_str->data.v_string, "Hello");
    munit_assert_int(array_int->data.v_int, ==, 42);

    // Create new frame and move references
    frame_t *new_frame = vm_new_frame(vm);
    frame_reference_object(new_frame, array1);

    // Pop old frame and collect garbage
    vm_frame_pop(vm);
    vm_collect_garbage(vm);

    // Only array and its contents should survive
    munit_assert_size(vm->objects->count, ==, 3); // array and its two elements

    // Clean up
    vm_free(vm);
    return MUNIT_OK;
}

static MunitResult test_complex_object_graph(const MunitParameter params[], void *data) {
    vm_t *vm = vm_new();
    munit_assert_not_null(vm);
    frame_t *frame = vm_new_frame(vm);
    munit_assert_not_null(frame);

    // Create a complex graph structure
    object_t *main_array = new_array(vm, 3);
    object_t *sub_array1 = new_array(vm, 2);
    object_t *sub_array2 = new_array(vm, 2);
    object_t *shared_str = new_string(vm, "Shared String");

    // Create diamond-shaped reference pattern
    frame_reference_object(frame, main_array);
    array_set(main_array, 0, sub_array1);
    array_set(main_array, 1, sub_array2);
    array_set(main_array, 2, shared_str);
    array_set(sub_array1, 0, shared_str);
    array_set(sub_array2, 0, shared_str);

    // Create some unreferenced objects
    new_string(vm, "Unreferenced 1");
    new_string(vm, "Unreferenced 2");

    size_t initial_count = vm->objects->count;
    vm_collect_garbage(vm);

    // Only the diamond structure should survive
    munit_assert_size(vm->objects->count, ==, 4);  // main_array, sub_array1, sub_array2, shared_str

    // Verify the structure is intact
    munit_assert_string_equal(((object_t*)array_get(sub_array1, 0))->data.v_string, "Shared String");
    munit_assert_string_equal(((object_t*)array_get(sub_array2, 0))->data.v_string, "Shared String");
    munit_assert_ptr_equal(array_get(sub_array1, 0), array_get(sub_array2, 0));

    vm_free(vm);
    return MUNIT_OK;
}

static MunitResult test_frame_management_edge_cases(const MunitParameter params[], void *data) {
    vm_t *vm = vm_new();
    munit_assert_not_null(vm);

    // Test multiple frame pushes and pops
    frame_t *frames[10];
    object_t *shared_obj = NULL;

    // Create multiple frames with shared objects
    for (int i = 0; i < 10; i++) {
        frames[i] = vm_new_frame(vm);
        munit_assert_not_null(frames[i]);

        // Create an object in first frame and share it with others
        if (i == 0) {
            shared_obj = new_string(vm, "Shared across frames");
            frame_reference_object(frames[i], shared_obj);
        } else {
            frame_reference_object(frames[i], shared_obj);
        }
    }

    // Pop frames in reverse order except the first one
    for (int i = 9; i > 0; i--) {
        vm_frame_pop(vm);
        vm_collect_garbage(vm);
        // Shared object should still exist as it's referenced by first frame
        munit_assert_ptr_not_null(shared_obj);
        munit_assert_string_equal(shared_obj->data.v_string, "Shared across frames");
    }

    // Pop the last frame
    vm_frame_pop(vm);
    vm_collect_garbage(vm);
    // Now all objects should be collected
    munit_assert_size(vm->objects->count, ==, 0);

    vm_free(vm);
    return MUNIT_OK;
}

static MunitResult test_gc_corner_cases(const MunitParameter params[], void *data) {
    vm_t *vm = vm_new();
    munit_assert_not_null(vm);
    frame_t *frame = vm_new_frame(vm);
    munit_assert_not_null(frame);

    // Test case 1: Nested arrays with mixed types
    object_t *nested = new_array(vm, 3);
    frame_reference_object(frame, nested);

    object_t *int_obj = new_integer(vm, 42);
    object_t *float_obj = new_float(vm, 3.14f);
    object_t *inner_array = new_array(vm, 2);

    array_set(nested, 0, int_obj);
    array_set(nested, 1, float_obj);
    array_set(nested, 2, inner_array);

    array_set(inner_array, 0, new_string(vm, "Inner String"));
    array_set(inner_array, 1, new_integer(vm, 100));

    // Test case 2: Array with NULL elements
    object_t *sparse_array = new_array(vm, 5);
    frame_reference_object(frame, sparse_array);
    array_set(sparse_array, 2, new_string(vm, "Middle"));

    // Test case 3: Multiple GC runs
    for (int i = 0; i < 5; i++) {
        vm_collect_garbage(vm);
        // Verify objects are still intact
        munit_assert_int(((object_t*)array_get(nested, 0))->data.v_int, ==, 42);
        munit_assert_float(((object_t*)array_get(nested, 1))->data.v_float, ==, 3.14f);
        munit_assert_string_equal(
            ((object_t*)array_get(sparse_array, 2))->data.v_string,
            "Middle"
        );
    }

    // Test case 4: Remove all references and verify cleanup
    vm_frame_pop(vm);
    vm_collect_garbage(vm);
    munit_assert_size(vm->objects->count, ==, 0);

    vm_free(vm);
    return MUNIT_OK;
}


static MunitTest test_suite_tests[] = {
    {
        "/gc_test",
        test_garbage_collection,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/complex_object_graph",
        test_complex_object_graph,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/frame_management_edge_cases",
        test_frame_management_edge_cases,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/gc_corner_cases",
        test_gc_corner_cases,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
{NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

static const MunitSuite test_suite = {
    "/garbage_collection",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}