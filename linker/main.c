#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Linker.h"

typedef struct FileBuffer {
    byte *data;
    size_t size;
} FileBuffer;

static FileBuffer read_file(const char *path) {
    FileBuffer result = {0};
    FILE *file = fopen(path, "rb");
    if (file == NULL || fseek(file, 0, SEEK_END) != 0) {
        if (file != NULL) {
            fclose(file);
        }
        return result;
    }
    long size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return result;
    }
    result.size = (size_t)size;
    result.data = malloc(result.size);
    if ((result.size != 0 && result.data == NULL) ||
        (result.size != 0 && fread(result.data, 1, result.size, file) != result.size)) {
        free(result.data);
        result = (FileBuffer){0};
    }
    fclose(file);
    return result;
}

int main(int argc, char **argv) {
    if (argc < 4 || strcmp(argv[1], "-o") != 0) {
        fprintf(stderr, "Usage: %s -o <executable> [--entry <symbol>] <object>...\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *output_path = argv[2];
    const char *entry = NULL;
    int first_object = 3;
    if (first_object + 1 < argc && strcmp(argv[first_object], "--entry") == 0) {
        entry = argv[first_object + 1];
        first_object += 2;
    }
    if (first_object == argc) {
        fprintf(stderr, "No object files supplied\n");
        return EXIT_FAILURE;
    }
    size_t count = (size_t)(argc - first_object);
    FileBuffer *files = calloc(count, sizeof *files);
    const byte **objects = calloc(count, sizeof *objects);
    size_t *sizes = calloc(count, sizeof *sizes);
    if (files == NULL || objects == NULL || sizes == NULL) {
        free(files);
        free(objects);
        free(sizes);
        return EXIT_FAILURE;
    }
    for (size_t index = 0; index < count; index++) {
        files[index] = read_file(argv[first_object + (int)index]);
        if (files[index].data == NULL) {
            fprintf(stderr, "Unable to read object: %s\n", argv[first_object + (int)index]);
            for (size_t cleanup = 0; cleanup <= index; cleanup++) {
                free(files[cleanup].data);
            }
            free(files);
            free(objects);
            free(sizes);
            return EXIT_FAILURE;
        }
        objects[index] = files[index].data;
        sizes[index] = files[index].size;
    }
    byte *encoded = NULL;
    size_t encoded_size = 0;
    ZlLinkError error;
    bool success = zl_link_objects(objects, sizes, count, entry, &encoded, &encoded_size, &error);
    if (!success) {
        fprintf(stderr, "link failed (error %d)\n", (int)error);
    } else {
        FILE *file = fopen(output_path, "wb");
        success = file != NULL && fwrite(encoded, 1, encoded_size, file) == encoded_size &&
                  fclose(file) == 0;
        if (!success) {
            fprintf(stderr, "Unable to write executable: %s\n", output_path);
        }
    }
    free(encoded);
    for (size_t index = 0; index < count; index++) {
        free(files[index].data);
    }
    free(files);
    free(objects);
    free(sizes);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
