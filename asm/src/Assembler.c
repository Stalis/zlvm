// Created by Stanislav on 2019-06-01.
//

#include "Assembler.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "Image.h"
#include "Instruction.h"
#include "Memory.h"
#include "Registers.h"
#include "VirtualMachine.h"

static void line_to_upper(char *line) {
    for (char *character = line; *character != '\0'; character++) {
        *character = (char)toupper((unsigned char)*character);
    }
}

static Condition parse_condition(const Token *token);
static Register parse_register(const Token *token);
static void validate_operands(Opcode opcode, Statement *statement);
static void validate_directive(const Directive *directive);
static size_t select_section(AssemblerContext *context, const char *name);
static void record_line(AssemblerContext *context, LineList **last, Line *line);
static bool is_supported_section(const char *name);
static void validate_section_range(const AsmSection *section, const Token *token);

void asm_init(AssemblerContext *context) {
    context->entry = NULL;
    context->externals = NULL;
    context->externalsCount = 0;
    context->globals = NULL;
    context->globalsCount = 0;
    context->labels = NULL;
    context->lines = NULL;
    context->sections = NULL;
    context->section_count = 0;
    context->current_section = 0;
    select_section(context, "text");
}

static const char *const ASM_CONTEXT_DELIMITER = ".";

static const char *set_label_context(const char *context, const char *label);

void asm_processDirectives(AssemblerContext *context, ParserContext *parser) {
    LineList *last = NULL;
    LineStream *stream = lineStream_new(parser->lines);
    parser->lines = NULL;
    parser->lines_count = 0;
    Line *line = lineStream_read(stream);
    const char *procedure_context = NULL;

    while (line != NULL) {
        if (procedure_context != NULL && line->label != NULL) {
            line->label = (char *)set_label_context(procedure_context, line->label);
        }
        if (line->type == L_DIR) {
            validate_directive(line->dir);
            if (is_data_directive(line->dir->type)) {
                Directive *dir = line->dir;
                line->type = L_RAW;
                line->raw = asm_calloc(1, sizeof(struct RawData));
                line->raw->data = directive_get_raw_data(dir, &line->raw->size);
                directive_free(dir);
            } else {
                switch (line->dir->type) {
                    case DIR_SECTION:
                        if (!is_supported_section(line->dir->argv[0]->value)) {
                            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR,
                                             "Unsupported section name", line->dir->argv[0]);
                        }
                        context->current_section =
                            select_section(context, line->dir->argv[0]->value);
                        break;
                    case DIR_GLOBAL:
                        asm_addGlobal(context, asm_strdup(line->dir->argv[0]->value));
                        break;
                    case DIR_EXTERN:
                        asm_addExternal(context, asm_strdup(line->dir->argv[0]->value));
                        break;
                    case DIR_ALIGN: {
                        dword alignment = token_get_int_value(line->dir->argv[0]);
                        AsmSection *section = &context->sections[context->current_section];
                        if (alignment == 0 || alignment > UINT32_MAX ||
                            (alignment & (alignment - 1)) != 0) {
                            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR,
                                             "Alignment must be a nonzero power of two",
                                             line->dir->argv[0]);
                        }
                        size_t remainder = section->size & ((size_t)alignment - 1);
                        if (remainder != 0) {
                            size_t padding = (size_t)alignment - remainder;
                            if (padding > SIZE_MAX - section->size) {
                                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE,
                                                 "Section is too large", line->dir->argv[0]);
                            }
                            section->size += padding;
                        }
                        if (section->alignment < alignment) {
                            section->alignment = (uint32_t)alignment;
                        }
                        validate_section_range(section, line->dir->argv[0]);
                    } break;
                    case DIR_ENTRY:
                        context->entry = asm_strdup(line->dir->argv[0]->value);
                        break;
                    case DIR_LOCATE: {
                        dword address = token_get_int_value(line->dir->argv[0]);
                        AsmSection *section = &context->sections[context->current_section];
                        if (address > UINT32_MAX || address < section->address) {
                            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR,
                                             "Section location overlaps existing content",
                                             line->dir->argv[0]);
                        }
                        if (section->size == 0) {
                            section->address = (uint32_t)address;
                            section->flags |= 1u;
                        } else if ((size_t)address - section->address < section->size) {
                            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR,
                                             "Section location overlaps existing content",
                                             line->dir->argv[0]);
                        } else {
                            section->size = (size_t)address - section->address;
                        }
                        validate_section_range(section, line->dir->argv[0]);
                    } break;
                    case DIR_PROC:
                        if (stream->first == NULL || stream->first->value == NULL) {
                            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                             ".proc must be followed by a procedure body",
                                             line->dir->name);
                        }
                        stream->first->value->label = "";
                        procedure_context = line->dir->argv[0]->value;
                        break;
                    case DIR_ENDPROC:
                        procedure_context = NULL;
                        break;
                    case DIR_MACRO:
                        // Macro definitions are removed before parsing.
                        break;
                    case DIR_ENDMACRO:
                        break;
                    default:
                        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, "Invalid directive",
                                         line->dir->name);
                }

                directive_free(line->dir);
                asm_free(line);
                line = NULL;
            }
        }

        if (line != NULL) {
            record_line(context, &last, line);
        }

        line = lineStream_read(stream);
    }
    asm_free(stream);
}

