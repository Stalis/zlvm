#include "Image.h"

#include <stdlib.h>
#include <string.h>

enum {
    IMAGE_VERSION = 1,
    IMAGE_HEADER_SIZE = 52,
    IMAGE_SECTION_RECORD_SIZE = 28,
    IMAGE_SYMBOL_RECORD_SIZE = 20,
    IMAGE_RELOCATION_RECORD_SIZE = 20,
};

static const byte image_magic[4] = {'Z', 'L', 'I', 'M'};

static void set_error(ZlImageError *error, ZlImageError value) {
    if (error != NULL) {
        *error = value;
    }
}

static bool add_size(size_t left, size_t right, size_t *result) {
    if (right > SIZE_MAX - left) {
        return false;
    }
    *result = left + right;
    return true;
}

static bool multiply_size(size_t left, size_t right, size_t *result) {
    if (left != 0 && right > SIZE_MAX / left) {
        return false;
    }
    *result = left * right;
    return true;
}

static void put16(byte *output, size_t offset, uint16_t value) {
    output[offset] = (byte)value;
    output[offset + 1] = (byte)(value >> 8);
}

static void put32(byte *output, size_t offset, uint32_t value) {
    for (size_t index = 0; index < sizeof value; index++) {
        output[offset + index] = (byte)(value >> (index * 8));
    }
}

static uint16_t get16(const byte *input, size_t offset) {
    return (uint16_t)input[offset] | (uint16_t)input[offset + 1] << 8;
}

static uint32_t get32(const byte *input, size_t offset) {
    uint32_t value = 0;
    for (size_t index = 0; index < sizeof value; index++) {
        value |= (uint32_t)input[offset + index] << (index * 8);
    }
    return value;
}

static bool in_range(size_t offset, size_t size, size_t input_size) {
    return offset <= input_size && size <= input_size - offset;
}

static bool get_string(const byte *strings, size_t string_size, uint32_t offset,
                       const char **value) {
    if (offset >= string_size) {
        return false;
    }
    const byte *end = memchr(strings + offset, '\0', string_size - offset);
    if (end == NULL) {
        return false;
    }
    *value = (const char *)(strings + offset);
    return true;
}

static bool append_string(byte *strings, size_t string_size, size_t *cursor, const char *value,
                          uint32_t *offset) {
    value = value == NULL ? "" : value;
    size_t length = strlen(value) + 1;
    if (length > string_size - *cursor || *cursor > UINT32_MAX) {
        return false;
    }
    *offset = (uint32_t)*cursor;
    memcpy(strings + *cursor, value, length);
    *cursor += length;
    return true;
}

