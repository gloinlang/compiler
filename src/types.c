#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Global struct registry
StructType *registered_structs = NULL;
int struct_count = 0;
int next_struct_type_id = TYPE_STRUCT_START;

// Type information table
static const Type type_table[] = {
    // kind,        size, signed, numeric, comparable, ordered, is_pointer, pointed_to
    {TYPE_VOID,     0,    0,      0,       0,          0,       0,         NULL},
    {TYPE_BOOL,     1,    0,      0,       1,          0,       0,         NULL},
    {TYPE_I8,       1,    1,      1,       1,          1,       0,         NULL},
    {TYPE_I16,      2,    1,      1,       1,          1,       0,         NULL},
    {TYPE_I32,      4,    1,      1,       1,          1,       0,         NULL},
    {TYPE_I64,      8,    1,      1,       1,          1,       0,         NULL},
    {TYPE_U8,       1,    0,      1,       1,          1,       0,         NULL},
    {TYPE_U16,      2,    0,      1,       1,          1,       0,         NULL},
    {TYPE_U32,      4,    0,      1,       1,          1,       0,         NULL},
    {TYPE_U64,      8,    0,      1,       1,          1,       0,         NULL},
    {TYPE_F32,      4,    1,      1,       1,          1,       0,         NULL},
    {TYPE_F64,      8,    1,      1,       1,          1,       0,         NULL},
    {TYPE_CHAR,     1,    0,      0,       1,          1,       0,         NULL},
    {TYPE_STRING,   8,    0,      0,       1,          0,       0,         NULL}  // String comparisons only ==, !=
};

// Pointer types are handled dynamically
static const int POINTER_SIZE = 8;  // 64-bit pointers

const Type* get_type_info(TypeKind kind) {
    // Handle basic types
    if (kind >= TYPE_VOID && kind <= TYPE_STRING) {
        return &type_table[kind];
    }
    
    // Handle pointer types dynamically
    if (is_pointer_type(kind)) {
        static Type pointer_type_info = {0, POINTER_SIZE, 0, 0, 1, 0, 1, NULL};
        pointer_type_info.kind = kind;
        return &pointer_type_info;
    }
    
    // Unknown type
    static Type unknown_type = {TYPE_UNKNOWN, 0, 0, 0, 0, 0, 0, NULL};
    return &unknown_type;
}

const char* type_to_string(TypeKind kind) {
    switch (kind) {
        case TYPE_VOID: return "void";
        case TYPE_BOOL: return "bool";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_CHAR: return "char";
        case TYPE_STRING: return "string";
        // Pointer types
        case TYPE_PTR_VOID: return "*void";
        case TYPE_PTR_BOOL: return "*bool";
        case TYPE_PTR_I8: return "*i8";
        case TYPE_PTR_I16: return "*i16";
        case TYPE_PTR_I32: return "*i32";
        case TYPE_PTR_I64: return "*i64";
        case TYPE_PTR_U8: return "*u8";
        case TYPE_PTR_U16: return "*u16";
        case TYPE_PTR_U32: return "*u32";
        case TYPE_PTR_U64: return "*u64";
        case TYPE_PTR_F32: return "*f32";
        case TYPE_PTR_F64: return "*f64";
        case TYPE_PTR_CHAR: return "*char";
        case TYPE_PTR_STRING: return "*string";
        default: 
            // Check if it's a struct type
            if (is_struct_type(kind)) {
                StructType *st = get_struct_type(kind);
                return st ? st->name : "unknown_struct";
            }
            return "unknown";
    }
}