static size_t select_section(AssemblerContext *context, const char *name) {
    for (size_t index = 0; index < context->section_count; index++) {
        if (strcmp(context->sections[index].name, name) == 0) {
            return index;
        }
    }
    context->sections =
        asm_realloc(context->sections, (context->section_count + 1) * sizeof *context->sections);
    AsmSection *section = &context->sections[context->section_count];
    *section = (AsmSection){
        .name = asm_strdup(name),
        .type = (strcmp(name, ".bss") == 0 || strcmp(name, "bss") == 0)     ? ZL_IMAGE_SECTION_BSS
                : (strcmp(name, ".data") == 0 || strcmp(name, "data") == 0) ? ZL_IMAGE_SECTION_DATA
                                                                            : ZL_IMAGE_SECTION_TEXT,
        .alignment = 1};
    return context->section_count++;
}

static bool is_supported_section(const char *name) {
    return strcmp(name, ".text") == 0 || strcmp(name, "text") == 0 || strcmp(name, ".data") == 0 ||
           strcmp(name, "data") == 0 || strcmp(name, ".bss") == 0 || strcmp(name, "bss") == 0;
}

static void validate_section_range(const AsmSection *section, const Token *token) {
    if (section->address > ZLVM_ROM_SIZE || section->size > ZLVM_ROM_SIZE - section->address) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR, "Section content exceeds VM ROM size",
                         token);
    }
}

static void record_line(AssemblerContext *context, LineList **last, Line *line) {
    AsmSection *section = &context->sections[context->current_section];
    line->section_index = context->current_section;
    line->section_offset = section->size;
    line->size = line->type == L_STMT ? ZLVM_INSTRUCTION_SIZE : line->raw->size;
    if (line->size > SIZE_MAX - section->size) {
        ZLASM_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE, "Section is too large");
    }
    section->size += line->size;
    validate_section_range(section, NULL);
    if (context->lines == NULL) {
        context->lines = asm_calloc(1, sizeof(LineList));
    }
    if (*last == NULL) {
        *last = context->lines;
    } else {
        (*last)->next = asm_calloc(1, sizeof(LineList));
        *last = (*last)->next;
    }
    (*last)->value = line;
}

void asm_addGlobal(AssemblerContext *context, const char *symbol) {
    if (context->globals == NULL) {
        context->globals = asm_malloc(sizeof(const char *));
    } else {
        context->globals =
            asm_realloc(context->globals, sizeof(const char *) * (context->globalsCount + 1));
    }
    context->globals[context->globalsCount++] = symbol;
}

