#ifndef TYPES_H
#define TYPES_H

// Fundamental type categories
typedef enum {
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_I8,
    TYPE_I16, 
    TYPE_I32,
    TYPE_I64,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_F32,
    TYPE_F64,
    TYPE_CHAR,
    TYPE_STRING,
    // Pointer types (start from a high number to leave room for basic types)
    TYPE_PTR_VOID = 100,
    TYPE_PTR_BOOL,
    TYPE_PTR_I8,
    TYPE_PTR_I16,
    TYPE_PTR_I32,
    TYPE_PTR_I64,
    TYPE_PTR_U8,
    TYPE_PTR_U16,
    TYPE_PTR_U32,
    TYPE_PTR_U64,
    TYPE_PTR_F32,
    TYPE_PTR_F64,
    TYPE_PTR_CHAR,
    TYPE_PTR_STRING,
    // Struct types (start from 200 to leave room for expansion)
    TYPE_STRUCT_START = 200,
    TYPE_UNKNOWN = 255
} TypeKind;

// Type information structure
typedef struct Type {
    TypeKind kind;
    int size;              // Size in bytes
    int is_signed;         // For integer types
    int is_numeric;        // Can participate in arithmetic
    int is_comparable;     // Can be compared with ==, !=
    int is_ordered;        // Can be compared with <, >, <=, >=
    int is_pointer;        // Is this a pointer type
    struct Type *pointed_to; // For pointer types, what they point to
} Type;

// Pointer type structure
typedef struct PointerType {
    TypeKind base_type;    // What the pointer points to
    int indirection_level; // Number of * (for future multi-level pointers)
} PointerType;

// Struct field information
typedef struct StructField {
    char *name;
    TypeKind type;
    int offset;    // Offset in bytes from start of struct
} StructField;

// Struct type information
typedef struct StructType {
    char *name;
    TypeKind type_id;      // Unique type ID for this struct
    StructField *fields;
    int field_count;
    int total_size;        // Total size of struct in bytes
} StructType;

// Global struct registry
extern StructType *registered_structs;
extern int struct_count;
extern int next_struct_type_id;

// Type system functions
const Type* get_type_info(TypeKind kind);
const char* type_to_string(TypeKind kind);
TypeKind string_to_type(const char* type_str);
int types_compatible(TypeKind left, TypeKind right);
int types_comparable(TypeKind left, TypeKind right);
int can_convert(TypeKind from, TypeKind to);
TypeKind get_binary_result_type(TypeKind left, TypeKind right, int is_comparison);

// Pointer type functions
int is_pointer_type(TypeKind type);
TypeKind make_pointer_type(TypeKind base_type);
TypeKind get_pointed_type(TypeKind pointer_type);
const char* pointer_type_to_string(TypeKind base_type);

// Struct type functions
TypeKind register_struct_type(const char *name, StructField *fields, int field_count);
StructType* get_struct_type(TypeKind type_id);
StructType* find_struct_by_name(const char *name);
int get_field_offset(TypeKind struct_type, const char *field_name);
TypeKind get_field_type(TypeKind struct_type, const char *field_name);
int is_struct_type(TypeKind type);

// Type checking utilities
int is_integer_type(TypeKind type);
int is_unsigned_type(TypeKind type);
int is_signed_type(TypeKind type);
int is_floating_type(TypeKind type);
int is_numeric_type(TypeKind type);

#endif
