#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "Image.h"
#include "Linker.h"
#include "asm/zlasm.h"

int main(void) {
    ZlasmResult layout = zlasm_assemble_object(
        ".section data\n.locate 64\n.byte 1\n.align 4\n.byte 2\n", "layout.asm");
    assert(layout.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    ZlImage layout_image;
    ZlImageError layout_error;
    assert(zl_image_decode(layout.binary, layout.binary_size, &layout_image, &layout_error));
    assert(layout_image.section_count == 2 && layout_image.sections[1].address == 64 &&
           layout_image.sections[1].data_size == 5 && layout_image.sections[1].data[0] == 1 &&
           layout_image.sections[1].data[1] == 0 && layout_image.sections[1].data[3] == 0 &&
           layout_image.sections[1].data[4] == 2);
    zl_image_free(&layout_image);
    zlasm_result_free(&layout);

    const char *caller = ".section text\n.global start\n.extern target\nstart: jmp #target\n";
    const char *callee = ".section text\n.global target\ntarget: int 0xff\n";
    ZlasmResult first = zlasm_assemble_object(caller, "caller.asm");
    ZlasmResult second = zlasm_assemble_object(callee, "callee.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    assert(second.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    const byte *objects[] = {first.binary, second.binary};
    size_t sizes[] = {first.binary_size, second.binary_size};
    byte *executable = NULL;
    size_t executable_size = 0;
    ZlLinkError link_error;
    assert(zl_link_objects(objects, sizes, 2, "start", &executable, &executable_size, &link_error));
    ZlImage image;
    ZlImageError image_error;
    assert(zl_image_decode(executable, executable_size, &image, &image_error));
    assert(image.entry_point == 8 && image.section_count == 2);
    assert(image.sections[0].data[4] == 16 && image.sections[0].data[5] == 0);
    zl_image_free(&image);
    free(executable);
    zlasm_result_free(&first);
    zlasm_result_free(&second);

    first = zlasm_assemble_object(".section text\n.entry main\nmain: int 0xff\n", "entry.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    const byte *entry_objects[] = {first.binary};
    size_t entry_sizes[] = {first.binary_size};
    assert(zl_link_objects(entry_objects, entry_sizes, 1, NULL, &executable, &executable_size,
                           &link_error));
    assert(zl_image_decode(executable, executable_size, &image, &image_error));
    assert(image.entry_point == 8);
    zl_image_free(&image);
    free(executable);
    zlasm_result_free(&first);

    first = zlasm_assemble_object(".section text\n.global start\nstart: int 0xff\n", "a.asm");
    second = zlasm_assemble_object(".section text\n.global start\nstart: int 0xff\n", "b.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_NONE &&
           second.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    const byte *duplicates[] = {first.binary, second.binary};
    size_t duplicate_sizes[] = {first.binary_size, second.binary_size};
    assert(!zl_link_objects(duplicates, duplicate_sizes, 2, "start", &executable, &executable_size,
                            &link_error));
    assert(link_error == ZL_LINK_ERROR_DUPLICATE_SYMBOL);
    zlasm_result_free(&first);
    zlasm_result_free(&second);

    first = zlasm_assemble_object(
        ".section text\n.global start\n.extern local\nstart: jmp #local\n", "a.asm");
    second = zlasm_assemble_object(".section text\nlocal: int 0xff\n", "b.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_NONE &&
           second.diagnostic.code == ZLASM_DIAGNOSTIC_NONE);
    const byte *local_objects[] = {first.binary, second.binary};
    size_t local_sizes[] = {first.binary_size, second.binary_size};
    assert(!zl_link_objects(local_objects, local_sizes, 2, "start", &executable, &executable_size,
                            &link_error));
    assert(link_error == ZL_LINK_ERROR_UNRESOLVED_SYMBOL);
    zlasm_result_free(&first);
    zlasm_result_free(&second);

    first = zlasm_assemble_object(".section bss\n.byte 1\n", "bss.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_SECTION_ERROR);
    zlasm_result_free(&first);

    first = zlasm_assemble_object(".section unknown\n", "section.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_SECTION_ERROR);
    zlasm_result_free(&first);

    first = zlasm_assemble_object(".section text\n.locate 4095\n.byte 1, 2\n", "range.asm");
    assert(first.diagnostic.code == ZLASM_DIAGNOSTIC_SECTION_ERROR);
    zlasm_result_free(&first);
    return EXIT_SUCCESS;
}