void asm_addExternal(AssemblerContext *context, const char *symbol) {
    if (context->externals == NULL) {
        context->externals = asm_malloc(sizeof(const char *));
    } else {
        context->externals =
            asm_realloc(context->externals, sizeof(const char *) * (context->externalsCount + 1));
    }
    context->externals[context->externalsCount++] = symbol;
}

void asm_processLabels(AssemblerContext *context) {
    LineList *last = context->lines;
    context->labels = asm_calloc(1, sizeof(LabelTable));
    size_t address = 0;
    while (last != NULL) {
        if (last->value->label != NULL) {
            if (labelInfo_getIfExist(context->labels, last->value->label) != NULL) {
                ZLASM_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR, "Duplicate label definition");
            }
            labelTable_setOrCreate(context->labels, asm_strdup(last->value->label), address);
        }
        if (last->value->type == L_STMT) {
            address += ZLVM_INSTRUCTION_SIZE;
        } else if (last->value->type == L_RAW) {
            address += last->value->raw->size;
        }
        last = last->next;
    }
}

byte *asm_translate(AssemblerContext *context, size_t *output_size) {
    const size_t growth_size = 1024;
    size_t capacity = growth_size;
    size_t offset = 0;
    byte *result = asm_calloc(capacity, sizeof *result);

    for (LineList *current = context->lines; current != NULL; current = current->next) {
        const byte *data = NULL;
        size_t data_size = 0;
        Instruction instruction = {0};
        byte encoded_instruction[ZLVM_INSTRUCTION_SIZE];

        if (current->value->type == L_STMT) {
            Statement *statement = current->value->stmt;
            line_to_upper(statement->opcode->value);
            instruction.opcode_ = string_to_opcode(statement->opcode->value);
            if (instruction.opcode_ == OPCODE_TOTAL) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_OPCODE, "Unknown opcode",
                                 statement->opcode);
            }
            validate_operands(instruction.opcode_, statement);

            instruction.condition_ =
                statement->cond == NULL ? C_UNCONDITIONAL : parse_condition(statement->cond);
            instruction.register1 =
                statement->reg1 == NULL ? R_ZERO : parse_register(statement->reg1);
            instruction.register2 =
                statement->reg2 == NULL ? R_ZERO : parse_register(statement->reg2);

            if (statement->imm != NULL) {
                if (statement->imm->type == TOK_LABEL_USE) {
                    LabelInfo *label = labelInfo_getIfExist(context->labels, statement->imm->value);
                    if (label == NULL) {
                        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_LABEL, "Unknown label",
                                         statement->imm);
                    }
                    if (label->address > WORD_MAX) {
                        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE,
                                         "Label address exceeds word size", statement->imm);
                    }
                    instruction.immediate = (word)label->address;
                } else if (statement->imm->type == TOK_CHAR_LITERAL) {
                    instruction.immediate = token_get_char_value(statement->imm);
                } else {
                    dword value = token_get_int_value(statement->imm);
                    if (value > WORD_MAX) {
                        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE,
                                         "Immediate exceeds word size", statement->imm);
                    }
                    instruction.immediate = (word)value;
                }
            }

            if (!instruction_encode(encoded_instruction, sizeof encoded_instruction,
                                    &instruction)) {
                ZLASM_FAIL(ZLASM_DIAGNOSTIC_INTERNAL_ERROR, "Instruction encoding failed");
            }
            data = encoded_instruction;
            data_size = ZLVM_INSTRUCTION_SIZE;
        } else if (current->value->type == L_RAW) {
            data = current->value->raw->data;
            data_size = current->value->raw->size;
        } else {
            ZLASM_FAIL(ZLASM_DIAGNOSTIC_INTERNAL_ERROR, "Invalid line type");
        }

        if (data_size > SIZE_MAX - offset) {
            ZLASM_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE, "Assembler output is too large");
        }
        size_t required_size = offset + data_size;
        if (required_size > capacity) {
            while (required_size > capacity) {
                if (capacity > SIZE_MAX - growth_size) {
                    ZLASM_FAIL(ZLASM_DIAGNOSTIC_OUTPUT_TOO_LARGE, "Assembler output is too large");
                }
                capacity += growth_size;
            }
            result = asm_realloc(result, capacity);
        }

        memcpy(result + offset, data, data_size);
        offset = required_size;
    }

    *output_size = offset;
    return result;
}

