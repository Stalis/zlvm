#include "Linker.h"
#include "VirtualMachine.h"

#include <stdlib.h>
#include <string.h>

typedef struct LinkSection {
    ZlImageSection image;
    size_t object_index;
    size_t source_index;
} LinkSection;

static void set_error(ZlLinkError *error, ZlLinkError value) {
    if (error != NULL) {
        *error = value;
    }
}

static void write32(byte *data, uint32_t value) {
    for (size_t index = 0; index < 4; index++) {
        data[index] = (byte)(value >> (index * 8));
    }
}

static bool align_address(uint64_t value, uint32_t alignment, uint32_t *result) {
    if (value > UINT64_MAX - alignment + 1) {
        return false;
    }
    uint64_t aligned = (value + alignment - 1) & ~((uint64_t)alignment - 1);
    if (aligned > UINT32_MAX) {
        return false;
    }
    *result = (uint32_t)aligned;
    return true;
}

static bool overlaps(const LinkSection *sections, size_t count, uint32_t address, size_t size) {
    uint64_t end = (uint64_t)address + size;
    for (size_t index = 0; index < count; index++) {
        uint64_t other_end =
            (uint64_t)sections[index].image.address + sections[index].image.data_size;
        if ((uint64_t)address < other_end && (uint64_t)sections[index].image.address < end) {
            return true;
        }
    }
    return false;
}

static ptrdiff_t find_section(const LinkSection *sections, size_t count, size_t object_index,
                              size_t source_index) {
    for (size_t index = 0; index < count; index++) {
        if (sections[index].object_index == object_index &&
            sections[index].source_index == source_index) {
            return (ptrdiff_t)index;
        }
    }
    return -1;
}

