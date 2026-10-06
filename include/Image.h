#ifndef ZLVM_IMAGE_H
#define ZLVM_IMAGE_H

#include "Types.h"

#include <stddef.h>

#define ZL_IMAGE_UNDEFINED_SECTION UINT32_MAX

typedef enum ZlImageKind {
    ZL_IMAGE_OBJECT = 1,
    ZL_IMAGE_EXECUTABLE = 2,
} ZlImageKind;

typedef enum ZlImageSectionType {
    ZL_IMAGE_SECTION_TEXT = 1,
    ZL_IMAGE_SECTION_DATA = 2,
    ZL_IMAGE_SECTION_BSS = 3,
} ZlImageSectionType;

typedef enum ZlImageSymbolBinding {
    ZL_IMAGE_SYMBOL_LOCAL = 0,
    ZL_IMAGE_SYMBOL_GLOBAL = 1,
    ZL_IMAGE_SYMBOL_WEAK = 2,
} ZlImageSymbolBinding;

typedef enum ZlImageSymbolType {
    ZL_IMAGE_SYMBOL_NOTYPE = 0,
    ZL_IMAGE_SYMBOL_OBJECT = 1,
    ZL_IMAGE_SYMBOL_FUNCTION = 2,
} ZlImageSymbolType;

typedef enum ZlImageRelocationType {
    ZL_IMAGE_RELOCATION_ABSOLUTE32 = 1,
    ZL_IMAGE_RELOCATION_PC_RELATIVE32 = 2,
} ZlImageRelocationType;

typedef enum ZlImageError {
    ZL_IMAGE_ERROR_NONE = 0,
    ZL_IMAGE_ERROR_INVALID_ARGUMENT,
    ZL_IMAGE_ERROR_OUT_OF_MEMORY,
    ZL_IMAGE_ERROR_BAD_MAGIC,
    ZL_IMAGE_ERROR_UNSUPPORTED_VERSION,
    ZL_IMAGE_ERROR_INVALID_KIND,
    ZL_IMAGE_ERROR_MALFORMED,
    ZL_IMAGE_ERROR_OVERFLOW,
    ZL_IMAGE_ERROR_BOUNDS,
} ZlImageError;

typedef struct ZlImageSection {
    const char *name;
    uint32_t type;
    uint32_t flags;
    uint32_t alignment;
    uint32_t address;
    const byte *data;
    size_t data_size;
} ZlImageSection;

typedef struct ZlImageSymbol {
    const char *name;
    uint32_t section_index;
    uint32_t value;
    uint32_t size;
    uint8_t binding;
    uint8_t type;
} ZlImageSymbol;

typedef struct ZlImageRelocation {
    uint32_t section_index;
    uint32_t offset;
    uint32_t type;
    uint32_t symbol_index;
    int32_t addend;
} ZlImageRelocation;

typedef struct ZlImage {
    uint8_t kind;
    uint32_t entry_point;
    ZlImageSection *sections;
    size_t section_count;
    ZlImageSymbol *symbols;
    size_t symbol_count;
    ZlImageRelocation *relocations;
    size_t relocation_count;
} ZlImage;

/**
 * Encode an image into a newly allocated buffer. The caller owns the returned buffer and must free
 * it. Input names and section data remain borrowed and must stay valid for the duration of the
 * call.
 */
bool zl_image_encode(const ZlImage *image, byte **output, size_t *output_size, ZlImageError *error);

/**
 * Decode an image into newly allocated arrays, names, and section data. The caller owns those
 * allocations and must release them with zl_image_free().
 */
bool zl_image_decode(const byte *input, size_t input_size, ZlImage *image, ZlImageError *error);

/** Release allocations owned by a decoded image and reset it to zero. */
void zl_image_free(ZlImage *image);

#endif // ZLVM_IMAGE_H