static ptrdiff_t find_symbol_line(const AssemblerContext *context, const char *name,
                                  size_t section_index) {
    for (LineList *item = context->lines; item != NULL; item = item->next) {
        Line *line = item->value;
        if (line->section_index == section_index && line->label != NULL &&
            strcmp(line->label, name) == 0) {
            return (ptrdiff_t)line->section_offset;
        }
    }
    return -1;
}

static ptrdiff_t find_image_symbol(const ZlImageSymbol *symbols, size_t count, const char *name) {
    for (size_t index = 0; index < count; index++) {
        if (strcmp(symbols[index].name, name) == 0) {
            return (ptrdiff_t)index;
        }
    }
    return -1;
}

byte *asm_translate_object(AssemblerContext *context, size_t *output_size) {
    ZlImage image = {.kind = ZL_IMAGE_OBJECT, .entry_symbol = context->entry};
    image.section_count = context->section_count;
    image.sections = asm_calloc(image.section_count, sizeof *image.sections);
    for (size_t index = 0; index < image.section_count; index++) {
        AsmSection *source = &context->sections[index];
        image.sections[index] = (ZlImageSection){.name = source->name,
                                                 .type = source->type,
                                                 .flags = source->flags,
                                                 .alignment = source->alignment,
                                                 .address = source->address,
                                                 .data_size = source->size};
        if (source->type != ZL_IMAGE_SECTION_BSS) {
            image.sections[index].data = asm_calloc(source->size, sizeof(byte));
        }
    }

    size_t symbol_count = 0;
    for (LineList *item = context->lines; item != NULL; item = item->next) {
        if (item->value->label != NULL &&
            find_image_symbol(image.symbols, symbol_count, item->value->label) < 0) {
            image.symbols = asm_realloc(image.symbols, (symbol_count + 1) * sizeof *image.symbols);
            image.symbols[symbol_count++] = (ZlImageSymbol){
                .name = item->value->label,
                .section_index = item->value->section_index,
                .value = (uint32_t)item->value->section_offset,
                .binding = ZL_IMAGE_SYMBOL_LOCAL,
                .type = item->value->section_index < context->section_count &&
                                context->sections[item->value->section_index].type ==
                                    ZL_IMAGE_SECTION_TEXT
                            ? ZL_IMAGE_SYMBOL_FUNCTION
                            : ZL_IMAGE_SYMBOL_OBJECT};
        }
    }
    for (size_t index = 0; index < context->globalsCount; index++) {
        ssize_t symbol = find_image_symbol(image.symbols, symbol_count, context->globals[index]);
        if (symbol < 0) {
            ZLASM_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR, "Global symbol has no definition");
        }
        image.symbols[symbol].binding = ZL_IMAGE_SYMBOL_GLOBAL;
    }
    for (size_t index = 0; index < context->externalsCount; index++) {
        const char *name = context->externals[index];
        if (find_image_symbol(image.symbols, symbol_count, name) < 0) {
            image.symbols = asm_realloc(image.symbols, (symbol_count + 1) * sizeof *image.symbols);
            image.symbols[symbol_count++] =
                (ZlImageSymbol){.name = name,
                                .section_index = ZL_IMAGE_UNDEFINED_SECTION,
                                .binding = ZL_IMAGE_SYMBOL_GLOBAL};
        }
    }
    image.symbol_count = symbol_count;

    for (LineList *item = context->lines; item != NULL; item = item->next) {
        Line *line = item->value;
        if (line->type == L_RAW) {
            if (image.sections[line->section_index].type == ZL_IMAGE_SECTION_BSS) {
                ZLASM_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR,
                           "Initialized data cannot be emitted in bss");
            }
            memcpy((byte *)image.sections[line->section_index].data + line->section_offset,
                   line->raw->data, line->raw->size);
            continue;
        }
        Statement *statement = line->stmt;
        if (image.sections[line->section_index].type == ZL_IMAGE_SECTION_BSS) {
            ZLASM_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR, "Instructions cannot be emitted in bss");
        }
        line_to_upper(statement->opcode->value);
        Instruction instruction = {
            .opcode_ = string_to_opcode(statement->opcode->value),
            .condition_ =
                statement->cond == NULL ? C_UNCONDITIONAL : parse_condition(statement->cond),
            .register1 = statement->reg1 == NULL ? R_ZERO : parse_register(statement->reg1),
            .register2 = statement->reg2 == NULL ? R_ZERO : parse_register(statement->reg2)};
        validate_operands(instruction.opcode_, statement);
        if (statement->imm != NULL) {
            if (statement->imm->type == TOK_LABEL_USE) {
                ptrdiff_t local =
                    find_symbol_line(context, statement->imm->value, line->section_index);
                ptrdiff_t symbol =
                    find_image_symbol(image.symbols, symbol_count, statement->imm->value);
                if (local >= 0) {
                    instruction.immediate = (word)local;
                } else {
                    if (symbol < 0) {
                        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_LABEL, "Unknown label",
                                         statement->imm);
                    }
                    instruction.immediate = 0;
                    image.relocations =
                        asm_realloc(image.relocations,
                                    (image.relocation_count + 1) * sizeof *image.relocations);
                    image.relocations[image.relocation_count++] = (ZlImageRelocation){
                        (uint32_t)line->section_index, (uint32_t)(line->section_offset + 4),
                        ZL_IMAGE_RELOCATION_ABSOLUTE32, (uint32_t)symbol, 0};
                }
            } else if (statement->imm->type == TOK_CHAR_LITERAL) {
                instruction.immediate = token_get_char_value(statement->imm);
            } else {
                dword value = token_get_int_value(statement->imm);
                if (value > WORD_MAX) {
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_VALUE_OUT_OF_RANGE,
                                     "Immediate exceeds word size", statement->imm);
                }
                instruction.immediate = (word)value;
            }
        }
        byte encoded[ZLVM_INSTRUCTION_SIZE];
        if (!instruction_encode(encoded, sizeof encoded, &instruction)) {
            ZLASM_FAIL(ZLASM_DIAGNOSTIC_INTERNAL_ERROR, "Instruction encoding failed");
        }
        memcpy((byte *)image.sections[line->section_index].data + line->section_offset, encoded,
               sizeof encoded);
    }
    byte *result = NULL;
    ZlImageError error;
    if (!zl_image_encode(&image, &result, output_size, &error)) {
        ZLASM_FAIL(ZLASM_DIAGNOSTIC_SECTION_ERROR, "Unable to encode object image");
    }
    return result;
}

