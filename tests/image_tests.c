#include <stdlib.h>
#include <string.h>

#include "Image.h"
#include "VirtualMachine.h"

#define REQUIRE(condition)                                                                         \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool test_empty_object_golden_bytes(void) {
    ZlImage image = {.kind = ZL_IMAGE_OBJECT};
    byte *encoded = NULL;
    size_t encoded_size = 0;
    ZlImageError error = ZL_IMAGE_ERROR_NONE;
    REQUIRE(zl_image_encode(&image, &encoded, &encoded_size, &error));
    static const byte expected[] = {
        0x5a, 0x4c, 0x49, 0x4d, 0x01, 0x00, 0x01, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x34, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x34, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00,
    };
    REQUIRE(encoded_size == sizeof expected);
    REQUIRE(memcmp(encoded, expected, sizeof expected) == 0);
    free(encoded);
    return true;
}

static bool test_object_round_trip(void) {
    byte code[] = {0x00, 0x00, 0x00, 0x00};
    ZlImageSection sections[] = {{".text", ZL_IMAGE_SECTION_TEXT, 1, 4, 0, code, sizeof code}};
    ZlImageSymbol symbols[] = {{"start", 0, 0, 4, ZL_IMAGE_SYMBOL_GLOBAL, ZL_IMAGE_SYMBOL_FUNCTION},
                               {"external", ZL_IMAGE_UNDEFINED_SECTION, 0, 0,
                                ZL_IMAGE_SYMBOL_GLOBAL, ZL_IMAGE_SYMBOL_NOTYPE}};
    ZlImageRelocation relocations[] = {{0, 0, ZL_IMAGE_RELOCATION_ABSOLUTE32, 1, 4}};
    ZlImage image = {.kind = ZL_IMAGE_OBJECT,
                     .sections = sections,
                     .section_count = 1,
                     .symbols = symbols,
                     .symbol_count = 2,
                     .relocations = relocations,
                     .relocation_count = 1};
    byte *encoded = NULL;
    size_t encoded_size = 0;
    ZlImageError error;
    REQUIRE(zl_image_encode(&image, &encoded, &encoded_size, &error));
    ZlImage decoded = {0};
    REQUIRE(zl_image_decode(encoded, encoded_size, &decoded, &error));
    REQUIRE(decoded.section_count == 1 && strcmp(decoded.sections[0].name, ".text") == 0);
    REQUIRE(decoded.symbol_count == 2 &&
            decoded.symbols[1].section_index == ZL_IMAGE_UNDEFINED_SECTION);
    REQUIRE(decoded.relocations[0].addend == 4);
    zl_image_free(&decoded);
    encoded[80 + 18] = 1;
    REQUIRE(!zl_image_decode(encoded, encoded_size, &(ZlImage){0}, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_MALFORMED);
    free(encoded);
    encoded = NULL;
    relocations[0].section_index = 1;
    REQUIRE(!zl_image_encode(&image, &encoded, &encoded_size, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_MALFORMED);
    relocations[0].section_index = 0;
    symbols[0].section_index = 3;
    REQUIRE(!zl_image_encode(&image, &encoded, &encoded_size, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_MALFORMED);
    return true;
}

static bool test_executable_entry_and_malformed_input(void) {
    byte code[8] = {0};
    ZlImageSection section = {".text", ZL_IMAGE_SECTION_TEXT, 1, 8, 8, code, sizeof code};
    ZlImage image = {
        .kind = ZL_IMAGE_EXECUTABLE, .entry_point = 8, .sections = &section, .section_count = 1};
    byte *encoded = NULL;
    size_t encoded_size = 0;
    ZlImageError error;
    REQUIRE(zl_image_encode(&image, &encoded, &encoded_size, &error));
    VirtualMachine vm = {0};
    vm_initialize(&vm, 256);
    REQUIRE(vm_loadImage(&vm, encoded, encoded_size, &error));
    REQUIRE(vm._entryPoint == 8);
    vm_destroy(&vm);
    encoded[0] = 0;
    REQUIRE(!zl_image_decode(encoded, encoded_size, &(ZlImage){0}, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_BAD_MAGIC);
    encoded[0] = 'Z';
    encoded[4] = 2;
    REQUIRE(!zl_image_decode(encoded, encoded_size, &(ZlImage){0}, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_UNSUPPORTED_VERSION);
    encoded[4] = 1;
    encoded[7] = 1;
    REQUIRE(!zl_image_decode(encoded, encoded_size, &(ZlImage){0}, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_MALFORMED);
    ZlImage object = {.kind = ZL_IMAGE_OBJECT, .entry_point = 1};
    REQUIRE(!zl_image_encode(&object, &encoded, &encoded_size, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_MALFORMED);
    ZlImageRelocation relocation = {0, 0, ZL_IMAGE_RELOCATION_ABSOLUTE32, 0, 0};
    image.relocations = &relocation;
    image.relocation_count = 1;
    REQUIRE(!zl_image_encode(&image, &encoded, &encoded_size, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_MALFORMED);
    free(encoded);
    return true;
}

static bool test_image_load_is_atomic_and_cleanup_is_safe(void) {
    byte text[] = {0x01};
    byte data[] = {0x02};
    ZlImageSection sections[] = {
        {".text", ZL_IMAGE_SECTION_TEXT, 1, 1, 8, text, sizeof text},
        {".data", ZL_IMAGE_SECTION_DATA, 1, 1, ZLVM_ROM_SIZE, data, sizeof data}};
    ZlImage image = {
        .kind = ZL_IMAGE_EXECUTABLE, .entry_point = 8, .sections = sections, .section_count = 2};
    byte *encoded = NULL;
    size_t encoded_size = 0;
    ZlImageError error;
    REQUIRE(zl_image_encode(&image, &encoded, &encoded_size, &error));
    VirtualMachine vm = {0};
    vm_initialize(&vm, 256);
    vm._rom[8] = 0xaa;
    REQUIRE(!vm_loadImage(&vm, encoded, encoded_size, &error));
    REQUIRE(error == ZL_IMAGE_ERROR_BOUNDS && vm._rom[8] == 0xaa);
    vm_destroy(&vm);
    free(encoded);

    ZlImage incomplete = {.section_count = 1, .symbol_count = 1};
    zl_image_free(&incomplete);
    return true;
}

int main(void) {
    return test_empty_object_golden_bytes() && test_object_round_trip() &&
                   test_executable_entry_and_malformed_input() &&
                   test_image_load_is_atomic_and_cleanup_is_safe()
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
