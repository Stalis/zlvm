#ifndef ZLVM_LINKER_H
#define ZLVM_LINKER_H

#include "Image.h"

typedef enum ZlLinkError {
    ZL_LINK_ERROR_NONE = 0,
    ZL_LINK_ERROR_INVALID_ARGUMENT,
    ZL_LINK_ERROR_OUT_OF_MEMORY,
    ZL_LINK_ERROR_INCOMPATIBLE_OBJECT,
    ZL_LINK_ERROR_DUPLICATE_SYMBOL,
    ZL_LINK_ERROR_UNRESOLVED_SYMBOL,
    ZL_LINK_ERROR_OVERLAP,
    ZL_LINK_ERROR_OVERFLOW,
    ZL_LINK_ERROR_NO_ENTRY,
} ZlLinkError;

/** Link encoded relocatable objects into a newly allocated encoded executable image.
 *
 * If entry_symbol is NULL, the first object entry symbol is used, falling back to "start".
 */
bool zl_link_objects(const byte *const *objects, const size_t *object_sizes, size_t object_count,
                     const char *entry_symbol, byte **output, size_t *output_size,
                     ZlLinkError *error);

#endif