TypeKind string_to_type(const char* type_str) {
    // Handle pointer types
    if (type_str[0] == '*') {
        const char* base_type_str = type_str + 1;  // Skip the '*'
        TypeKind base_type = string_to_type(base_type_str);
        return make_pointer_type(base_type);
    }
    
    // Handle basic types
    if (strcmp(type_str, "void") == 0) return TYPE_VOID;
    if (strcmp(type_str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(type_str, "i8") == 0) return TYPE_I8;
    if (strcmp(type_str, "i16") == 0) return TYPE_I16;
    if (strcmp(type_str, "i32") == 0) return TYPE_I32;
    if (strcmp(type_str, "i64") == 0) return TYPE_I64;
    if (strcmp(type_str, "u8") == 0) return TYPE_U8;
    if (strcmp(type_str, "u16") == 0) return TYPE_U16;
    if (strcmp(type_str, "u32") == 0) return TYPE_U32;
    if (strcmp(type_str, "u64") == 0) return TYPE_U64;
    if (strcmp(type_str, "f32") == 0) return TYPE_F32;
    if (strcmp(type_str, "f64") == 0) return TYPE_F64;
    if (strcmp(type_str, "char") == 0) return TYPE_CHAR;
    if (strcmp(type_str, "string") == 0) return TYPE_STRING;
    
    // Check if it's a struct type
    StructType *st = find_struct_by_name(type_str);
    if (st) return st->type_id;
    
    return TYPE_UNKNOWN;
}

int types_compatible(TypeKind left, TypeKind right) {
    // Exact match
    if (left == right) return 1;
    
    // Numeric types can be compatible with implicit conversion
    const Type* left_info = get_type_info(left);
    const Type* right_info = get_type_info(right);
    
    // Both must be numeric for compatibility
    if (!left_info->is_numeric || !right_info->is_numeric) {
        return 0;
    }
    
    // For now, require exact type match for strict type checking
    // Later we could add implicit conversions
    return 0;
}

int types_comparable(TypeKind left, TypeKind right) {
    // Must be the same type to compare
    if (left != right) return 0;
    
    const Type* type_info = get_type_info(left);
    return type_info->is_comparable;
}

int can_convert(TypeKind from, TypeKind to) {
    // Exact match
    if (from == to) return 1;
    
    const Type* from_info = get_type_info(from);
    const Type* to_info = get_type_info(to);
    
    // Can convert between numeric types (for now, require explicit)
    if (from_info->is_numeric && to_info->is_numeric) {
        return 0; // Require explicit conversion for now
    }
    
    return 0;
}

TypeKind get_binary_result_type(TypeKind left, TypeKind right, int is_comparison) {
    if (is_comparison) {
        // All comparison operations return bool
        return TYPE_BOOL;
    }
    
    // For arithmetic operations, both operands must be the same type
    if (left == right) {
        const Type* type_info = get_type_info(left);
        if (type_info->is_numeric) {
            return left;
        }
    }
    
    return TYPE_UNKNOWN;
}

int is_integer_type(TypeKind type) {
    return type >= TYPE_I8 && type <= TYPE_U64;
}

int is_unsigned_type(TypeKind type) {
    return type >= TYPE_U8 && type <= TYPE_U64;
}

int is_signed_type(TypeKind type) {
    return (type >= TYPE_I8 && type <= TYPE_I64) || type == TYPE_F32 || type == TYPE_F64;
}

int is_floating_type(TypeKind type) {
    return type == TYPE_F32 || type == TYPE_F64;
}

int is_numeric_type(TypeKind type) {
    const Type* type_info = get_type_info(type);
    return type_info->is_numeric;
}

// Pointer type functions
int is_pointer_type(TypeKind type) {
    return type >= TYPE_PTR_VOID && type <= TYPE_PTR_STRING;
}

TypeKind make_pointer_type(TypeKind base_type) {
    switch (base_type) {
        case TYPE_VOID: return TYPE_PTR_VOID;
        case TYPE_BOOL: return TYPE_PTR_BOOL;
        case TYPE_I8: return TYPE_PTR_I8;
        case TYPE_I16: return TYPE_PTR_I16;
        case TYPE_I32: return TYPE_PTR_I32;
        case TYPE_I64: return TYPE_PTR_I64;
        case TYPE_U8: return TYPE_PTR_U8;
        case TYPE_U16: return TYPE_PTR_U16;
        case TYPE_U32: return TYPE_PTR_U32;
        case TYPE_U64: return TYPE_PTR_U64;
        case TYPE_F32: return TYPE_PTR_F32;
        case TYPE_F64: return TYPE_PTR_F64;
        case TYPE_CHAR: return TYPE_PTR_CHAR;
        case TYPE_STRING: return TYPE_PTR_STRING;
        default: return TYPE_UNKNOWN;
    }
}

TypeKind get_pointed_type(TypeKind pointer_type) {
    switch (pointer_type) {
        case TYPE_PTR_VOID: return TYPE_VOID;
        case TYPE_PTR_BOOL: return TYPE_BOOL;
        case TYPE_PTR_I8: return TYPE_I8;
        case TYPE_PTR_I16: return TYPE_I16;
        case TYPE_PTR_I32: return TYPE_I32;
        case TYPE_PTR_I64: return TYPE_I64;
        case TYPE_PTR_U8: return TYPE_U8;
        case TYPE_PTR_U16: return TYPE_U16;
        case TYPE_PTR_U32: return TYPE_U32;
        case TYPE_PTR_U64: return TYPE_U64;
        case TYPE_PTR_F32: return TYPE_F32;
        case TYPE_PTR_F64: return TYPE_F64;
        case TYPE_PTR_CHAR: return TYPE_CHAR;
        case TYPE_PTR_STRING: return TYPE_STRING;
        default: return TYPE_UNKNOWN;
    }
}

const char* pointer_type_to_string(TypeKind base_type) {
    TypeKind ptr_type = make_pointer_type(base_type);
    return type_to_string(ptr_type);
}

// Struct type functions
TypeKind register_struct_type(const char *name, StructField *fields, int field_count) {
    // Allocate new struct type
    registered_structs = realloc(registered_structs, (struct_count + 1) * sizeof(StructType));
    
    StructType *new_struct = &registered_structs[struct_count];
    new_struct->name = strdup(name);
    new_struct->type_id = next_struct_type_id++;
    new_struct->field_count = field_count;
    new_struct->fields = malloc(field_count * sizeof(StructField));
    
    // Calculate field offsets and total size
    int offset = 0;
    for (int i = 0; i < field_count; i++) {
        new_struct->fields[i].name = strdup(fields[i].name);
        new_struct->fields[i].type = fields[i].type;
        new_struct->fields[i].offset = offset;
        
        // Get size of field type
        const Type *field_type = get_type_info(fields[i].type);
        int field_size = field_type ? field_type->size : 8; // Default to 8 for unknown types
        offset += field_size;
    }
    
    new_struct->total_size = offset;
    struct_count++;
    
    return new_struct->type_id;
}

StructType* get_struct_type(TypeKind type_id) {
    for (int i = 0; i < struct_count; i++) {
        if (registered_structs[i].type_id == type_id) {
            return &registered_structs[i];
        }
    }
    return NULL;
}

StructType* find_struct_by_name(const char *name) {
    for (int i = 0; i < struct_count; i++) {
        if (strcmp(registered_structs[i].name, name) == 0) {
            return &registered_structs[i];
        }
    }
    return NULL;
}

int get_field_offset(TypeKind struct_type, const char *field_name) {
    StructType *st = get_struct_type(struct_type);
    if (!st) return -1;
    
    for (int i = 0; i < st->field_count; i++) {
        if (strcmp(st->fields[i].name, field_name) == 0) {
            return st->fields[i].offset;
        }
    }
    return -1;
}

TypeKind get_field_type(TypeKind struct_type, const char *field_name) {
    StructType *st = get_struct_type(struct_type);
    if (!st) return TYPE_UNKNOWN;
    
    for (int i = 0; i < st->field_count; i++) {
        if (strcmp(st->fields[i].name, field_name) == 0) {
            return st->fields[i].type;
        }
    }
    return TYPE_UNKNOWN;
}

int is_struct_type(TypeKind type) {
    return type >= TYPE_STRUCT_START && type < TYPE_UNKNOWN;
}