static const char *set_label_context(const char *context, const char *label) {
    if (strlen(label) == 0) {
        return asm_strdup(context);
    }
    size_t size = strlen(context) + strlen(ASM_CONTEXT_DELIMITER) + strlen(label) + 1;
    char *result = asm_malloc(size);
    snprintf(result, size, "%s%s%s", context, ASM_CONTEXT_DELIMITER, label);
    return result;
}

static bool is_integer_token(const Token *token) {
    return token->type == TOK_INT_BIN || token->type == TOK_INT_OCT || token->type == TOK_INT_DEC ||
           token->type == TOK_INT_HEX;
}

static bool is_data_value_token(const Token *token, bool allow_string) {
    return is_integer_token(token) || token->type == TOK_CHAR_LITERAL ||
           (allow_string && token->type == TOK_STRING_LITERAL);
}

static void require_arity(const Directive *directive, size_t expected_count) {
    if (directive->argc != expected_count) {
        const Token *token =
            directive->argc > expected_count ? directive->argv[expected_count] : directive->name;
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, "Invalid directive argument count",
                         token);
    }
}

static void validate_directive(const Directive *directive) {
    switch (directive->type) {
        case DIR_SECTION:
        case DIR_GLOBAL:
        case DIR_ENTRY:
        case DIR_PROC:
        case DIR_MACRO:
            require_arity(directive, 1);
            if (directive->argv[0]->type != TOK_ID) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, "Directive expects a symbol",
                                 directive->argv[0]);
            }
            break;
        case DIR_EXTERN:
            if (directive->argc == 0) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, ".extern expects a symbol",
                                 directive->name);
            }
            if (directive->argv[0]->type != TOK_ID) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                 ".extern first argument must be a symbol", directive->argv[0]);
            }
            break;
        case DIR_ALIGN:
        case DIR_LOCATE:
        case DIR_SPACE:
            require_arity(directive, 1);
            if (!is_integer_token(directive->argv[0])) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, "Directive expects an integer",
                                 directive->argv[0]);
            }
            break;
        case DIR_ASCII:
        case DIR_ASCIIZ:
            if (directive->argc == 0) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                 "Data directive expects at least one value", directive->name);
            }
            for (size_t index = 0; index < directive->argc; index++) {
                if (!is_data_value_token(directive->argv[index], true)) {
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                     "Invalid data directive value", directive->argv[index]);
                }
            }
            break;
        case DIR_BYTE:
        case DIR_HWORD:
        case DIR_WORD:
        case DIR_DWORD:
            if (directive->argc == 0) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                 "Data directive expects at least one value", directive->name);
            }
            for (size_t index = 0; index < directive->argc; index++) {
                if (!is_data_value_token(directive->argv[index], false)) {
                    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE,
                                     "Invalid numeric directive value", directive->argv[index]);
                }
            }
            break;
        case DIR_ENDMACRO:
        case DIR_ENDPROC:
            require_arity(directive, 0);
            break;
        default:
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_DIRECTIVE, "Invalid directive",
                             directive->name);
    }
}