static bool valid_alignment(uint32_t alignment) {
    return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

static char *copy_string(const char *value) {
    size_t length = strlen(value) + 1;
    char *copy = malloc(length);
    if (copy != NULL) {
        memcpy(copy, value, length);
    }
    return copy;
}

void zl_image_free(ZlImage *image) {
    if (image == NULL) {
        return;
    }
    if (image->sections != NULL) {
        for (size_t index = 0; index < image->section_count; index++) {
            free((void *)image->sections[index].name);
            free((void *)image->sections[index].data);
        }
    }
    if (image->symbols != NULL) {
        for (size_t index = 0; index < image->symbol_count; index++) {
            free((void *)image->symbols[index].name);
        }
    }
    free(image->sections);
    free(image->symbols);
    free(image->relocations);
    *image = (ZlImage){0};
}

bool zl_image_encode(const ZlImage *image, byte **output, size_t *output_size,
                     ZlImageError *error) {
    if (image == NULL || output == NULL || output_size == NULL ||
        (image->section_count != 0 && image->sections == NULL) ||
        (image->symbol_count != 0 && image->symbols == NULL) ||
        (image->relocation_count != 0 && image->relocations == NULL)) {
        set_error(error, ZL_IMAGE_ERROR_INVALID_ARGUMENT);
        return false;
    }
    if (image->kind != ZL_IMAGE_OBJECT && image->kind != ZL_IMAGE_EXECUTABLE) {
        set_error(error, ZL_IMAGE_ERROR_INVALID_KIND);
        return false;
    }
    if (image->section_count > UINT32_MAX || image->symbol_count > UINT32_MAX ||
        image->relocation_count > UINT32_MAX) {
        set_error(error, ZL_IMAGE_ERROR_OVERFLOW);
        return false;
    }
    if (image->kind == ZL_IMAGE_EXECUTABLE && image->relocation_count != 0) {
        set_error(error, ZL_IMAGE_ERROR_MALFORMED);
        return false;
    }

    size_t string_size = 1;
    for (size_t index = 0; index < image->section_count + image->symbol_count; index++) {
        const char *name = index < image->section_count
                               ? image->sections[index].name
                               : image->symbols[index - image->section_count].name;
        size_t length = strlen(name == NULL ? "" : name) + 1;
        if (!add_size(string_size, length, &string_size)) {
            set_error(error, ZL_IMAGE_ERROR_OVERFLOW);
            return false;
        }
    }
    size_t section_bytes;
    size_t symbol_bytes;
    size_t relocation_bytes;
    if (!multiply_size(image->section_count, IMAGE_SECTION_RECORD_SIZE, &section_bytes) ||
        !multiply_size(image->symbol_count, IMAGE_SYMBOL_RECORD_SIZE, &symbol_bytes) ||
        !multiply_size(image->relocation_count, IMAGE_RELOCATION_RECORD_SIZE, &relocation_bytes)) {
        set_error(error, ZL_IMAGE_ERROR_OVERFLOW);
        return false;
    }
    size_t section_offset = IMAGE_HEADER_SIZE;
    size_t symbol_offset;
    size_t relocation_offset;
    size_t string_offset;
    size_t payload_offset;
    if (!add_size(section_offset, section_bytes, &symbol_offset) ||
        !add_size(symbol_offset, symbol_bytes, &relocation_offset) ||
        !add_size(relocation_offset, relocation_bytes, &string_offset) ||
        !add_size(string_offset, string_size, &payload_offset)) {
        set_error(error, ZL_IMAGE_ERROR_OVERFLOW);
        return false;
    }

    size_t payload_size = 0;
    for (size_t index = 0; index < image->section_count; index++) {
        const ZlImageSection *section = &image->sections[index];
        if (section->data_size > UINT32_MAX || (section->data_size != 0 && section->data == NULL) ||
            !valid_alignment(section->alignment) ||
            (section->type < ZL_IMAGE_SECTION_TEXT || section->type > ZL_IMAGE_SECTION_BSS) ||
            (section->type == ZL_IMAGE_SECTION_BSS && section->data_size != 0) ||
            section->data_size > UINT32_MAX - section->address) {
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
        if (!add_size(payload_size, section->data_size, &payload_size)) {
            set_error(error, ZL_IMAGE_ERROR_OVERFLOW);
            return false;
        }
    }
    for (size_t index = 0; index < image->symbol_count; index++) {
        const ZlImageSymbol *symbol = &image->symbols[index];
        if (symbol->section_index != ZL_IMAGE_UNDEFINED_SECTION &&
            symbol->section_index >= image->section_count) {
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
    }
    for (size_t index = 0; index < image->relocation_count; index++) {
        const ZlImageRelocation *relocation = &image->relocations[index];
        if (relocation->section_index >= image->section_count ||
            relocation->symbol_index >= image->symbol_count ||
            (relocation->type != ZL_IMAGE_RELOCATION_ABSOLUTE32 &&
             relocation->type != ZL_IMAGE_RELOCATION_PC_RELATIVE32) ||
            relocation->offset > image->sections[relocation->section_index].data_size ||
            image->sections[relocation->section_index].data_size - relocation->offset < 4) {
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
    }
    size_t total_size;
    if (!add_size(payload_offset, payload_size, &total_size) || total_size > UINT32_MAX) {
        set_error(error, ZL_IMAGE_ERROR_OVERFLOW);
        return false;
    }
    if (image->kind == ZL_IMAGE_OBJECT && image->entry_point != 0) {
        set_error(error, ZL_IMAGE_ERROR_MALFORMED);
        return false;
    }
    if (image->kind == ZL_IMAGE_EXECUTABLE && image->entry_point == 0) {
        set_error(error, ZL_IMAGE_ERROR_MALFORMED);
        return false;
    }
    if (image->kind == ZL_IMAGE_EXECUTABLE) {
        bool entry_found = false;
        for (size_t index = 0; index < image->section_count; index++) {
            const ZlImageSection *section = &image->sections[index];
            if (section->type == ZL_IMAGE_SECTION_TEXT && image->entry_point >= section->address &&
                image->entry_point - section->address < section->data_size) {
                entry_found = true;
            }
        }
        if (!entry_found) {
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
    }
    byte *result = calloc(total_size, 1);
    if (result == NULL) {
        set_error(error, ZL_IMAGE_ERROR_OUT_OF_MEMORY);
        return false;
    }

    memcpy(result, image_magic, sizeof image_magic);
    put16(result, 4, IMAGE_VERSION);
    result[6] = image->kind;
    put32(result, 8, IMAGE_HEADER_SIZE);
    put32(result, 12, (uint32_t)image->section_count);
    put32(result, 16, (uint32_t)image->symbol_count);
    put32(result, 20, (uint32_t)image->relocation_count);
    put32(result, 24, image->entry_point);
    put32(result, 28, (uint32_t)section_offset);
    put32(result, 32, (uint32_t)symbol_offset);
    put32(result, 36, (uint32_t)relocation_offset);
    put32(result, 40, (uint32_t)string_offset);
    put32(result, 44, (uint32_t)string_size);
    put32(result, 48, (uint32_t)payload_offset);

    byte *strings = result + string_offset;
    size_t string_cursor = 1;
    size_t payload_cursor = payload_offset;
    for (size_t index = 0; index < image->section_count; index++) {
        const ZlImageSection *section = &image->sections[index];
        size_t record = section_offset + index * IMAGE_SECTION_RECORD_SIZE;
        uint32_t name_offset;
        append_string(strings, string_size, &string_cursor, section->name, &name_offset);
        put32(result, record, name_offset);
        put32(result, record + 4, section->type);
        put32(result, record + 8, section->flags);
        put32(result, record + 12, section->alignment);
        put32(result, record + 16, section->address);
        put32(result, record + 20, (uint32_t)payload_cursor);
        put32(result, record + 24, (uint32_t)section->data_size);
        memcpy(result + payload_cursor, section->data, section->data_size);
        payload_cursor += section->data_size;
    }
    for (size_t index = 0; index < image->symbol_count; index++) {
        const ZlImageSymbol *symbol = &image->symbols[index];
        size_t record = symbol_offset + index * IMAGE_SYMBOL_RECORD_SIZE;
        uint32_t name_offset;
        append_string(strings, string_size, &string_cursor, symbol->name, &name_offset);
        put32(result, record, name_offset);
        put32(result, record + 4, symbol->section_index);
        put32(result, record + 8, symbol->value);
        put32(result, record + 12, symbol->size);
        result[record + 16] = symbol->binding;
        result[record + 17] = symbol->type;
    }
    for (size_t index = 0; index < image->relocation_count; index++) {
        const ZlImageRelocation *relocation = &image->relocations[index];
        size_t record = relocation_offset + index * IMAGE_RELOCATION_RECORD_SIZE;
        put32(result, record, relocation->section_index);
        put32(result, record + 4, relocation->offset);
        put32(result, record + 8, relocation->type);
        put32(result, record + 12, relocation->symbol_index);
        put32(result, record + 16, (uint32_t)relocation->addend);
    }
    *output = result;
    *output_size = total_size;
    set_error(error, ZL_IMAGE_ERROR_NONE);
    return true;
}

bool zl_image_decode(const byte *input, size_t input_size, ZlImage *image, ZlImageError *error) {
    if (input == NULL || image == NULL) {
        set_error(error, ZL_IMAGE_ERROR_INVALID_ARGUMENT);
        return false;
    }
    *image = (ZlImage){0};
    if (input_size < IMAGE_HEADER_SIZE || memcmp(input, image_magic, sizeof image_magic) != 0) {
        set_error(error, ZL_IMAGE_ERROR_BAD_MAGIC);
        return false;
    }
    if (get16(input, 4) != IMAGE_VERSION) {
        set_error(error, ZL_IMAGE_ERROR_UNSUPPORTED_VERSION);
        return false;
    }
    uint8_t kind = input[6];
    if ((kind != ZL_IMAGE_OBJECT && kind != ZL_IMAGE_EXECUTABLE) || input[7] != 0 ||
        get32(input, 8) != IMAGE_HEADER_SIZE) {
        set_error(error, ZL_IMAGE_ERROR_MALFORMED);
        return false;
    }
    uint32_t section_count = get32(input, 12);
    uint32_t symbol_count = get32(input, 16);
    uint32_t relocation_count = get32(input, 20);
    size_t section_offset = get32(input, 28);
    size_t symbol_offset = get32(input, 32);
    size_t relocation_offset = get32(input, 36);
    size_t string_offset = get32(input, 40);
    size_t string_size = get32(input, 44);
    size_t payload_offset = get32(input, 48);
    size_t section_bytes;
    size_t symbol_bytes;
    size_t relocation_bytes;
    if (!multiply_size(section_count, IMAGE_SECTION_RECORD_SIZE, &section_bytes) ||
        !multiply_size(symbol_count, IMAGE_SYMBOL_RECORD_SIZE, &symbol_bytes) ||
        !multiply_size(relocation_count, IMAGE_RELOCATION_RECORD_SIZE, &relocation_bytes) ||
        !in_range(section_offset, section_bytes, input_size) ||
        !in_range(symbol_offset, symbol_bytes, input_size) ||
        !in_range(relocation_offset, relocation_bytes, input_size) ||
        !in_range(string_offset, string_size, input_size) || payload_offset > input_size ||
        section_offset < IMAGE_HEADER_SIZE || symbol_offset < section_offset + section_bytes ||
        relocation_offset < symbol_offset + symbol_bytes ||
        string_offset < relocation_offset + relocation_bytes || string_offset > payload_offset ||
        string_size > payload_offset - string_offset) {
        set_error(error, ZL_IMAGE_ERROR_BOUNDS);
        return false;
    }
    image->kind = kind;
    image->entry_point = get32(input, 24);
    if ((kind == ZL_IMAGE_OBJECT && image->entry_point != 0) ||
        (kind == ZL_IMAGE_EXECUTABLE && (image->entry_point == 0 || relocation_count != 0))) {
        set_error(error, ZL_IMAGE_ERROR_MALFORMED);
        return false;
    }
    image->section_count = section_count;
    image->symbol_count = symbol_count;
    image->relocation_count = relocation_count;
    image->sections = calloc(section_count, sizeof *image->sections);
    image->symbols = calloc(symbol_count, sizeof *image->symbols);
    image->relocations = calloc(relocation_count, sizeof *image->relocations);
    if ((section_count != 0 && image->sections == NULL) ||
        (symbol_count != 0 && image->symbols == NULL) ||
        (relocation_count != 0 && image->relocations == NULL)) {
        zl_image_free(image);
        set_error(error, ZL_IMAGE_ERROR_OUT_OF_MEMORY);
        return false;
    }

    const byte *strings = input + string_offset;
    for (size_t index = 0; index < section_count; index++) {
        size_t record = section_offset + index * IMAGE_SECTION_RECORD_SIZE;
        uint32_t data_offset = get32(input, record + 20);
        uint32_t data_size = get32(input, record + 24);
        const char *name;
        uint32_t type = get32(input, record + 4);
        uint32_t address = get32(input, record + 16);
        if (!get_string(strings, string_size, get32(input, record), &name) ||
            !valid_alignment(get32(input, record + 12)) ||
            (type < ZL_IMAGE_SECTION_TEXT || type > ZL_IMAGE_SECTION_BSS) ||
            (type == ZL_IMAGE_SECTION_BSS && data_size != 0) || data_offset < payload_offset ||
            !in_range(data_offset, data_size, input_size) || data_size > UINT32_MAX - address) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
        image->sections[index].name = copy_string(name);
        image->sections[index].type = type;
        image->sections[index].flags = get32(input, record + 8);
        image->sections[index].alignment = get32(input, record + 12);
        image->sections[index].address = get32(input, record + 16);
        image->sections[index].data_size = data_size;
        image->sections[index].data = malloc(data_size);
        if (image->sections[index].name == NULL ||
            (data_size != 0 && image->sections[index].data == NULL)) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_OUT_OF_MEMORY);
            return false;
        }
        memcpy((void *)image->sections[index].data, input + data_offset, data_size);
    }
    for (size_t index = 0; index < symbol_count; index++) {
        size_t record = symbol_offset + index * IMAGE_SYMBOL_RECORD_SIZE;
        uint32_t section_index = get32(input, record + 4);
        const char *name;
        if (!get_string(strings, string_size, get32(input, record), &name) ||
            (section_index != ZL_IMAGE_UNDEFINED_SECTION && section_index >= section_count)) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
        image->symbols[index].name = copy_string(name);
        image->symbols[index].section_index = section_index;
        image->symbols[index].value = get32(input, record + 8);
        image->symbols[index].size = get32(input, record + 12);
        image->symbols[index].binding = input[record + 16];
        image->symbols[index].type = input[record + 17];
        if (image->symbols[index].name == NULL) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_OUT_OF_MEMORY);
            return false;
        }
        if (input[record + 18] != 0 || input[record + 19] != 0) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
    }
    for (size_t index = 0; index < relocation_count; index++) {
        size_t record = relocation_offset + index * IMAGE_RELOCATION_RECORD_SIZE;
        ZlImageRelocation *relocation = &image->relocations[index];
        relocation->section_index = get32(input, record);
        relocation->offset = get32(input, record + 4);
        relocation->type = get32(input, record + 8);
        relocation->symbol_index = get32(input, record + 12);
        relocation->addend = (int32_t)get32(input, record + 16);
        if (relocation->section_index >= section_count ||
            relocation->symbol_index >= symbol_count ||
            (relocation->type != ZL_IMAGE_RELOCATION_ABSOLUTE32 &&
             relocation->type != ZL_IMAGE_RELOCATION_PC_RELATIVE32) ||
            relocation->offset > image->sections[relocation->section_index].data_size ||
            image->sections[relocation->section_index].data_size - relocation->offset < 4) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
    }
    if (kind == ZL_IMAGE_EXECUTABLE) {
        bool entry_found = false;
        if (image->entry_point == 0) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
        for (size_t index = 0; index < section_count; index++) {
            const ZlImageSection *section = &image->sections[index];
            if (section->type == ZL_IMAGE_SECTION_TEXT && image->entry_point >= section->address &&
                image->entry_point - section->address < section->data_size) {
                entry_found = true;
            }
        }
        if (!entry_found) {
            zl_image_free(image);
            set_error(error, ZL_IMAGE_ERROR_MALFORMED);
            return false;
        }
    }
    set_error(error, ZL_IMAGE_ERROR_NONE);
    return true;
}
