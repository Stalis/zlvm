#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm/zlasm.h"
#include "src/Memory.h"

static ZlasmResult read_source(const char *path, bool object);
static char *derive_output_path(const char *input_path, bool object);
static void write_binary(const char *path, const byte *data, size_t size);
static void print_diagnostic(const ZlasmDiagnostic *diagnostic);

int main(int argc, char **argv) {
    bool object = false;
    int input_index = 1;
    if (argc > 1 && strcmp(argv[1], "-c") == 0) {
        object = true;
        input_index++;
    }
    if (argc != input_index + 1 && argc != input_index + 3) {
        fprintf(stderr, "Usage: %s [-c] <assembly-file> [-o <output-file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == input_index + 3 && strcmp(argv[input_index + 1], "-o") != 0) {
        fprintf(stderr, "Expected -o before the output path\n");
        return EXIT_FAILURE;
    }

    char *derived_path = NULL;
    const char *output_path = argc == input_index + 3
                                  ? argv[input_index + 2]
                                  : (derived_path = derive_output_path(argv[input_index], object));

    ZlasmResult result = read_source(argv[input_index], object);
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

static ZlasmResult read_source(const char *path, bool object) {
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
    ZlasmResult result =
        object ? zlasm_assemble_object(source, path) : zlasm_assemble(source, path);
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

static char *derive_output_path(const char *input_path, bool object) {
    const char *last_separator = strrchr(input_path, '/');
    const char *last_dot = strrchr(input_path, '.');
    size_t stem_length = strlen(input_path);

    if (last_dot != NULL && (last_separator == NULL || last_dot > last_separator)) {
        stem_length = (size_t)(last_dot - input_path);
    }

    const char *extension = object ? ".zlo" : ".bin";
    char *output_path = asm_malloc(stem_length + strlen(extension) + 1);
    memcpy(output_path, input_path, stem_length);
    strcpy(output_path + stem_length, extension);
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