bool zl_link_objects(const byte *const *objects, const size_t *object_sizes, size_t object_count,
                     const char *entry_symbol, byte **output, size_t *output_size,
                     ZlLinkError *error) {
    if (objects == NULL || object_sizes == NULL || object_count == 0 || output == NULL ||
        output_size == NULL) {
        set_error(error, ZL_LINK_ERROR_INVALID_ARGUMENT);
        return false;
    }
    *output = NULL;
    *output_size = 0;
    ZlImage *images = calloc(object_count, sizeof *images);
    if (images == NULL) {
        set_error(error, ZL_LINK_ERROR_OUT_OF_MEMORY);
        return false;
    }
    for (size_t index = 0; index < object_count; index++) {
        ZlImageError image_error;
        if (!zl_image_decode(objects[index], object_sizes[index], &images[index], &image_error) ||
            images[index].kind != ZL_IMAGE_OBJECT) {
            for (size_t cleanup = 0; cleanup <= index; cleanup++) {
                zl_image_free(&images[cleanup]);
            }
            free(images);
            set_error(error, ZL_LINK_ERROR_INCOMPATIBLE_OBJECT);
            return false;
        }
    }
    const char *selected_entry = entry_symbol;
    if (selected_entry == NULL) {
        for (size_t index = 0; index < object_count; index++) {
            if (images[index].entry_symbol != NULL) {
                selected_entry = images[index].entry_symbol;
                break;
            }
        }
        if (selected_entry == NULL) {
            selected_entry = "start";
        }
    }
    size_t section_count = 0;
    for (size_t index = 0; index < object_count; index++) {
        section_count += images[index].section_count;
    }
    LinkSection *sections = calloc(section_count, sizeof *sections);
    if (sections == NULL) {
        set_error(error, ZL_LINK_ERROR_OUT_OF_MEMORY);
        goto cleanup_images;
    }
    size_t section_cursor = 0;
    uint32_t cursor = 8;
    for (size_t object = 0; object < object_count; object++) {
        for (size_t source = 0; source < images[object].section_count; source++) {
            ZlImageSection *input = &images[object].sections[source];
            LinkSection *section = &sections[section_cursor++];
            section->object_index = object;
            section->source_index = source;
            section->image = *input;
            uint32_t address = input->address;
            if (address == 0 && (input->flags & 1u) == 0) {
                if (!align_address(cursor, input->alignment, &address)) {
                    set_error(error, ZL_LINK_ERROR_OVERFLOW);
                    goto cleanup_sections;
                }
            }
            if ((uint64_t)address + input->data_size > ZLVM_ROM_SIZE ||
                overlaps(sections, section_cursor - 1, address, input->data_size)) {
                set_error(error, overlaps(sections, section_cursor - 1, address, input->data_size)
                                     ? ZL_LINK_ERROR_OVERLAP
                                     : ZL_LINK_ERROR_OVERFLOW);
                goto cleanup_sections;
            }
            section->image.address = address;
            cursor = (uint32_t)((uint64_t)address + input->data_size);
        }
    }

    size_t total_symbols = 0;
    for (size_t index = 0; index < object_count; index++) {
        total_symbols += images[index].symbol_count;
    }
    ZlImageSymbol *symbols = calloc(total_symbols, sizeof *symbols);
    if (symbols == NULL) {
        set_error(error, ZL_LINK_ERROR_OUT_OF_MEMORY);
        goto cleanup_sections;
    }
    size_t symbol_count = 0;
    for (size_t object = 0; object < object_count; object++) {
        for (size_t source = 0; source < images[object].symbol_count; source++) {
            ZlImageSymbol symbol = images[object].symbols[source];
            if (symbol.section_index != ZL_IMAGE_UNDEFINED_SECTION) {
                ptrdiff_t section_index =
                    find_section(sections, section_count, object, symbol.section_index);
                if (section_index < 0) {
                    set_error(error, ZL_LINK_ERROR_INCOMPATIBLE_OBJECT);
                    free(symbols);
                    goto cleanup_sections;
                }
                symbol.value += sections[section_index].image.address;
                symbol.section_index = (uint32_t)section_index;
            }
            if (symbol.binding == ZL_IMAGE_SYMBOL_GLOBAL) {
                for (size_t existing = 0; existing < symbol_count; existing++) {
                    if (symbols[existing].binding == ZL_IMAGE_SYMBOL_GLOBAL &&
                        strcmp(symbols[existing].name, symbol.name) == 0 &&
                        symbols[existing].section_index != ZL_IMAGE_UNDEFINED_SECTION &&
                        symbol.section_index != ZL_IMAGE_UNDEFINED_SECTION) {
                        set_error(error, ZL_LINK_ERROR_DUPLICATE_SYMBOL);
                        free(symbols);
                        goto cleanup_sections;
                    }
                }
            }
            symbols[symbol_count++] = symbol;
        }
    }
    for (size_t object = 0; object < object_count; object++) {
        for (size_t index = 0; index < images[object].relocation_count; index++) {
            ZlImageRelocation *relocation = &images[object].relocations[index];
            ptrdiff_t destination =
                find_section(sections, section_count, object, relocation->section_index);
            if (destination < 0) {
                set_error(error, ZL_LINK_ERROR_INCOMPATIBLE_OBJECT);
                free(symbols);
                goto cleanup_sections;
            }
            ZlImageSymbol *reference = &images[object].symbols[relocation->symbol_index];
            int64_t value;
            if (reference->section_index != ZL_IMAGE_UNDEFINED_SECTION) {
                ptrdiff_t target_section =
                    find_section(sections, section_count, object, reference->section_index);
                if (target_section < 0) {
                    set_error(error, ZL_LINK_ERROR_INCOMPATIBLE_OBJECT);
                    free(symbols);
                    goto cleanup_sections;
                }
                value = (int64_t)sections[target_section].image.address + reference->value;
            } else {
                ptrdiff_t value_index = -1;
                for (size_t symbol = 0; symbol < symbol_count; symbol++) {
                    if (strcmp(symbols[symbol].name, reference->name) == 0 &&
                        symbols[symbol].binding == ZL_IMAGE_SYMBOL_GLOBAL &&
                        symbols[symbol].section_index != ZL_IMAGE_UNDEFINED_SECTION) {
                        value_index = (ptrdiff_t)symbol;
                        break;
                    }
                }
                if (value_index < 0) {
                    set_error(error, ZL_LINK_ERROR_UNRESOLVED_SYMBOL);
                    free(symbols);
                    goto cleanup_sections;
                }
                value = symbols[value_index].value;
            }
            uint64_t place = (uint64_t)sections[destination].image.address + relocation->offset;
            value += relocation->addend;
            if (relocation->type == ZL_IMAGE_RELOCATION_PC_RELATIVE32) {
                value -= (int64_t)place;
                if (value < INT32_MIN || value > INT32_MAX) {
                    set_error(error, ZL_LINK_ERROR_OVERFLOW);
                    free(symbols);
                    goto cleanup_sections;
                }
            } else if (value < 0 || (uint64_t)value > UINT32_MAX) {
                set_error(error, ZL_LINK_ERROR_OVERFLOW);
                free(symbols);
                goto cleanup_sections;
            }
            if (relocation->offset > sections[destination].image.data_size ||
                sections[destination].image.data_size - relocation->offset < 4) {
                set_error(error, ZL_LINK_ERROR_OVERFLOW);
                free(symbols);
                goto cleanup_sections;
            }
            write32((byte *)sections[destination].image.data + relocation->offset, (uint32_t)value);
        }
    }
    uint32_t entry = 0;
    for (size_t index = 0; index < symbol_count; index++) {
        if (symbols[index].section_index != ZL_IMAGE_UNDEFINED_SECTION &&
            strcmp(symbols[index].name, selected_entry) == 0) {
            entry = symbols[index].value;
            break;
        }
    }
    if (entry == 0) {
        set_error(error, ZL_LINK_ERROR_NO_ENTRY);
        free(symbols);
        goto cleanup_sections;
    }
    ZlImageSection *output_sections = calloc(section_count, sizeof *output_sections);
    if (output_sections == NULL) {
        set_error(error, ZL_LINK_ERROR_OUT_OF_MEMORY);
        free(symbols);
        goto cleanup_sections;
    }
    for (size_t index = 0; index < section_count; index++) {
        output_sections[index] = sections[index].image;
    }
    ZlImage executable = {.kind = ZL_IMAGE_EXECUTABLE,
                          .entry_point = entry,
                          .sections = output_sections,
                          .section_count = section_count};
    ZlImageError image_error;
    bool success = zl_image_encode(&executable, output, output_size, &image_error);
    free(output_sections);
    free(symbols);
    if (!success) {
        set_error(error, image_error == ZL_IMAGE_ERROR_OVERFLOW
                             ? ZL_LINK_ERROR_OVERFLOW
                             : ZL_LINK_ERROR_INCOMPATIBLE_OBJECT);
        goto cleanup_sections;
    }
    set_error(error, ZL_LINK_ERROR_NONE);

cleanup_sections:
    free(sections);
cleanup_images:
    for (size_t index = 0; index < object_count; index++) {
        zl_image_free(&images[index]);
    }
    free(images);
    return *output != NULL;
}