static void validate_operands(Opcode opcode, Statement *statement) {
    int registers = statement->reg1 != NULL ? (statement->reg2 != NULL ? 2 : 1) : 0;
    int expected_registers;
    int expected_immediate;

    switch (opcode) {
        case NOP:
        case POP:
        case DUP:
        case SYSCALL:
        case RET:
            expected_registers = 0;
            expected_immediate = 0;
            break;
        case POPR:
        case PUSHR:
        case NOT:
        case INC:
        case DEC:
            expected_registers = 1;
            expected_immediate = 0;
            break;
        case PUSHI:
        case INT:
        case JMP:
        case JMPAL:
            expected_registers = 0;
            expected_immediate = 1;
            break;
        case MOVR:
        case ADDR:
        case SUBR:
        case MULR:
        case DIVR:
        case MODR:
        case ANDR:
        case ORR:
        case XORR:
        case NANDR:
        case NORR:
        case CMPR:
        case CMPSR:
        case ADDSR:
        case SUBSR:
        case MULSR:
        case DIVSR:
        case MODSR:
            expected_registers = 2;
            expected_immediate = 0;
            break;
        case MOVI:
        case ADDI:
        case SUBI:
        case MULI:
        case DIVI:
        case MODI:
        case ANDI:
        case ORI:
        case XORI:
        case NANDI:
        case NORI:
        case CMPI:
        case CMPSI:
        case ADDSI:
        case SUBSI:
        case MULSI:
        case DIVSI:
        case MODSI:
            expected_registers = 1;
            expected_immediate = 1;
            break;
        case LOADB:
        case STOREB:
        case LOADH:
        case STOREH:
        case LOADW:
        case STOREW:
            expected_registers = 2;
            expected_immediate = -1;
            break;
        default:
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_OPCODE, "Unknown opcode", statement->opcode);
    }

    if (registers != expected_registers ||
        (expected_immediate >= 0 && (statement->imm != NULL) != expected_immediate)) {
        ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_INVALID_OPERANDS, "Invalid operands", statement->opcode);
    }
}

