#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm/zlasm.h"
#include "src/Memory.h"

static ZlasmResult read_source(const char *path);
static char *derive_output_path(const char *input_path);
static void write_binary(const char *path, const byte *data, size_t size);
static void print_diagnostic(const ZlasmDiagnostic *diagnostic);

int main(int argc, char **argv) {
    if (argc != 2 && argc != 4) {
        fprintf(stderr, "Usage: %s <assembly-file> [-o <output-file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 4 && strcmp(argv[2], "-o") != 0) {
        fprintf(stderr, "Expected -o before the output path\n");
        return EXIT_FAILURE;
    }

    char *derived_path = NULL;
    const char *output_path = argc == 4 ? argv[3] : (derived_path = derive_output_path(argv[1]));

    ZlasmResult result = read_source(argv[1]);
    if (result.diagnostic.code != ZLASM_DIAGNOSTIC_NONE) {
        print_diagnostic(&result.diagnostic);
        asm_free(derived_path);
        return EXIT_FAILURE;
    }
    write_binary(output_path, result.binary, result.binary_size);

    zlasm_result_free(&result);
    asm_free(derived_path);
    return EXIT_SUCCESS;
}

static ZlasmResult read_source(const char *path) {
    const size_t growth_size = 1024;
    size_t capacity = growth_size;
    size_t length = 0;
    char *source = asm_malloc(capacity + 1);

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open assembly source: %s\n", path);
        exit(EXIT_FAILURE);
    }

    while (true) {
        size_t available = capacity - length;
        size_t bytes_read = fread(source + length, 1, available, file);
        length += bytes_read;

        if (bytes_read < available) {
            if (ferror(file)) {
                fprintf(stderr, "Unable to read assembly source: %s\n", path);
                fclose(file);
                asm_free(source);
                exit(EXIT_FAILURE);
            }
            break;
        }

        capacity += growth_size;
        source = asm_realloc(source, capacity + 1);
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Unable to close assembly source: %s\n", path);
        asm_free(source);
        exit(EXIT_FAILURE);
    }

    source[length] = '\0';
    ZlasmResult result = zlasm_assemble(source, path);
    asm_free(source);
    return result;
}

static void print_diagnostic(const ZlasmDiagnostic *diagnostic) {
    if (diagnostic->has_source_location) {
        fprintf(stderr, "%s:%zu:%zu: error ZLASM%04d: %s [bytes %zu..%zu)\n",
                diagnostic->source_filename, diagnostic->line, diagnostic->column,
                (int)diagnostic->code, diagnostic->message, diagnostic->byte_offset,
                diagnostic->byte_offset + diagnostic->byte_length);
    } else {
        fprintf(stderr, "%s: error ZLASM%04d: %s\n", diagnostic->source_filename,
                (int)diagnostic->code, diagnostic->message);
    }
}

static char *derive_output_path(const char *input_path) {
    const char *last_separator = strrchr(input_path, '/');
    const char *last_dot = strrchr(input_path, '.');
    size_t stem_length = strlen(input_path);

    if (last_dot != NULL && (last_separator == NULL || last_dot > last_separator)) {
        stem_length = (size_t)(last_dot - input_path);
    }

    char *output_path = asm_malloc(stem_length + sizeof(".bin"));
    memcpy(output_path, input_path, stem_length);
    memcpy(output_path + stem_length, ".bin", sizeof(".bin"));
    return output_path;
}

static void write_binary(const char *path, const byte *data, size_t size) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open output file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    if (fwrite(data, 1, size, file) != size) {
        fprintf(stderr, "Unable to write output file: %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Unable to close output file: %s\n", path);
        exit(EXIT_FAILURE);
    }
}