static Condition parse_condition(const Token *token) {
    const char *string = token->value;

#define CHECK(str, code)                                                                           \
    if (strcmp(string, str) == 0) {                                                                \
        return code;                                                                               \
    }

    CHECK("un", C_UNCONDITIONAL);

    CHECK("zs", C_ZERO_SET);
    CHECK("eq", C_ZERO_SET);
    CHECK("zc", C_ZERO_CLEAR);
    CHECK("ne", C_ZERO_CLEAR);

    CHECK("cs", C_CARRY_SET);
    CHECK("hs", C_CARRY_SET); // unsigned higher or same
    CHECK("cc", C_CARRY_CLEAR);
    CHECK("lo", C_CARRY_CLEAR); // unsigned lower

    CHECK("ns", C_NEGATIVE_SET);
    CHECK("mi", C_NEGATIVE_SET);
    CHECK("nc", C_NEGATIVE_CLEAR);
    CHECK("pl", C_NEGATIVE_CLEAR);

    CHECK("vs", C_OVERFLOW_SET);
    CHECK("vc", C_OVERFLOW_CLEAR);

    CHECK("ss", C_SIGNED_SET);
    CHECK("sc", C_SIGNED_CLEAR);

    CHECK("uh", C_UNSIGNED_HIGHER);
    CHECK("ul", C_UNSIGNED_LOWER_OR_SAME);

    CHECK("gt", C_GREATER);
    CHECK("ge", C_GREATER_OR_EQUALS);
    CHECK("lt", C_LESS);
    CHECK("le", C_LESS_OR_EQUALS);

    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_CONDITION, "Unknown condition", token);
#undef CHECK
}

static Register parse_register(const Token *token) {
    const char *string = token->value;
    if (string == NULL) {
        return R_ZERO;
    }

    if (string[0] == 'r') {
        if (string[1] == '\0') {
            ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_REGISTER, "Invalid numeric register", token);
        }

        size_t index = 0;
        for (const char *character = string + 1; *character != '\0'; character++) {
            if (*character < '0' || *character > '9') {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_REGISTER, "Invalid numeric register",
                                 token);
            }
            size_t digit = (size_t)(*character - '0');
            if (index > (R_TOTAL - 1) / 10 || index * 10 + digit >= R_TOTAL) {
                ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_REGISTER, "Invalid numeric register",
                                 token);
            }
            index = index * 10 + digit;
        }
        return (Register)index;
    }

#define CHECK(str, reg)                                                                            \
    if (strcmp(string, str) == 0) {                                                                \
        return reg;                                                                                \
    }

    CHECK("zero", R_ZERO);
    CHECK("at", R_AT);

    CHECK("v0", R_V0);
    CHECK("v1", R_V1);
    CHECK("v2", R_V2);
    CHECK("v3", R_V3);

    CHECK("a0", R_A0);
    CHECK("a1", R_A1);
    CHECK("a2", R_A2);
    CHECK("a3", R_A3);

    CHECK("t0", R_T0);
    CHECK("t1", R_T1);
    CHECK("t2", R_T2);
    CHECK("t3", R_T3);
    CHECK("t4", R_T4);
    CHECK("t5", R_T5);
    CHECK("t6", R_T6);
    CHECK("t7", R_T7);

    CHECK("s0", R_S0);
    CHECK("s1", R_S1);
    CHECK("s2", R_S2);
    CHECK("s3", R_S3);
    CHECK("s4", R_S4);
    CHECK("s5", R_S5);
    CHECK("s6", R_S6);
    CHECK("s7", R_S7);

    CHECK("k0", R_K0);

    CHECK("sc", R_SC);
    CHECK("lp", R_LP);
    CHECK("sp", R_SP);
    CHECK("bp", R_BP);
    CHECK("pc", R_PC);

#undef CHECK
    ZLASM_TOKEN_FAIL(ZLASM_DIAGNOSTIC_UNKNOWN_REGISTER, "Unknown register", token);
}
