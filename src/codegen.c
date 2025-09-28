#define _GNU_SOURCE
#include "codegen.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CodeGen *create_codegen(const char *module_name) {
  CodeGen *codegen = malloc(sizeof(CodeGen));

  // Initialize LLVM
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  // Create context, module, and builder
  codegen->context = LLVMContextCreate();
  codegen->module =
      LLVMModuleCreateWithNameInContext(module_name, codegen->context);
  codegen->builder = LLVMCreateBuilderInContext(codegen->context);
  codegen->current_function = NULL;

  // Initialize symbol tables
  codegen->variable_count = 0;
  codegen->function_count = 0;
  codegen->loop_depth = 0;
  codegen->has_error = 0;

  // Add standard library functions
  add_builtin_functions(codegen);

  return codegen;
}

void add_builtin_functions(CodeGen *codegen) {
  // Add printf function
  LLVMTypeRef printf_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // char*
  };
  LLVMTypeRef printf_type = LLVMFunctionType(
      LLVMInt32TypeInContext(codegen->context), // return type: int
      printf_args, 1, 1                         // variadic function
  );
  LLVMValueRef printf_func =
      LLVMAddFunction(codegen->module, "printf", printf_type);

  // Add puts function
  LLVMTypeRef puts_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // char*
  };
  LLVMTypeRef puts_type = LLVMFunctionType(
      LLVMInt32TypeInContext(codegen->context), // return type: int
      puts_args, 1, 0                           // not variadic
  );
  LLVMValueRef puts_func = LLVMAddFunction(codegen->module, "puts", puts_type);

  // Add strlen function
  LLVMTypeRef strlen_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // char*
  };
  LLVMTypeRef strlen_type = LLVMFunctionType(
      LLVMInt64TypeInContext(codegen->context), // return type: size_t (i64)
      strlen_args, 1, 0                         // not variadic
  );
  LLVMValueRef strlen_func =
      LLVMAddFunction(codegen->module, "strlen", strlen_type);

  // Add scanf function
  LLVMTypeRef scanf_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // char*
  };
  LLVMTypeRef scanf_type = LLVMFunctionType(
      LLVMInt32TypeInContext(codegen->context), // return type: int
      scanf_args, 1, 1                          // variadic function
  );
  LLVMValueRef scanf_func =
      LLVMAddFunction(codegen->module, "scanf", scanf_type);

  // Add getline function
  LLVMTypeRef getline_args[] = {
      LLVMPointerType(
          LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0),
          0),                                                       // char**
      LLVMPointerType(LLVMInt64TypeInContext(codegen->context), 0), // size_t*
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0)   // FILE*
  };
  LLVMTypeRef getline_type = LLVMFunctionType(
      LLVMInt64TypeInContext(codegen->context), // return type: ssize_t (i64)
      getline_args, 3, 0                        // not variadic
  );
  LLVMValueRef getline_func =
      LLVMAddFunction(codegen->module, "getline", getline_type);

  // Add atoi function (string to int)
  LLVMTypeRef atoi_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // char*
  };
  LLVMTypeRef atoi_type = LLVMFunctionType(
      LLVMInt32TypeInContext(codegen->context), // return type: int
      atoi_args, 1, 0                           // not variadic
  );
  LLVMValueRef atoi_func = LLVMAddFunction(codegen->module, "atoi", atoi_type);

  // Add atol function (string to long)
  LLVMTypeRef atol_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // char*
  };
  LLVMTypeRef atol_type = LLVMFunctionType(
      LLVMInt64TypeInContext(codegen->context), // return type: long
      atol_args, 1, 0                           // not variadic
  );
  LLVMValueRef atol_func = LLVMAddFunction(codegen->module, "atol", atol_type);

  // Add sprintf function (for number to string conversion)
  LLVMTypeRef sprintf_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0), // char*
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0)  // char*
  };
  LLVMTypeRef sprintf_type = LLVMFunctionType(
      LLVMInt32TypeInContext(codegen->context), // return type: int
      sprintf_args, 2, 1                        // variadic function
  );
  LLVMValueRef sprintf_func =
      LLVMAddFunction(codegen->module, "sprintf", sprintf_type);

  // Add malloc function
  LLVMTypeRef malloc_args[] = {
      LLVMInt64TypeInContext(codegen->context) // size_t (i64)
  };
  LLVMTypeRef malloc_type = LLVMFunctionType(
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0), // void* return
      malloc_args, 1, 0                                            // not variadic
  );
  LLVMValueRef malloc_func =
      LLVMAddFunction(codegen->module, "malloc", malloc_type);

  // Add free function
  LLVMTypeRef free_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0) // void*
  };
  LLVMTypeRef free_type = LLVMFunctionType(
      LLVMVoidTypeInContext(codegen->context), // void return
      free_args, 1, 0                          // not variadic
  );
  LLVMValueRef free_func = LLVMAddFunction(codegen->module, "free", free_type);

  // Add realloc function  
  LLVMTypeRef realloc_args[] = {
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0), // void* ptr
      LLVMInt64TypeInContext(codegen->context)                     // size_t size
  };
  LLVMTypeRef realloc_type = LLVMFunctionType(
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0), // void* return
      realloc_args, 2, 0                                           // not variadic
  );
  LLVMValueRef realloc_func =
      LLVMAddFunction(codegen->module, "realloc", realloc_type);
}

LLVMValueRef codegen_std_print(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 1) {
    fprintf(stderr, "std.print() expects exactly 1 argument\n");
    return NULL;
  }

  // Get the argument
  LLVMValueRef arg = codegen_expression(codegen, call->data.call.args[0]);
  if (!arg)
    return NULL;

  // Get printf function
  LLVMValueRef printf_func = LLVMGetNamedFunction(codegen->module, "printf");
  if (!printf_func) {
    fprintf(stderr, "printf function not found\n");
    return NULL;
  }

  // Determine the argument type to choose appropriate format string
  TypeKind arg_type = get_expression_type(codegen, call->data.call.args[0]);
  LLVMValueRef format_str;

  switch (arg_type) {
  case TYPE_STRING:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%s", "fmt");
    break;
  case TYPE_I8:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hhd", "fmt");
    break;
  case TYPE_I16:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hd", "fmt");
    break;
  case TYPE_I32:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%d", "fmt");
    break;
  case TYPE_I64:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%ld", "fmt");
    break;
  case TYPE_I128:
    // For now, treat i128 as unsupported (needs custom formatting)
    fprintf(stderr, "i128 printing not yet implemented - needs custom formatting\n");
    return NULL;
  case TYPE_U8:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hhu", "fmt");
    break;
  case TYPE_U16:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hu", "fmt");
    break;
  case TYPE_U32:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%u", "fmt");
    break;
  case TYPE_U64:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%lu", "fmt");
    break;
  case TYPE_U128:
    // For now, treat u128 as unsupported (needs custom formatting)
    fprintf(stderr, "u128 printing not yet implemented - needs custom formatting\n");
    return NULL;
  case TYPE_BOOL:
    // Convert boolean to string representation
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%s", "fmt");
    // We'll need to convert the boolean to a string
    LLVMValueRef true_str =
        LLVMBuildGlobalStringPtr(codegen->builder, "true", "true_str");
    LLVMValueRef false_str =
        LLVMBuildGlobalStringPtr(codegen->builder, "false", "false_str");
    arg =
        LLVMBuildSelect(codegen->builder, arg, true_str, false_str, "bool_str");
    break;
  default:
    fprintf(stderr, "Unsupported type for std.print(): %s\n",
            type_to_string(arg_type));
    return NULL;
  }

  // Create arguments array
  LLVMValueRef args[] = {format_str, arg};

  // Call printf
  LLVMTypeRef printf_type = LLVMGlobalGetValueType(printf_func);
  return LLVMBuildCall2(codegen->builder, printf_type, printf_func, args, 2,
                        "");
}

LLVMValueRef codegen_std_println(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 1) {
    fprintf(stderr, "std.println() expects exactly 1 argument\n");
    return NULL;
  }

  // Get the argument
  LLVMValueRef arg = codegen_expression(codegen, call->data.call.args[0]);
  if (!arg)
    return NULL;

  // Get printf function
  LLVMValueRef printf_func = LLVMGetNamedFunction(codegen->module, "printf");
  if (!printf_func) {
    fprintf(stderr, "printf function not found\n");
    return NULL;
  }

  // Determine the argument type to choose appropriate format string
  TypeKind arg_type = get_expression_type(codegen, call->data.call.args[0]);
  LLVMValueRef format_str;

  switch (arg_type) {
  case TYPE_STRING:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%s\n", "fmt");
    break;
  case TYPE_I8:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hhd\n", "fmt");
    break;
  case TYPE_I16:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hd\n", "fmt");
    break;
  case TYPE_I32:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%d\n", "fmt");
    break;
  case TYPE_I64:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%ld\n", "fmt");
    break;
  case TYPE_I128:
    fprintf(stderr, "i128 printing not yet implemented - needs custom formatting\n");
    return NULL;
  case TYPE_U8:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hhu\n", "fmt");
    break;
  case TYPE_U16:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hu\n", "fmt");
    break;
  case TYPE_U32:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%u\n", "fmt");
    break;
  case TYPE_U64:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%lu\n", "fmt");
    break;
  case TYPE_U128:
    fprintf(stderr, "u128 printing not yet implemented - needs custom formatting\n");
    return NULL;
  case TYPE_BOOL:
    // Convert boolean to string representation
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%s\n", "fmt");
    // We'll need to convert the boolean to a string
    LLVMValueRef true_str =
        LLVMBuildGlobalStringPtr(codegen->builder, "true", "true_str");
    LLVMValueRef false_str =
        LLVMBuildGlobalStringPtr(codegen->builder, "false", "false_str");
    arg =
        LLVMBuildSelect(codegen->builder, arg, true_str, false_str, "bool_str");
    break;
  default:
    fprintf(stderr, "Unsupported type for std.println(): %s\n",
            type_to_string(arg_type));
    return NULL;
  }

  // Create arguments array
  LLVMValueRef args[] = {format_str, arg};

  // Call printf
  LLVMTypeRef printf_type = LLVMGlobalGetValueType(printf_func);
  return LLVMBuildCall2(codegen->builder, printf_type, printf_func, args, 2,
                        "");
}

LLVMValueRef codegen_std_input(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 0) {
    fprintf(stderr, "std.input() expects no arguments\n");
    return NULL;
  }

  // Get scanf function
  LLVMValueRef scanf_func = LLVMGetNamedFunction(codegen->module, "scanf");
  if (!scanf_func) {
    fprintf(stderr, "scanf function not found\n");
    return NULL;
  }

  // Allocate space for the input string (256 characters max)
  LLVMTypeRef char_type = LLVMInt8TypeInContext(codegen->context);
  LLVMTypeRef array_type = LLVMArrayType(char_type, 256);
  LLVMValueRef input_buffer =
      LLVMBuildAlloca(codegen->builder, array_type, "input_buffer");

  // Create format string for scanf "%255s" (prevent buffer overflow)
  LLVMValueRef format_str =
      LLVMBuildGlobalStringPtr(codegen->builder, "%255s", "fmt");

  // Cast array to char* for scanf
  LLVMValueRef buffer_ptr =
      LLVMBuildBitCast(codegen->builder, input_buffer,
                       LLVMPointerType(char_type, 0), "buffer_ptr");

  // Create arguments array
  LLVMValueRef args[] = {format_str, buffer_ptr};

  // Call scanf
  LLVMTypeRef scanf_type = LLVMGlobalGetValueType(scanf_func);
  LLVMBuildCall2(codegen->builder, scanf_type, scanf_func, args, 2, "");

  // Return the string pointer
  return buffer_ptr;
}

LLVMValueRef codegen_std_readln(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 0) {
    fprintf(stderr, "std.readln() expects no arguments\n");
    return NULL;
  }

  // Get getline function
  LLVMValueRef getline_func = LLVMGetNamedFunction(codegen->module, "getline");
  if (!getline_func) {
    fprintf(stderr, "getline function not found\n");
    return NULL;
  }

  // Allocate variables for getline
  LLVMTypeRef char_ptr_type =
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0);
  LLVMTypeRef char_ptr_ptr_type = LLVMPointerType(char_ptr_type, 0);
  LLVMTypeRef size_t_type = LLVMInt64TypeInContext(codegen->context);
  LLVMTypeRef size_t_ptr_type = LLVMPointerType(size_t_type, 0);

  // Allocate space for line pointer and size
  LLVMValueRef line_ptr =
      LLVMBuildAlloca(codegen->builder, char_ptr_type, "line_ptr");
  LLVMValueRef size_ptr =
      LLVMBuildAlloca(codegen->builder, size_t_type, "size_ptr");

  // Initialize line pointer to NULL and size to 0
  LLVMBuildStore(codegen->builder, LLVMConstNull(char_ptr_type), line_ptr);
  LLVMBuildStore(codegen->builder, LLVMConstInt(size_t_type, 0, 0), size_ptr);

  // Get stdin - we need to declare it as an external global
  LLVMValueRef stdin_ptr = LLVMGetNamedGlobal(codegen->module, "stdin");
  if (!stdin_ptr) {
    // Add stdin as external global if not already declared
    LLVMTypeRef file_ptr_type =
        LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0);
    stdin_ptr = LLVMAddGlobal(codegen->module, file_ptr_type, "stdin");
    LLVMSetLinkage(stdin_ptr, LLVMExternalLinkage);
  }
  LLVMValueRef stdin_val = LLVMBuildLoad2(
      codegen->builder,
      LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0), stdin_ptr,
      "stdin_val");

  // Create arguments array
  LLVMValueRef args[] = {line_ptr, size_ptr, stdin_val};

  // Call getline
  LLVMTypeRef getline_type = LLVMGlobalGetValueType(getline_func);
  LLVMBuildCall2(codegen->builder, getline_type, getline_func, args, 3, "");

  // Load and return the line pointer
  return LLVMBuildLoad2(codegen->builder, char_ptr_type, line_ptr, "line");
}

LLVMValueRef codegen_std_to_int(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 1) {
    fprintf(stderr, "std.to_int() expects exactly 1 argument\n");
    return NULL;
  }

  // Get the argument (should be a string)
  LLVMValueRef arg = codegen_expression(codegen, call->data.call.args[0]);
  if (!arg)
    return NULL;

  // Get atoi function
  LLVMValueRef atoi_func = LLVMGetNamedFunction(codegen->module, "atoi");
  if (!atoi_func) {
    fprintf(stderr, "atoi function not found\n");
    return NULL;
  }

  // Call atoi
  LLVMTypeRef atoi_type = LLVMGlobalGetValueType(atoi_func);
  return LLVMBuildCall2(codegen->builder, atoi_type, atoi_func, &arg, 1, "");
}

LLVMValueRef codegen_std_to_i64(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 1) {
    fprintf(stderr, "std.to_i64() expects exactly 1 argument\n");
    return NULL;
  }

  // Get the argument (should be a string)
  LLVMValueRef arg = codegen_expression(codegen, call->data.call.args[0]);
  if (!arg)
    return NULL;

  // Get atol function
  LLVMValueRef atol_func = LLVMGetNamedFunction(codegen->module, "atol");
  if (!atol_func) {
    fprintf(stderr, "atol function not found\n");
    return NULL;
  }

  // Call atol
  LLVMTypeRef atol_type = LLVMGlobalGetValueType(atol_func);
  return LLVMBuildCall2(codegen->builder, atol_type, atol_func, &arg, 1, "");
}

LLVMValueRef codegen_std_to_string(CodeGen *codegen, ASTNode *call) {
  if (call->data.call.arg_count != 1) {
    fprintf(stderr, "std.to_string() expects exactly 1 argument\n");
    return NULL;
  }

  // Get the argument
  LLVMValueRef arg = codegen_expression(codegen, call->data.call.args[0]);
  if (!arg)
    return NULL;

  // Get sprintf function
  LLVMValueRef sprintf_func = LLVMGetNamedFunction(codegen->module, "sprintf");
  if (!sprintf_func) {
    fprintf(stderr, "sprintf function not found\n");
    return NULL;
  }

  // Allocate buffer for the result string (32 chars should be enough for
  // numbers)
  LLVMTypeRef char_type = LLVMInt8TypeInContext(codegen->context);
  LLVMTypeRef array_type = LLVMArrayType(char_type, 32);
  LLVMValueRef buffer =
      LLVMBuildAlloca(codegen->builder, array_type, "str_buffer");
  LLVMValueRef buffer_ptr = LLVMBuildBitCast(
      codegen->builder, buffer, LLVMPointerType(char_type, 0), "buffer_ptr");

  // Determine the argument type to choose appropriate format string
  TypeKind arg_type = get_expression_type(codegen, call->data.call.args[0]);
  LLVMValueRef format_str;
  LLVMValueRef final_arg = arg;

  switch (arg_type) {
  case TYPE_I8:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hhd", "fmt");
    break;
  case TYPE_I16:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hd", "fmt");
    break;
  case TYPE_I32:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%d", "fmt");
    break;
  case TYPE_I64:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%ld", "fmt");
    break;
  case TYPE_I128:
    fprintf(stderr, "i128 to_string not yet implemented - needs custom formatting\n");
    return NULL;
  case TYPE_U8:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hhu", "fmt");
    break;
  case TYPE_U16:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%hu", "fmt");
    break;
  case TYPE_U32:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%u", "fmt");
    break;
  case TYPE_U64:
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%lu", "fmt");
    break;
  case TYPE_U128:
    fprintf(stderr, "u128 to_string not yet implemented - needs custom formatting\n");
    return NULL;
  case TYPE_BOOL:
    // Convert boolean to string representation
    format_str = LLVMBuildGlobalStringPtr(codegen->builder, "%s", "fmt");
    // We'll need to convert the boolean to a string
    LLVMValueRef true_str =
        LLVMBuildGlobalStringPtr(codegen->builder, "true", "true_str");
    LLVMValueRef false_str =
        LLVMBuildGlobalStringPtr(codegen->builder, "false", "false_str");
    final_arg =
        LLVMBuildSelect(codegen->builder, arg, true_str, false_str, "bool_str");
    break;
  case TYPE_STRING:
    // If already a string, just return it
    return arg;
  default:
    fprintf(stderr, "Unsupported type for std.to_string(): %s\n",
            type_to_string(arg_type));
    return NULL;
  }

  // Create arguments array
  LLVMValueRef args[] = {buffer_ptr, format_str, final_arg};

  // Call sprintf
  LLVMTypeRef sprintf_type = LLVMGlobalGetValueType(sprintf_func);
  LLVMBuildCall2(codegen->builder, sprintf_type, sprintf_func, args, 3, "");

  // Return the buffer pointer
  return buffer_ptr;
}

LLVMValueRef codegen_cast(CodeGen *codegen, ASTNode *call) {
    if (call->data.call.arg_count != 2) {
        fprintf(stderr, "cast() expects exactly 2 arguments: cast(value, target_type)\n");
        return NULL;
    }

    // Get the value to cast
    LLVMValueRef value = codegen_expression(codegen, call->data.call.args[0]);
    if (!value) {
        return NULL;
    }

    // Get the source type
    TypeKind src_type = get_expression_type(codegen, call->data.call.args[0]);
    
    // Get the target type from the second argument (should be a string literal with type name)
    ASTNode *target_type_node = call->data.call.args[1];
    const char *target_type_name = NULL;
    
    if (target_type_node->type == NODE_IDENTIFIER) {
        target_type_name = target_type_node->data.identifier.name;
    } else if (target_type_node->type == NODE_LITERAL && 
               strcmp(target_type_node->data.literal.type, "string") == 0) {
        target_type_name = target_type_node->data.literal.value;
    } else {
        fprintf(stderr, "cast() second argument must be a type name (identifier or string)\n");
        return NULL;
    }
    TypeKind target_type = string_to_type(target_type_name);
    
    if (target_type == TYPE_UNKNOWN) {
        fprintf(stderr, "cast(): unknown target type '%s'\n", target_type_name);
        return NULL;
    }

    // Get LLVM types
    LLVMTypeRef src_llvm_type = get_llvm_type_from_kind(codegen, src_type);
    LLVMTypeRef target_llvm_type = get_llvm_type_from_kind(codegen, target_type);
    
    if (!src_llvm_type || !target_llvm_type) {
        fprintf(stderr, "cast(): failed to get LLVM types\n");
        return NULL;
    }

    // Perform the cast based on the types
    const Type *src_info = get_type_info(src_type);
    const Type *target_info = get_type_info(target_type);
    
    if (!src_info || !target_info) {
        fprintf(stderr, "cast(): failed to get type information\n");
        return NULL;
    }

    // Same types - no cast needed
    if (src_type == target_type) {
        return value;
    }

    // Integer to integer casts
    if (src_info->is_numeric && target_info->is_numeric &&
        !strcmp(type_to_string(src_type), type_to_string(target_type)) == 0) {
        
        // Check if it's a widening or narrowing conversion
        if (src_info->size < target_info->size) {
            // Widening conversion - safe
            if (src_info->is_signed && target_info->is_signed) {
                // Signed extension
                return LLVMBuildSExt(codegen->builder, value, target_llvm_type, "cast_sext");
            } else if (!src_info->is_signed && !target_info->is_signed) {
                // Zero extension
                return LLVMBuildZExt(codegen->builder, value, target_llvm_type, "cast_zext");
            } else {
                // Signed/unsigned conversion with extension
                return LLVMBuildIntCast(codegen->builder, value, target_llvm_type, "cast_int");
            }
        } else if (src_info->size > target_info->size) {
            // Narrowing conversion - potential runtime overflow
            return LLVMBuildTrunc(codegen->builder, value, target_llvm_type, "cast_trunc");
        } else {
            // Same size, different signedness
            return LLVMBuildBitCast(codegen->builder, value, target_llvm_type, "cast_bitcast");
        }
    }

    // Pointer casts (between different pointer types or integer to pointer)
    if (is_pointer_type(target_type) || is_pointer_type(src_type)) {
        // Allow any pointer to pointer cast (like C void* casting)
        return LLVMBuildBitCast(codegen->builder, value, target_llvm_type, "cast_ptr");
    }

    // Integer to pointer cast (for null pointers, addresses, etc.)
    if (src_info->is_numeric && is_pointer_type(target_type)) {
        return LLVMBuildIntToPtr(codegen->builder, value, target_llvm_type, "cast_int_to_ptr");
    }

    // Pointer to integer cast
    if (is_pointer_type(src_type) && target_info->is_numeric) {
        return LLVMBuildPtrToInt(codegen->builder, value, target_llvm_type, "cast_ptr_to_int");
    }

    // For now, reject other conversions
    fprintf(stderr, "cast(): conversion from %s to %s not yet supported\n",
            type_to_string(src_type), target_type_name);
    return NULL;
}

LLVMValueRef codegen_std_malloc(CodeGen *codegen, ASTNode *call) {
    if (call->data.call.arg_count != 1) {
        fprintf(stderr, "std.malloc() expects exactly 1 argument (size)\n");
        return NULL;
    }

    // Get the size argument
    LLVMValueRef size_arg = codegen_expression(codegen, call->data.call.args[0]);
    if (!size_arg) {
        return NULL;
    }

    // Get malloc function
    LLVMValueRef malloc_func = LLVMGetNamedFunction(codegen->module, "malloc");
    if (!malloc_func) {
        fprintf(stderr, "malloc function not found\n");
        return NULL;
    }

    // Ensure size is i64 (size_t)
    TypeKind size_type = get_expression_type(codegen, call->data.call.args[0]);
    if (size_type != TYPE_I64) {
        // Cast to i64 if necessary
        LLVMTypeRef i64_type = LLVMInt64TypeInContext(codegen->context);
        size_arg = LLVMBuildIntCast(codegen->builder, size_arg, i64_type, "size_cast");
    }

    // Call malloc
    LLVMTypeRef malloc_type = LLVMGlobalGetValueType(malloc_func);
    return LLVMBuildCall2(codegen->builder, malloc_type, malloc_func, &size_arg, 1, "malloc_result");
}

LLVMValueRef codegen_std_free(CodeGen *codegen, ASTNode *call) {
    if (call->data.call.arg_count != 1) {
        fprintf(stderr, "std.free() expects exactly 1 argument (pointer)\n");
        return NULL;
    }

    // Get the pointer argument
    LLVMValueRef ptr_arg = codegen_expression(codegen, call->data.call.args[0]);
    if (!ptr_arg) {
        return NULL;
    }

    // Get free function
    LLVMValueRef free_func = LLVMGetNamedFunction(codegen->module, "free");
    if (!free_func) {
        fprintf(stderr, "free function not found\n");
        return NULL;
    }

    // Cast pointer to void* if necessary
    LLVMTypeRef void_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0);
    ptr_arg = LLVMBuildBitCast(codegen->builder, ptr_arg, void_ptr_type, "ptr_cast");

    // Call free
    LLVMTypeRef free_type = LLVMGlobalGetValueType(free_func);
    return LLVMBuildCall2(codegen->builder, free_type, free_func, &ptr_arg, 1, "");
}

void free_codegen(CodeGen *codegen) {
  if (!codegen)
    return;

  // Free symbol table entries
  for (int i = 0; i < codegen->variable_count; i++) {
    free(codegen->variables[i].name);
  }
  for (int i = 0; i < codegen->function_count; i++) {
    free(codegen->functions[i].name);
  }

  // Free LLVM objects
  LLVMDisposeBuilder(codegen->builder);
  LLVMDisposeModule(codegen->module);
  LLVMContextDispose(codegen->context);

  free(codegen);
}

LLVMTypeRef get_llvm_type(CodeGen *codegen, const char *type_name) {
  TypeKind type_kind = string_to_type(type_name);
  return get_llvm_type_from_kind(codegen, type_kind);
}

LLVMTypeRef get_llvm_type_from_kind(CodeGen *codegen, TypeKind type_kind) {
  // Handle pointer types
  if (is_pointer_type(type_kind)) {
    TypeKind pointed_type = get_pointed_type(type_kind);
    LLVMTypeRef base_type = get_llvm_type_from_kind(codegen, pointed_type);
    return LLVMPointerType(base_type, 0);
  }

  switch (type_kind) {
  case TYPE_VOID:
    return LLVMVoidTypeInContext(codegen->context);
  case TYPE_BOOL:
    return LLVMInt1TypeInContext(codegen->context); // i1 for bool
  case TYPE_I8:
    return LLVMInt8TypeInContext(codegen->context);
  case TYPE_I16:
    return LLVMInt16TypeInContext(codegen->context);
  case TYPE_I32:
    return LLVMInt32TypeInContext(codegen->context);
  case TYPE_I64:
    return LLVMInt64TypeInContext(codegen->context);
  case TYPE_I128:
    return LLVMIntTypeInContext(codegen->context, 128);  // 128-bit integer
  case TYPE_U8:
    return LLVMInt8TypeInContext(codegen->context);
  case TYPE_U16:
    return LLVMInt16TypeInContext(codegen->context);
  case TYPE_U32:
    return LLVMInt32TypeInContext(codegen->context);
  case TYPE_U64:
    return LLVMInt64TypeInContext(codegen->context);
  case TYPE_U128:
    return LLVMIntTypeInContext(codegen->context, 128);  // 128-bit integer (same as i128)
  case TYPE_F32:
    return LLVMFloatTypeInContext(codegen->context);
  case TYPE_F64:
    return LLVMDoubleTypeInContext(codegen->context);
  case TYPE_F128:
    return LLVMFP128TypeInContext(codegen->context);     // 128-bit float
  case TYPE_CHAR:
    return LLVMInt8TypeInContext(codegen->context);
  case TYPE_STRING:
    return LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0);
  default:
    // Check if it's a struct type
    if (is_struct_type(type_kind)) {
      StructType *st = get_struct_type(type_kind);
      if (st) {
        // Create LLVM struct type
        LLVMTypeRef *field_types =
            malloc(st->field_count * sizeof(LLVMTypeRef));
        for (int i = 0; i < st->field_count; i++) {
          field_types[i] = get_llvm_type_from_kind(codegen, st->fields[i].type);
        }

        LLVMTypeRef struct_type =
            LLVMStructType(field_types, st->field_count, 0);
        free(field_types);
        return struct_type;
      }
    }

    fprintf(stderr, "Unknown type kind: %d\n", type_kind);
    return LLVMInt32TypeInContext(codegen->context); // Default fallback
  }
}

LLVMValueRef get_variable(CodeGen *codegen, const char *name) {
  for (int i = 0; i < codegen->variable_count; i++) {
    if (strcmp(codegen->variables[i].name, name) == 0) {
      return codegen->variables[i].value;
    }
  }
  return NULL;
}

LLVMTypeRef get_variable_type(CodeGen *codegen, const char *name) {
  for (int i = 0; i < codegen->variable_count; i++) {
    if (strcmp(codegen->variables[i].name, name) == 0) {
      return codegen->variables[i].type;
    }
  }
  return NULL;
}

int get_variable_mutability(CodeGen *codegen, const char *name) {
  for (int i = 0; i < codegen->variable_count; i++) {
    if (strcmp(codegen->variables[i].name, name) == 0) {
      return codegen->variables[i].is_mutable;
    }
  }
  return -1; // Variable not found
}

void set_variable(CodeGen *codegen, const char *name, LLVMValueRef value,
                  LLVMTypeRef type, int is_mutable) {
  set_variable_with_type(codegen, name, value, type, is_mutable, TYPE_UNKNOWN);
}

void set_variable_with_type(CodeGen *codegen, const char *name,
                            LLVMValueRef value, LLVMTypeRef type,
                            int is_mutable, TypeKind type_kind) {
  // Check if variable already exists (update it)
  for (int i = 0; i < codegen->variable_count; i++) {
    if (strcmp(codegen->variables[i].name, name) == 0) {
      codegen->variables[i].value = value;
      codegen->variables[i].type = type;
      codegen->variables[i].is_mutable = is_mutable;
      codegen->variables[i].type_kind = type_kind;
      return;
    }
  }

  // Add new variable
  if (codegen->variable_count < 256) {
    codegen->variables[codegen->variable_count].name = strdup(name);
    codegen->variables[codegen->variable_count].value = value;
    codegen->variables[codegen->variable_count].type = type;
    codegen->variables[codegen->variable_count].is_mutable = is_mutable;
    codegen->variables[codegen->variable_count].type_kind = type_kind;
    codegen->variable_count++;
  }
}

TypeKind get_variable_type_kind(CodeGen *codegen, const char *name) {
  for (int i = 0; i < codegen->variable_count; i++) {
    if (strcmp(codegen->variables[i].name, name) == 0) {
      return codegen->variables[i].type_kind;
    }
  }
  return TYPE_UNKNOWN;
}

TypeKind get_expression_type(CodeGen *codegen, ASTNode *node) {
  if (!node)
    return TYPE_UNKNOWN;

  switch (node->type) {
  case NODE_LITERAL:
    return node->data.literal.resolved_type;
  case NODE_IDENTIFIER:
    return get_variable_type_kind(codegen, node->data.identifier.name);
  case NODE_BINARY_OP:
    // If resolved_type is not set, try to infer it from operands
    if (node->data.binary_op.resolved_type != TYPE_UNKNOWN) {
      return node->data.binary_op.resolved_type;
    } else {
      // For arithmetic operations, return the type of the left operand
      // This is a simplification - in a full implementation we'd do proper type
      // inference
      TypeKind left_type =
          get_expression_type(codegen, node->data.binary_op.left);
      TypeKind right_type =
          get_expression_type(codegen, node->data.binary_op.right);

      // For comparison operations, return bool
      if (node->data.binary_op.operator == OP_LT ||
          node->data.binary_op.operator == OP_GT ||
          node->data.binary_op.operator == OP_LE ||
          node->data.binary_op.operator == OP_GE ||
          node->data.binary_op.operator == OP_EQ ||
          node->data.binary_op.operator == OP_NE) {
        return TYPE_BOOL;
      }

      // For arithmetic operations, return the common type (simplified)
      if (left_type != TYPE_UNKNOWN)
        return left_type;
      if (right_type != TYPE_UNKNOWN)
        return right_type;
      return TYPE_UNKNOWN;
    }
  case NODE_UNARY_OP:
    return node->data.unary_op.resolved_type;
  case NODE_CALL:
    // Handle built-in function return types
    if (strcmp(node->data.call.name, "std.to_string") == 0) {
      return TYPE_STRING;
    } else if (strcmp(node->data.call.name, "std.to_int") == 0) {
      return TYPE_I32;
    } else if (strcmp(node->data.call.name, "std.to_i64") == 0) {
      return TYPE_I64;
    } else if (strcmp(node->data.call.name, "std.input") == 0) {
      return TYPE_STRING;
    } else if (strcmp(node->data.call.name, "std.readln") == 0) {
      return TYPE_STRING;
    } else if (strcmp(node->data.call.name, "cast") == 0) {
      // Cast function returns the target type
      if (node->data.call.arg_count >= 2) {
        ASTNode *target_type_arg = node->data.call.args[1];
        if (target_type_arg->type == NODE_LITERAL && 
            strcmp(target_type_arg->data.literal.type, "string") == 0) {
          return string_to_type(target_type_arg->data.literal.value);
        }
      }
      return TYPE_UNKNOWN;
    } else {
      // For user-defined functions, we'd need to look up the function signature
      // For now, return unknown
      return TYPE_UNKNOWN;
    }
  default:
    return TYPE_UNKNOWN;
  }
}

LLVMValueRef get_function(CodeGen *codegen, const char *name) {
  for (int i = 0; i < codegen->function_count; i++) {
    if (strcmp(codegen->functions[i].name, name) == 0) {
      return codegen->functions[i].function;
    }
  }
  return NULL;
}

void set_function(CodeGen *codegen, const char *name, LLVMValueRef function) {
  if (codegen->function_count < 256) {
    codegen->functions[codegen->function_count].name = strdup(name);
    codegen->functions[codegen->function_count].function = function;
    codegen->function_count++;
  }
}

void process_local_import(CodeGen *codegen, ASTNode *import) {
  if (import->type != NODE_IMPORT ||
      import->data.import.import_type != IMPORT_LOCAL) {
    fprintf(stderr, "Expected local import node\n");
    return;
  }

  // Build file path: import path + .gloin extension
  char *path = import->data.import.path;
  char *file_path = malloc(strlen(path) + 7); // +7 for ".gloin\0"
  sprintf(file_path, "%s.gloin", path);

  // Check if file exists
  FILE *file = fopen(file_path, "r");
  if (!file) {
    fprintf(stderr, "Cannot open import file: %s\n", file_path);
    free(file_path);
    return;
  }
  fclose(file);

  // Parse the imported file
  ASTNode *imported_program = parse_file(file_path);
  free(file_path);

  if (!imported_program) {
    fprintf(stderr, "Failed to parse imported file\n");
    return;
  }

  // Add imported functions to the current module
  for (int i = 0; i < imported_program->data.program.function_count; i++) {
    ASTNode *func = imported_program->data.program.functions[i];
    if (func->type == NODE_FUNCTION) {
      // Generate the function in the current module
      codegen_function(codegen, func);
    }
  }

  // TODO: Clean up the imported AST properly
}

void process_external_import(CodeGen *codegen, ASTNode *import) {
  if (import->type != NODE_IMPORT ||
      import->data.import.import_type != IMPORT_EXTERNAL) {
    fprintf(stderr, "Expected external import node\n");
    return;
  }

  // Build file path: includes/package_name.gloin
  char *package_name = import->data.import.path;
  char *file_path =
      malloc(strlen(package_name) + 17); // +17 for "includes/" + ".gloin\0"
  sprintf(file_path, "includes/%s.gloin", package_name);

  // Check if file exists
  FILE *file = fopen(file_path, "r");
  if (!file) {
    fprintf(stderr, "Cannot open external package: %s\n", file_path);
    fprintf(stderr,
            "Make sure the package is installed in includes/ directory\n");
    fprintf(stderr, "You can install it with: mine dig <package_url>\n");
    free(file_path);
    return;
  }
  fclose(file);

  // TODO: Validate package against armory.toml dependencies
  // For now, we'll just check if armory.toml exists and warn if the package
  // isn't listed
  FILE *armory_file = fopen("armory.toml", "r");
  if (armory_file) {
    fclose(armory_file);
    // Could add validation here in the future
    printf("Info: Using external package '%s' (check armory.toml for version "
           "info)\n",
           package_name);
  }

  // Parse the external package file
  ASTNode *imported_program = parse_file(file_path);
  free(file_path);

  if (!imported_program) {
    fprintf(stderr, "Failed to parse external package\n");
    return;
  }

  // Add imported functions to the current module
  for (int i = 0; i < imported_program->data.program.function_count; i++) {
    ASTNode *func = imported_program->data.program.functions[i];
    if (func->type == NODE_FUNCTION) {
      // Generate the function in the current module
      codegen_function(codegen, func);
    }
  }

  // TODO: Clean up the imported AST properly
}

LLVMValueRef codegen_program(CodeGen *codegen, ASTNode *program) {
  if (program->type != NODE_PROGRAM) {
    fprintf(stderr, "Expected program node\n");
    return NULL;
  }

  // Process imports
  for (int i = 0; i < program->data.program.import_count; i++) {
    ASTNode *import = program->data.program.imports[i];
    if (import->data.import.import_type == IMPORT_LOCAL) {
      process_local_import(codegen, import);
    } else if (import->data.import.import_type == IMPORT_EXTERNAL) {
      process_external_import(codegen, import);
    }
    // TODO: Actually handle std imports
    // Skip std imports (already handled by built-in functions)
  }

  // First, perform type checking and resolution
  resolve_types(program);

  // Generate all functions and structs
  for (int i = 0; i < program->data.program.function_count; i++) {
    ASTNode *node = program->data.program.functions[i];
    if (node->type == NODE_FUNCTION) {
      codegen_function(codegen, node);
    } else if (node->type == NODE_STRUCT) {
      codegen_struct(codegen, node);
    } else {
      fprintf(stderr, "Unexpected node type in program: %d\n", node->type);
    }
    if (codegen->has_error) {
      return NULL;
    }
  }

  // Verify the module
  char *error = NULL;
  LLVMVerifyModule(codegen->module, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);

  return NULL; // Program doesn't return a value
}

LLVMValueRef codegen_function(CodeGen *codegen, ASTNode *function) {
  if (function->type != NODE_FUNCTION) {
    fprintf(stderr, "Expected function node\n");
    return NULL;
  }

  // Get return type
  LLVMTypeRef return_type =
      get_llvm_type(codegen, function->data.function.return_type);

  // Build parameter types array
  LLVMTypeRef *param_types = NULL;
  int param_count = function->data.function.param_count;

  if (param_count > 0) {
    param_types = malloc(sizeof(LLVMTypeRef) * param_count);
    for (int i = 0; i < param_count; i++) {
      ASTNode *param = function->data.function.params[i];
      param_types[i] = get_llvm_type(codegen, param->data.parameter.type);
    }
  }

  LLVMTypeRef function_type =
      LLVMFunctionType(return_type, param_types, param_count, 0);

  // Create the function
  LLVMValueRef llvm_function = LLVMAddFunction(
      codegen->module, function->data.function.name, function_type);

  // Add to function table
  set_function(codegen, function->data.function.name, llvm_function);

  // Create basic block
  LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(llvm_function, "entry");
  LLVMPositionBuilderAtEnd(codegen->builder, entry_block);

  // Set current function for variable scoping
  codegen->current_function = llvm_function;

  // Add parameters to symbol table
  for (int i = 0; i < param_count; i++) {
    ASTNode *param = function->data.function.params[i];
    LLVMValueRef param_value = LLVMGetParam(llvm_function, i);
    LLVMSetValueName(param_value, param->data.parameter.name);

    // Create alloca for parameter and store the parameter value
    LLVMTypeRef param_type = get_llvm_type(codegen, param->data.parameter.type);
    LLVMValueRef param_alloca = LLVMBuildAlloca(codegen->builder, param_type,
                                                param->data.parameter.name);
    LLVMBuildStore(codegen->builder, param_value, param_alloca);

    // Add to symbol table (parameters are always mutable)
    set_variable_with_type(codegen, param->data.parameter.name, param_alloca, 
                          param_type, 1, param->data.parameter.resolved_type);
  }

  // Generate function body
  if (function->data.function.body) {
    codegen_block(codegen, function->data.function.body);
  }

  // Add default return if function doesn't end with one
  LLVMBasicBlockRef current_block = LLVMGetInsertBlock(codegen->builder);
  if (current_block && !LLVMGetBasicBlockTerminator(current_block)) {
    if (strcmp(function->data.function.return_type, "void") == 0) {
      LLVMBuildRetVoid(codegen->builder);
    } else {
      // Return default value (0 for integers)
      LLVMValueRef default_value = LLVMConstInt(return_type, 0, 0);
      LLVMBuildRet(codegen->builder, default_value);
    }
  }

  codegen->current_function = NULL;

  // Clean up parameter types array
  if (param_types) {
    free(param_types);
  }

  // TODO: Clear function-local variables from symbol table
  // For now, we'll keep it simple and rely on function scope

  return llvm_function;
}

LLVMValueRef codegen_block(CodeGen *codegen, ASTNode *block) {
  if (block->type != NODE_BLOCK) {
    fprintf(stderr, "Expected block node\n");
    return NULL;
  }

  LLVMValueRef last_value = NULL;

  for (int i = 0; i < block->data.block.statement_count; i++) {
    // Check if current block is terminated before processing next statement
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(codegen->builder);
    if (LLVMGetBasicBlockTerminator(current_block)) {
      // Current block is already terminated, remaining statements are
      // unreachable
      break;
    }

    last_value = codegen_statement(codegen, block->data.block.statements[i]);
    // Note: We let individual statement handlers manage their error reporting
    // NULL return values are normal for some statements like assignments
  }

  return last_value;
}

LLVMValueRef codegen_statement(CodeGen *codegen, ASTNode *statement) {
  switch (statement->type) {
  case NODE_VARIABLE_DECL:
    return codegen_variable_decl(codegen, statement);
  case NODE_ASSIGNMENT:
    return codegen_assignment(codegen, statement);
  case NODE_POINTER_ASSIGNMENT:
    return codegen_pointer_assignment(codegen, statement);
  case NODE_RETURN:
    return codegen_return(codegen, statement);
  case NODE_CALL:
    return codegen_call(codegen, statement);
  case NODE_STRUCT:
    return codegen_struct(codegen, statement);
  case NODE_BLOCK:
    return codegen_block(codegen, statement);
  case NODE_IF:
    return codegen_if(codegen, statement);
  case NODE_UNLESS:
    return codegen_unless(codegen, statement);
  case NODE_FOR:
    return codegen_for(codegen, statement);
  case NODE_WHILE:
    return codegen_while(codegen, statement);
  case NODE_SWITCH:
    return codegen_switch(codegen, statement);
  case NODE_MATCH:
    return codegen_match(codegen, statement);
  case NODE_BREAK:
    return codegen_break(codegen, statement);
  case NODE_CONTINUE:
    return codegen_continue(codegen, statement);
  default:
    return codegen_expression(codegen, statement);
  }
}

LLVMValueRef codegen_expression(CodeGen *codegen, ASTNode *expression) {
  switch (expression->type) {
  case NODE_LITERAL:
    return codegen_literal(codegen, expression);
  case NODE_IDENTIFIER:
    return codegen_identifier(codegen, expression);
  case NODE_CALL:
    return codegen_call(codegen, expression);
  case NODE_BINARY_OP:
    return codegen_binary_op(codegen, expression);
  case NODE_UNARY_OP:
    return codegen_unary_op(codegen, expression);
  case NODE_ASSIGNMENT:
    return codegen_assignment(codegen, expression);
  case NODE_FIELD_ACCESS:
    return codegen_field_access(codegen, expression);
  case NODE_METHOD_CALL:
    return codegen_method_call(codegen, expression);
  case NODE_STRUCT_LITERAL:
    return codegen_struct_literal(codegen, expression);
  default:
    fprintf(stderr, "Unknown expression type: %d\n", expression->type);
    return NULL;
  }
}

LLVMValueRef codegen_variable_decl(CodeGen *codegen, ASTNode *var_decl) {
  if (var_decl->type != NODE_VARIABLE_DECL) {
    fprintf(stderr, "Expected variable declaration node\n");
    return NULL;
  }

  // Get the type
  LLVMTypeRef var_type =
      get_llvm_type(codegen, var_decl->data.variable_decl.type);

  // Create alloca instruction for the variable
  LLVMValueRef alloca_inst = LLVMBuildAlloca(codegen->builder, var_type,
                                             var_decl->data.variable_decl.name);

  // Generate initial value
  LLVMValueRef initial_value = NULL;
  if (var_decl->data.variable_decl.value) {
    initial_value =
        codegen_expression(codegen, var_decl->data.variable_decl.value);
    if (initial_value) {
      // Check if this is a struct assignment
      if (var_decl->data.variable_decl.value->type == NODE_STRUCT_LITERAL) {
        // For struct literals, we need to copy the struct, not store a pointer
        LLVMValueRef struct_value = LLVMBuildLoad2(codegen->builder, var_type,
                                                   initial_value, "struct_val");
        LLVMBuildStore(codegen->builder, struct_value, alloca_inst);
      } else {
        // Get the target type and source type
        TypeKind target_type = var_decl->data.variable_decl.resolved_type;
        TypeKind source_type = get_expression_type(codegen, var_decl->data.variable_decl.value);
        
        // If types don't match, we need to cast
        if (target_type != source_type && source_type != TYPE_UNKNOWN) {
          LLVMTypeRef target_llvm_type = get_llvm_type_from_kind(codegen, target_type);
          const Type *src_info = get_type_info(source_type);
          const Type *target_info = get_type_info(target_type);
          
          if (src_info && target_info && src_info->is_numeric && target_info->is_numeric) {
            // Perform automatic type conversion for numeric types
            if (src_info->size < target_info->size) {
              // Widening conversion
              if (src_info->is_signed && target_info->is_signed) {
                initial_value = LLVMBuildSExt(codegen->builder, initial_value, target_llvm_type, "auto_sext");
              } else if (!src_info->is_signed && !target_info->is_signed) {
                initial_value = LLVMBuildZExt(codegen->builder, initial_value, target_llvm_type, "auto_zext");
              } else {
                initial_value = LLVMBuildIntCast(codegen->builder, initial_value, target_llvm_type, "auto_cast");
              }
            } else if (src_info->size > target_info->size) {
              // Narrowing conversion (truncation)
              initial_value = LLVMBuildTrunc(codegen->builder, initial_value, target_llvm_type, "auto_trunc");
            } else {
              // Same size, different signedness
              initial_value = LLVMBuildBitCast(codegen->builder, initial_value, target_llvm_type, "auto_bitcast");
            }
          }
        }
        
        LLVMBuildStore(codegen->builder, initial_value, alloca_inst);
      }
    }
  }

  // Add to symbol table
  set_variable_with_type(codegen, var_decl->data.variable_decl.name,
                         alloca_inst, var_type,
                         var_decl->data.variable_decl.is_mutable,
                         var_decl->data.variable_decl.resolved_type);

  return alloca_inst;
}

LLVMValueRef codegen_assignment(CodeGen *codegen, ASTNode *assignment) {
  if (assignment->type != NODE_ASSIGNMENT) {
    fprintf(stderr, "Expected assignment statement node\n");
    return NULL;
  }

  // Check if variable exists and is mutable
  const char *var_name = assignment->data.assignment.variable_name;
  LLVMValueRef var_alloca = get_variable(codegen, var_name);

  if (!var_alloca) {
    fprintf(stderr, "Error: Undefined variable '%s' in assignment\n", var_name);
    return NULL;
  }

  int mutability = get_variable_mutability(codegen, var_name);
  if (mutability == 0) {
    fprintf(stderr, "Error: Cannot assign to immutable variable '%s'\n",
            var_name);
    codegen->has_error = 1;
    return NULL;
  }

  // Generate code for the new value
  LLVMValueRef new_value =
      codegen_expression(codegen, assignment->data.assignment.value);
  if (!new_value) {
    fprintf(stderr, "Error: Failed to generate code for assignment value\n");
    return NULL;
  }

  // Store the new value
  return LLVMBuildStore(codegen->builder, new_value, var_alloca);
}

LLVMValueRef codegen_pointer_assignment(CodeGen *codegen, ASTNode *assignment) {
  if (assignment->type != NODE_POINTER_ASSIGNMENT) {
    fprintf(stderr, "Expected pointer assignment statement node\n");
    return NULL;
  }

  // Generate code for the target (should be a dereference)
  ASTNode *target = assignment->data.pointer_assignment.target;
  if (target->type != NODE_UNARY_OP ||
      target->data.unary_op.operator != UNARY_DEREFERENCE) {
    fprintf(stderr, "Error: Pointer assignment target must be a dereference\n");
    return NULL;
  }

  // Get the pointer value (the operand of the dereference)
  LLVMValueRef pointer =
      codegen_expression(codegen, target->data.unary_op.operand);
  if (!pointer) {
    fprintf(stderr,
            "Error: Failed to generate code for pointer in assignment\n");
    return NULL;
  }

  // Generate code for the new value
  LLVMValueRef new_value =
      codegen_expression(codegen, assignment->data.pointer_assignment.value);
  if (!new_value) {
    fprintf(stderr,
            "Error: Failed to generate code for pointer assignment value\n");
    return NULL;
  }

  // Store the new value at the pointer location
  return LLVMBuildStore(codegen->builder, new_value, pointer);
}

LLVMValueRef codegen_return(CodeGen *codegen, ASTNode *return_stmt) {
  if (return_stmt->type != NODE_RETURN) {
    fprintf(stderr, "Expected return statement node\n");
    return NULL;
  }

  if (return_stmt->data.return_stmt.value) {
    LLVMValueRef return_value =
        codegen_expression(codegen, return_stmt->data.return_stmt.value);
    return LLVMBuildRet(codegen->builder, return_value);
  } else {
    return LLVMBuildRetVoid(codegen->builder);
  }
}

LLVMValueRef codegen_call(CodeGen *codegen, ASTNode *call) {
  if (call->type != NODE_CALL) {
    fprintf(stderr, "Expected call node\n");
    return NULL;
  }

  // Handle built-in functions (like std.print)
  if (strcmp(call->data.call.name, "std.print") == 0) {
    return codegen_std_print(codegen, call);
  } else if (strcmp(call->data.call.name, "std.println") == 0) {
    return codegen_std_println(codegen, call);
  } else if (strcmp(call->data.call.name, "std.input") == 0) {
    return codegen_std_input(codegen, call);
  } else if (strcmp(call->data.call.name, "std.readln") == 0) {
    return codegen_std_readln(codegen, call);
  } else if (strcmp(call->data.call.name, "std.to_int") == 0) {
    return codegen_std_to_int(codegen, call);
  } else if (strcmp(call->data.call.name, "std.to_i64") == 0) {
    return codegen_std_to_i64(codegen, call);
  } else if (strcmp(call->data.call.name, "std.to_string") == 0) {
    return codegen_std_to_string(codegen, call);
  } else if (strcmp(call->data.call.name, "cast") == 0) {
    return codegen_cast(codegen, call);
  } else if (strcmp(call->data.call.name, "std.malloc") == 0) {
    return codegen_std_malloc(codegen, call);
  } else if (strcmp(call->data.call.name, "std.free") == 0) {
    return codegen_std_free(codegen, call);
  }

  // Look up the function
  LLVMValueRef function = get_function(codegen, call->data.call.name);
  if (!function) {
    fprintf(stderr, "Unknown function: %s\n", call->data.call.name);
    return NULL;
  }

  // Generate arguments
  LLVMValueRef *args = NULL;
  if (call->data.call.arg_count > 0) {
    args = malloc(sizeof(LLVMValueRef) * call->data.call.arg_count);
    for (int i = 0; i < call->data.call.arg_count; i++) {
      args[i] = codegen_expression(codegen, call->data.call.args[i]);
    }
  }

  // Create the call
  LLVMTypeRef function_type = LLVMGlobalGetValueType(function);
  LLVMValueRef call_result =
      LLVMBuildCall2(codegen->builder, function_type, function, args,
                     call->data.call.arg_count, "");

  if (args) {
    free(args);
  }

  return call_result;
}

LLVMValueRef codegen_literal(CodeGen *codegen, ASTNode *literal) {
  if (literal->type != NODE_LITERAL) {
    fprintf(stderr, "Expected literal node\n");
    return NULL;
  }

  if (strcmp(literal->data.literal.type, "i32") == 0) {
    int value = atoi(literal->data.literal.value);
    return LLVMConstInt(LLVMInt32TypeInContext(codegen->context), value, 0);
  } else if (strcmp(literal->data.literal.type, "bool") == 0) {
    int value = (strcmp(literal->data.literal.value, "true") == 0) ? 1 : 0;
    return LLVMConstInt(LLVMInt1TypeInContext(codegen->context), value, 0);
  } else if (strcmp(literal->data.literal.type, "string") == 0) {
    // Create a global string constant
    LLVMValueRef string_const = LLVMBuildGlobalStringPtr(
        codegen->builder, literal->data.literal.value, "str");
    return string_const;
  } else {
    fprintf(stderr, "Unknown literal type: %s\n", literal->data.literal.type);
    return NULL;
  }
}

LLVMValueRef codegen_identifier(CodeGen *codegen, ASTNode *identifier) {
  if (identifier->type != NODE_IDENTIFIER) {
    fprintf(stderr, "Expected identifier node\n");
    return NULL;
  }

  // Look up variable
  LLVMValueRef var = get_variable(codegen, identifier->data.identifier.name);
  if (!var) {
    fprintf(stderr, "Unknown variable: %s\n", identifier->data.identifier.name);
    return NULL;
  }

  // Get the variable type from our symbol table
  LLVMTypeRef var_type =
      get_variable_type(codegen, identifier->data.identifier.name);
  if (!var_type) {
    fprintf(stderr, "Unknown variable type: %s\n",
            identifier->data.identifier.name);
    return NULL;
  }

  // Load the value
  return LLVMBuildLoad2(codegen->builder, var_type, var,
                        identifier->data.identifier.name);
}

LLVMValueRef codegen_binary_op(CodeGen *codegen, ASTNode *binary_op) {
  if (binary_op->type != NODE_BINARY_OP) {
    fprintf(stderr, "Expected binary operation node\n");
    return NULL;
  }

  // Get operand types for type checking
  TypeKind left_type =
      get_expression_type(codegen, binary_op->data.binary_op.left);
  TypeKind right_type =
      get_expression_type(codegen, binary_op->data.binary_op.right);

  // Type compatibility checking
  int is_comparison = (binary_op->data.binary_op.operator >= OP_EQ &&
                       binary_op->data.binary_op.operator <= OP_GE);

  if (is_comparison) {
    if (!types_comparable(left_type, right_type)) {
      fprintf(stderr,
              "Error: Cannot compare incompatible types '%s' and '%s'\n",
              type_to_string(left_type), type_to_string(right_type));
      codegen->has_error = 1;
      return NULL;
    }
  } else {
    // Arithmetic operations
    if (!types_compatible(left_type, right_type)) {
      fprintf(stderr,
              "Error: Cannot perform arithmetic on incompatible types '%s' and "
              "'%s'\n",
              type_to_string(left_type), type_to_string(right_type));
      codegen->has_error = 1;
      return NULL;
    }
  }

  // Generate left and right operands
  LLVMValueRef left =
      codegen_expression(codegen, binary_op->data.binary_op.left);
  LLVMValueRef right =
      codegen_expression(codegen, binary_op->data.binary_op.right);

  if (!left || !right) {
    fprintf(stderr, "Failed to generate operands for binary operation\n");
    return NULL;
  }

  // Generate the appropriate LLVM instruction based on operator
  switch (binary_op->data.binary_op.operator) {
  case OP_ADD:
    return LLVMBuildAdd(codegen->builder, left, right, "addtmp");
  case OP_SUB:
    return LLVMBuildSub(codegen->builder, left, right, "subtmp");
  case OP_MUL:
    return LLVMBuildMul(codegen->builder, left, right, "multmp");
  case OP_DIV:
    // Use signed division for i32
    return LLVMBuildSDiv(codegen->builder, left, right, "divtmp");
  case OP_EQ:
    return LLVMBuildICmp(codegen->builder, LLVMIntEQ, left, right, "eqtmp");
  case OP_NE:
    return LLVMBuildICmp(codegen->builder, LLVMIntNE, left, right, "netmp");
  case OP_LT:
    return LLVMBuildICmp(codegen->builder, LLVMIntSLT, left, right, "lttmp");
  case OP_GT:
    return LLVMBuildICmp(codegen->builder, LLVMIntSGT, left, right, "gttmp");
  case OP_LE:
    return LLVMBuildICmp(codegen->builder, LLVMIntSLE, left, right, "letmp");
  case OP_GE:
    return LLVMBuildICmp(codegen->builder, LLVMIntSGE, left, right, "getmp");
  default:
    fprintf(stderr, "Unknown binary operator: %d\n",
            binary_op->data.binary_op.operator);
    return NULL;
  }
}

LLVMValueRef codegen_unary_op(CodeGen *codegen, ASTNode *unary_op) {
  if (unary_op->type != NODE_UNARY_OP) {
    fprintf(stderr, "Expected unary operation node\n");
    return NULL;
  }

  ASTNode *operand = unary_op->data.unary_op.operand;

  switch (unary_op->data.unary_op.operator) {
  case UNARY_ADDRESS_OF:
    // For address-of, we need to get the address of the operand
    if (operand->type == NODE_IDENTIFIER) {
      // Get the alloca instruction for this variable
      LLVMValueRef var_alloca =
          get_variable(codegen, operand->data.identifier.name);
      if (!var_alloca) {
        fprintf(stderr,
                "Error: Variable '%s' not found for address-of operation\n",
                operand->data.identifier.name);
        return NULL;
      }
      return var_alloca; // The alloca itself is the address
    } else {
      fprintf(stderr,
              "Error: Address-of operator can only be applied to variables\n");
      return NULL;
    }

  case UNARY_DEREFERENCE:
    // For dereference, we need to load from the pointer
    LLVMValueRef ptr_value = codegen_expression(codegen, operand);
    if (!ptr_value) {
      fprintf(stderr,
              "Error: Failed to generate code for dereference operand\n");
      return NULL;
    }

    // Get the pointed-to type for the load instruction
    TypeKind operand_type = get_expression_type(codegen, operand);
    if (!is_pointer_type(operand_type)) {
      fprintf(stderr, "Error: Cannot dereference non-pointer type '%s'\n",
              type_to_string(operand_type));
      codegen->has_error = 1;
      return NULL;
    }

    TypeKind pointed_type = get_pointed_type(operand_type);
    LLVMTypeRef pointed_llvm_type =
        get_llvm_type_from_kind(codegen, pointed_type);

    return LLVMBuildLoad2(codegen->builder, pointed_llvm_type, ptr_value,
                          "deref");

  default:
    fprintf(stderr, "Unknown unary operator: %d\n",
            unary_op->data.unary_op.operator);
    return NULL;
  }
}

void print_llvm_ir(CodeGen *codegen) {
  char *ir = LLVMPrintModuleToString(codegen->module);
  printf("%s\n", ir);
  LLVMDisposeMessage(ir);
}

int write_object_file(CodeGen *codegen, const char *filename) {
  char *error_msg;

  // Initialize target
  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllTargets();
  LLVMInitializeAllTargetMCs();
  LLVMInitializeAllAsmParsers();
  LLVMInitializeAllAsmPrinters();

  char *target_triple = LLVMGetDefaultTargetTriple();
  LLVMSetTarget(codegen->module, target_triple);

  LLVMTargetRef target;
  if (LLVMGetTargetFromTriple(target_triple, &target, &error_msg)) {
    fprintf(stderr, "Error getting target: %s\n", error_msg);
    LLVMDisposeMessage(error_msg);
    return 1;
  }

  LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(
      target, target_triple, "generic", "", LLVMCodeGenLevelDefault,
      LLVMRelocDefault, LLVMCodeModelDefault);

  if (LLVMTargetMachineEmitToFile(target_machine, codegen->module,
                                  (char *)filename, LLVMObjectFile,
                                  &error_msg)) {
    fprintf(stderr, "Error writing object file: %s\n", error_msg);
    LLVMDisposeMessage(error_msg);
    return 1;
  }

  LLVMDisposeTargetMachine(target_machine);
  LLVMDisposeMessage(target_triple);

  return 0;
}

int write_executable(CodeGen *codegen, const char *filename) {
  // First write object file
  char obj_filename[256];
  snprintf(obj_filename, sizeof(obj_filename), "%s.o", filename);

  if (write_object_file(codegen, obj_filename) != 0) {
    return 1;
  }

  // Then link it (using system linker)
  char link_command[512];
  snprintf(link_command, sizeof(link_command), "gcc -no-pie %s -o %s",
           obj_filename, filename);

  int result = system(link_command);

  // Clean up object file
  remove(obj_filename);

  return result;
}

// Struct-related code generation functions

LLVMValueRef codegen_struct(CodeGen *codegen, ASTNode *struct_decl) {
  // Struct declarations don't generate runtime code,
  // they just register LLVM types
  if (struct_decl->type != NODE_STRUCT) {
    fprintf(stderr, "Expected struct declaration node\n");
    return NULL;
  }

  // Get the struct type info from our type system
  StructType *st = find_struct_by_name(struct_decl->data.struct_decl.name);
  if (!st) {
    fprintf(stderr, "Struct type '%s' not found in type system\n",
            struct_decl->data.struct_decl.name);
    return NULL;
  }

  // Create LLVM struct type
  LLVMTypeRef *field_types = malloc(st->field_count * sizeof(LLVMTypeRef));
  for (int i = 0; i < st->field_count; i++) {
    field_types[i] = get_llvm_type_from_kind(codegen, st->fields[i].type);
  }

  LLVMTypeRef struct_type = LLVMStructType(field_types, st->field_count, 0);

  // Generate method functions
  for (int i = 0; i < struct_decl->data.struct_decl.method_count; i++) {
    ASTNode *method = struct_decl->data.struct_decl.methods[i];
    codegen_struct_method(codegen, method, struct_decl->data.struct_decl.name,
                          struct_type);
  }

  free(field_types);
  return NULL; // Struct declarations don't return values
}

LLVMValueRef codegen_struct_method(CodeGen *codegen, ASTNode *method,
                                   const char *struct_name,
                                   LLVMTypeRef struct_type) {
  if (method->type != NODE_STRUCT_METHOD) {
    fprintf(stderr, "Expected struct method node\n");
    return NULL;
  }

  // Create mangled method name: StructName_methodName
  char *mangled_name =
      malloc(strlen(struct_name) + strlen(method->data.struct_method.name) + 2);
  sprintf(mangled_name, "%s_%s", struct_name, method->data.struct_method.name);

  // Get return type
  LLVMTypeRef return_type =
      get_llvm_type(codegen, method->data.struct_method.return_type);

  // Build parameter types array (struct pointer + method parameters)
  int total_param_count =
      method->data.struct_method.param_count + 1; // +1 for struct instance
  LLVMTypeRef *param_types = malloc(sizeof(LLVMTypeRef) * total_param_count);

  // First parameter is pointer to struct (self)
  param_types[0] = LLVMPointerType(struct_type, 0);

  // Add method parameters
  for (int i = 0; i < method->data.struct_method.param_count; i++) {
    ASTNode *param = method->data.struct_method.params[i];
    param_types[i + 1] = get_llvm_type(codegen, param->data.parameter.type);
  }

  // Create function type
  LLVMTypeRef function_type =
      LLVMFunctionType(return_type, param_types, total_param_count, 0);

  // Create function
  LLVMValueRef function =
      LLVMAddFunction(codegen->module, mangled_name, function_type);

  // Set parameter names
  LLVMSetValueName(LLVMGetParam(function, 0), "self");
  for (int i = 0; i < method->data.struct_method.param_count; i++) {
    ASTNode *param = method->data.struct_method.params[i];
    LLVMSetValueName(LLVMGetParam(function, i + 1), param->data.parameter.name);
  }

  // Create basic block
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
  LLVMPositionBuilderAtEnd(codegen->builder, entry);

  // Save current variable scope
  int saved_var_count = codegen->variable_count;

  // Add 'self' parameter to variable scope as struct fields
  LLVMValueRef self_param = LLVMGetParam(function, 0);

  // Add method parameters to variable scope
  for (int i = 0; i < method->data.struct_method.param_count; i++) {
    ASTNode *param = method->data.struct_method.params[i];
    LLVMValueRef param_value = LLVMGetParam(function, i + 1);
    LLVMTypeRef param_type = get_llvm_type(codegen, param->data.parameter.type);

    // Create alloca for parameter and store the value
    LLVMValueRef param_alloca = LLVMBuildAlloca(codegen->builder, param_type,
                                                param->data.parameter.name);
    LLVMBuildStore(codegen->builder, param_value, param_alloca);

    set_variable_with_type(codegen, param->data.parameter.name, param_alloca,
                           param_type, 1, param->data.parameter.resolved_type);
  }

  // Add struct fields as accessible variables (through self pointer)
  StructType *st = find_struct_by_name(struct_name);
  if (st) {
    for (int i = 0; i < st->field_count; i++) {
      // Create GEP to access field through self pointer
      LLVMValueRef indices[2];
      indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);
      indices[1] = LLVMConstInt(LLVMInt32Type(), i, 0);

      LLVMValueRef field_ptr =
          LLVMBuildGEP2(codegen->builder, struct_type, self_param, indices, 2,
                        st->fields[i].name);
      LLVMTypeRef field_type =
          get_llvm_type_from_kind(codegen, st->fields[i].type);

      set_variable_with_type(codegen, st->fields[i].name, field_ptr, field_type,
                             1, st->fields[i].type);
    }
  }

  // Generate method body
  if (method->data.struct_method.body) {
    codegen_statement(codegen, method->data.struct_method.body);
  }

  // If no explicit return, add void return for void methods
  if (strcmp(method->data.struct_method.return_type, "void") == 0) {
    LLVMBuildRetVoid(codegen->builder);
  }

  // Restore variable scope
  codegen->variable_count = saved_var_count;

  free(param_types);
  free(mangled_name);
  return function;
}

LLVMValueRef codegen_field_access(CodeGen *codegen, ASTNode *field_access) {
  if (field_access->type != NODE_FIELD_ACCESS) {
    fprintf(stderr, "Expected field access node\n");
    return NULL;
  }

  // Get the object pointer (address, not value)
  LLVMValueRef object_ptr = NULL;
  if (field_access->data.field_access.object->type == NODE_IDENTIFIER) {
    // For identifiers, get the variable address directly (don't load the value)
    const char *var_name =
        field_access->data.field_access.object->data.identifier.name;
    object_ptr = get_variable(codegen, var_name);
    if (!object_ptr) {
      fprintf(stderr, "Unknown variable: %s\n", var_name);
      return NULL;
    }
  } else {
    // For other expressions, use normal codegen (might need adjustment for
    // other cases)
    object_ptr =
        codegen_expression(codegen, field_access->data.field_access.object);
    if (!object_ptr) {
      return NULL;
    }
  }

  // Get the object type from the symbol table
  TypeKind object_type = TYPE_UNKNOWN;
  if (field_access->data.field_access.object->type == NODE_IDENTIFIER) {
    const char *var_name =
        field_access->data.field_access.object->data.identifier.name;
    object_type = get_variable_type_kind(codegen, var_name);
  } else {
    object_type = get_node_type(field_access->data.field_access.object);
  }

  if (!is_struct_type(object_type)) {
    fprintf(stderr, "Cannot access field on non-struct type\n");
    return NULL;
  }

  // Get field index (not byte offset)
  StructType *st = get_struct_type(object_type);
  if (!st) {
    fprintf(stderr, "Failed to get struct type info\n");
    return NULL;
  }

  int field_index = -1;
  for (int i = 0; i < st->field_count; i++) {
    if (strcmp(st->fields[i].name,
               field_access->data.field_access.field_name) == 0) {
      field_index = i;
      break;
    }
  }

  if (field_index < 0) {
    fprintf(stderr, "Field '%s' not found in struct\n",
            field_access->data.field_access.field_name);
    return NULL;
  }

  // Generate GEP (Get Element Pointer) instruction
  LLVMTypeRef struct_llvm_type = get_llvm_type_from_kind(codegen, object_type);

  LLVMValueRef indices[2];
  indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0); // Dereference pointer
  indices[1] = LLVMConstInt(LLVMInt32Type(), field_index, 0); // Field index

  LLVMValueRef field_ptr = LLVMBuildGEP2(codegen->builder, struct_llvm_type,
                                         object_ptr, indices, 2, "field_ptr");

  // Load the field value
  TypeKind field_type = st->fields[field_index].type;
  LLVMTypeRef llvm_field_type = get_llvm_type_from_kind(codegen, field_type);

  return LLVMBuildLoad2(codegen->builder, llvm_field_type, field_ptr,
                        "field_val");
}

LLVMValueRef codegen_method_call(CodeGen *codegen, ASTNode *method_call) {
  if (method_call->type != NODE_METHOD_CALL) {
    fprintf(stderr, "Expected method call node\n");
    return NULL;
  }

  // Get the object type
  TypeKind object_type = TYPE_UNKNOWN;
  if (method_call->data.method_call.object->type == NODE_IDENTIFIER) {
    const char *var_name =
        method_call->data.method_call.object->data.identifier.name;
    object_type = get_variable_type_kind(codegen, var_name);
  } else {
    object_type = get_node_type(method_call->data.method_call.object);
  }

  if (!is_struct_type(object_type)) {
    fprintf(stderr, "Cannot call method on non-struct type\n");
    return NULL;
  }

  // Get struct type info
  StructType *st = get_struct_type(object_type);
  if (!st) {
    fprintf(stderr, "Failed to get struct type info for method call\n");
    return NULL;
  }

  // Create mangled function name: StructName_methodName
  char *mangled_name = malloc(
      strlen(st->name) + strlen(method_call->data.method_call.method_name) + 2);
  sprintf(mangled_name, "%s_%s", st->name,
          method_call->data.method_call.method_name);

  // Look up the function in the module
  LLVMValueRef function = LLVMGetNamedFunction(codegen->module, mangled_name);
  if (!function) {
    fprintf(stderr, "Method '%s' not found for struct '%s'\n",
            method_call->data.method_call.method_name, st->name);
    free(mangled_name);
    return NULL;
  }

  // Prepare arguments: self pointer + method arguments
  int total_arg_count = method_call->data.method_call.arg_count + 1;
  LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * total_arg_count);

  // First argument is pointer to the object (self)
  if (method_call->data.method_call.object->type == NODE_IDENTIFIER) {
    // Get the variable address directly (don't load the value)
    const char *var_name =
        method_call->data.method_call.object->data.identifier.name;
    args[0] = get_variable(codegen, var_name);
    if (!args[0]) {
      fprintf(stderr, "Unknown variable: %s\n", var_name);
      free(args);
      free(mangled_name);
      return NULL;
    }
  } else {
    // For other expressions, this would need more sophisticated handling
    fprintf(stderr, "Method calls on complex expressions not yet supported\n");
    free(args);
    free(mangled_name);
    return NULL;
  }

  // Add method arguments
  for (int i = 0; i < method_call->data.method_call.arg_count; i++) {
    args[i + 1] =
        codegen_expression(codegen, method_call->data.method_call.args[i]);
    if (!args[i + 1]) {
      fprintf(stderr, "Failed to generate code for method argument %d\n", i);
      free(args);
      free(mangled_name);
      return NULL;
    }
  }

  // Generate the function call
  LLVMTypeRef function_type = LLVMGlobalGetValueType(function);
  LLVMValueRef result =
      LLVMBuildCall2(codegen->builder, function_type, function, args,
                     total_arg_count, "method_call");

  free(args);
  free(mangled_name);
  return result;
}

LLVMValueRef codegen_struct_literal(CodeGen *codegen, ASTNode *struct_literal) {
  if (struct_literal->type != NODE_STRUCT_LITERAL) {
    fprintf(stderr, "Expected struct literal node\n");
    return NULL;
  }

  // Get the struct type
  StructType *st =
      find_struct_by_name(struct_literal->data.struct_literal.struct_type_name);
  if (!st) {
    fprintf(stderr, "Struct type '%s' not found\n",
            struct_literal->data.struct_literal.struct_type_name);
    return NULL;
  }

  // Create LLVM struct type
  LLVMTypeRef *field_types = malloc(st->field_count * sizeof(LLVMTypeRef));
  for (int i = 0; i < st->field_count; i++) {
    field_types[i] = get_llvm_type_from_kind(codegen, st->fields[i].type);
  }

  LLVMTypeRef struct_type = LLVMStructType(field_types, st->field_count, 0);

  // Allocate space for the struct
  LLVMValueRef struct_alloca =
      LLVMBuildAlloca(codegen->builder, struct_type, "struct_literal");

  // Initialize fields
  for (int i = 0; i < struct_literal->data.struct_literal.field_count; i++) {
    const char *field_name = struct_literal->data.struct_literal.field_names[i];
    ASTNode *field_value = struct_literal->data.struct_literal.field_values[i];

    // Find field index in struct
    int field_index = -1;
    for (int j = 0; j < st->field_count; j++) {
      if (strcmp(st->fields[j].name, field_name) == 0) {
        field_index = j;
        break;
      }
    }

    if (field_index < 0) {
      fprintf(stderr, "Field '%s' not found in struct '%s'\n", field_name,
              st->name);
      free(field_types);
      return NULL;
    }

    // Generate code for field value
    LLVMValueRef value = codegen_expression(codegen, field_value);
    if (!value) {
      free(field_types);
      return NULL;
    }

    // Store value in struct
    LLVMValueRef indices[2];
    indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);
    indices[1] = LLVMConstInt(LLVMInt32Type(), field_index, 0);

    LLVMValueRef field_ptr = LLVMBuildGEP2(
        codegen->builder, struct_type, struct_alloca, indices, 2, "field_ptr");
    LLVMBuildStore(codegen->builder, value, field_ptr);
  }

  free(field_types);
  return struct_alloca;
}

LLVMValueRef codegen_if(CodeGen *codegen, ASTNode *if_stmt) {
  // Generate condition
  LLVMValueRef condition =
      codegen_expression(codegen, if_stmt->data.if_stmt.condition);
  if (!condition)
    return NULL;

  // Create basic blocks
  LLVMValueRef function = codegen->current_function;
  LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(function, "if_then");
  LLVMBasicBlockRef else_block = NULL;
  LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(function, "if_merge");

  if (if_stmt->data.if_stmt.else_block) {
    else_block = LLVMAppendBasicBlock(function, "if_else");
    // Branch based on condition
    LLVMBuildCondBr(codegen->builder, condition, then_block, else_block);
  } else {
    // Branch based on condition (no else block)
    LLVMBuildCondBr(codegen->builder, condition, then_block, merge_block);
  }

  // Generate then block
  LLVMPositionBuilderAtEnd(codegen->builder, then_block);
  codegen_statement(codegen, if_stmt->data.if_stmt.then_block);
  // Only add branch if current block is not already terminated
  LLVMBasicBlockRef current_then_block = LLVMGetInsertBlock(codegen->builder);
  if (!LLVMGetBasicBlockTerminator(current_then_block)) {
    LLVMBuildBr(codegen->builder, merge_block);
  }

  // Generate else block if it exists
  LLVMBasicBlockRef current_else_block = NULL;
  if (else_block) {
    LLVMPositionBuilderAtEnd(codegen->builder, else_block);
    codegen_statement(codegen, if_stmt->data.if_stmt.else_block);
    // Only add branch if current block is not already terminated
    current_else_block = LLVMGetInsertBlock(codegen->builder);
    if (!LLVMGetBasicBlockTerminator(current_else_block)) {
      LLVMBuildBr(codegen->builder, merge_block);
    }
  }

  // Continue with merge block - but only if it's reachable
  // After processing then and else blocks, check if merge block has any
  // incoming edges
  int merge_has_predecessors = 0;

  // Check if condition can branch to merge (when there's no else block)
  if (!else_block) {
    merge_has_predecessors = 1;
  }

  // Check if then block can reach merge (wasn't terminated by
  // break/continue/return)
  if (current_then_block && !LLVMGetBasicBlockTerminator(current_then_block)) {
    merge_has_predecessors = 1;
  }

  // Check if else block can reach merge (if it exists)
  if (else_block && current_else_block &&
      !LLVMGetBasicBlockTerminator(current_else_block)) {
    merge_has_predecessors = 1;
  }

  if (merge_has_predecessors) {
    // At least one path can reach the merge block
    LLVMPositionBuilderAtEnd(codegen->builder, merge_block);
  } else {
    // All paths are terminated, merge block is unreachable
    LLVMDeleteBasicBlock(merge_block);
    // Position builder at end of the last block we generated
    if (else_block && current_else_block) {
      LLVMPositionBuilderAtEnd(codegen->builder, current_else_block);
    } else {
      LLVMPositionBuilderAtEnd(codegen->builder, current_then_block);
    }
  }

  return NULL; // Control flow statements don't return values
}

LLVMValueRef codegen_unless(CodeGen *codegen, ASTNode *unless_stmt) {
  // Generate condition
  LLVMValueRef condition =
      codegen_expression(codegen, unless_stmt->data.unless_stmt.condition);
  if (!condition)
    return NULL;

  // Create basic blocks
  LLVMValueRef function = codegen->current_function;
  LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(function, "unless_then");
  LLVMBasicBlockRef else_block = NULL;
  LLVMBasicBlockRef merge_block =
      LLVMAppendBasicBlock(function, "unless_merge");

  if (unless_stmt->data.unless_stmt.else_block) {
    else_block = LLVMAppendBasicBlock(function, "unless_else");
    // Branch based on condition (reversed logic for unless)
    LLVMBuildCondBr(codegen->builder, condition, else_block, then_block);
  } else {
    // Branch based on condition (reversed logic for unless, no else block)
    LLVMBuildCondBr(codegen->builder, condition, merge_block, then_block);
  }

  // Generate then block
  LLVMPositionBuilderAtEnd(codegen->builder, then_block);
  codegen_statement(codegen, unless_stmt->data.unless_stmt.then_block);
  // Only add branch if current block is not already terminated
  LLVMBasicBlockRef current_then_block = LLVMGetInsertBlock(codegen->builder);
  if (!LLVMGetBasicBlockTerminator(current_then_block)) {
    LLVMBuildBr(codegen->builder, merge_block);
  }

  // Generate else block if it exists
  LLVMBasicBlockRef current_else_block = NULL;
  if (else_block) {
    LLVMPositionBuilderAtEnd(codegen->builder, else_block);
    codegen_statement(codegen, unless_stmt->data.unless_stmt.else_block);
    // Only add branch if current block is not already terminated
    current_else_block = LLVMGetInsertBlock(codegen->builder);
    if (!LLVMGetBasicBlockTerminator(current_else_block)) {
      LLVMBuildBr(codegen->builder, merge_block);
    }
  }

  // Continue with merge block - but only if it's reachable
  // After processing then and else blocks, check if merge block has any
  // incoming edges
  int merge_has_predecessors = 0;

  // Check if condition can branch to merge (when there's no else block)
  if (!else_block) {
    merge_has_predecessors = 1;
  }

  // Check if then block can reach merge (wasn't terminated by
  // break/continue/return)
  if (current_then_block && !LLVMGetBasicBlockTerminator(current_then_block)) {
    merge_has_predecessors = 1;
  }

  // Check if else block can reach merge (if it exists)
  if (else_block && current_else_block &&
      !LLVMGetBasicBlockTerminator(current_else_block)) {
    merge_has_predecessors = 1;
  }

  if (merge_has_predecessors) {
    // At least one path can reach the merge block
    LLVMPositionBuilderAtEnd(codegen->builder, merge_block);
  } else {
    // All paths are terminated, merge block is unreachable
    LLVMDeleteBasicBlock(merge_block);
    // Position builder at end of the last block we generated
    if (else_block && current_else_block) {
      LLVMPositionBuilderAtEnd(codegen->builder, current_else_block);
    } else {
      LLVMPositionBuilderAtEnd(codegen->builder, current_then_block);
    }
  }

  return NULL; // Control flow statements don't return values
}

LLVMValueRef codegen_for(CodeGen *codegen, ASTNode *for_stmt) {
  // Create basic blocks
  LLVMValueRef function = codegen->current_function;
  LLVMBasicBlockRef init_block = LLVMAppendBasicBlock(function, "for_init");
  LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(function, "for_cond");
  LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(function, "for_body");
  LLVMBasicBlockRef update_block = LLVMAppendBasicBlock(function, "for_update");
  LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(function, "for_exit");

  // Generate initialization
  LLVMBuildBr(codegen->builder, init_block);
  LLVMPositionBuilderAtEnd(codegen->builder, init_block);
  if (for_stmt->data.for_stmt.init) {
    codegen_statement(codegen, for_stmt->data.for_stmt.init);
  }
  LLVMBuildBr(codegen->builder, cond_block);

  // Push loop context (continue goes to update, break goes to exit)
  push_loop_context(codegen, exit_block, update_block);

  // Generate condition check
  LLVMPositionBuilderAtEnd(codegen->builder, cond_block);
  if (for_stmt->data.for_stmt.condition) {
    LLVMValueRef condition =
        codegen_expression(codegen, for_stmt->data.for_stmt.condition);
    LLVMBuildCondBr(codegen->builder, condition, body_block, exit_block);
  } else {
    // Infinite loop if no condition
    LLVMBuildBr(codegen->builder, body_block);
  }

  // Generate body
  LLVMPositionBuilderAtEnd(codegen->builder, body_block);
  codegen_statement(codegen, for_stmt->data.for_stmt.body);
  // Only add branch if current block is not already terminated
  LLVMBasicBlockRef current_block = LLVMGetInsertBlock(codegen->builder);
  if (!LLVMGetBasicBlockTerminator(current_block)) {
    LLVMBuildBr(codegen->builder, update_block);
  }

  // Generate update
  LLVMPositionBuilderAtEnd(codegen->builder, update_block);
  if (for_stmt->data.for_stmt.update) {
    codegen_expression(codegen, for_stmt->data.for_stmt.update);
  }
  LLVMBuildBr(codegen->builder, cond_block);

  // Pop loop context
  pop_loop_context(codegen);

  // Continue with exit block
  LLVMPositionBuilderAtEnd(codegen->builder, exit_block);

  return NULL; // Control flow statements don't return values
}

LLVMValueRef codegen_while(CodeGen *codegen, ASTNode *while_stmt) {
  // Create basic blocks
  LLVMValueRef function = codegen->current_function;
  LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(function, "while_cond");
  LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(function, "while_body");
  LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(function, "while_exit");

  // Push loop context (continue goes to condition, break goes to exit)
  push_loop_context(codegen, exit_block, cond_block);

  // Jump to condition check
  LLVMBuildBr(codegen->builder, cond_block);

  // Generate condition check
  LLVMPositionBuilderAtEnd(codegen->builder, cond_block);
  LLVMValueRef condition =
      codegen_expression(codegen, while_stmt->data.while_stmt.condition);
  LLVMBuildCondBr(codegen->builder, condition, body_block, exit_block);

  // Generate body
  LLVMPositionBuilderAtEnd(codegen->builder, body_block);
  codegen_statement(codegen, while_stmt->data.while_stmt.body);
  // Only add branch if current block is not already terminated
  LLVMBasicBlockRef current_block = LLVMGetInsertBlock(codegen->builder);
  if (!LLVMGetBasicBlockTerminator(current_block)) {
    LLVMBuildBr(codegen->builder, cond_block);
  }

  // Pop loop context
  pop_loop_context(codegen);

  // Continue with exit block
  LLVMPositionBuilderAtEnd(codegen->builder, exit_block);

  return NULL; // Control flow statements don't return values
}

LLVMValueRef codegen_switch(CodeGen *codegen, ASTNode *switch_stmt) {
  // Generate switch expression
  LLVMValueRef switch_value =
      codegen_expression(codegen, switch_stmt->data.switch_stmt.expression);
  if (!switch_value)
    return NULL;

  // Create basic blocks
  LLVMValueRef function = codegen->current_function;
  LLVMBasicBlockRef default_block = NULL;
  LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(function, "switch_exit");

  // Create default block (will be used even if no explicit default)
  if (switch_stmt->data.switch_stmt.default_case) {
    default_block = LLVMAppendBasicBlock(function, "switch_default");
  } else {
    default_block = exit_block; // No explicit default, jump to exit
  }

  // Create case blocks
  LLVMBasicBlockRef *case_blocks = malloc(
      sizeof(LLVMBasicBlockRef) * switch_stmt->data.switch_stmt.case_count);
  for (int i = 0; i < switch_stmt->data.switch_stmt.case_count; i++) {
    char block_name[64];
    sprintf(block_name, "switch_case_%d", i);
    case_blocks[i] = LLVMAppendBasicBlock(function, block_name);
  }

  // Create the switch instruction
  LLVMValueRef switch_instr =
      LLVMBuildSwitch(codegen->builder, switch_value, default_block,
                      switch_stmt->data.switch_stmt.case_count);

  // Add cases to switch instruction and generate case bodies
  for (int i = 0; i < switch_stmt->data.switch_stmt.case_count; i++) {
    ASTNode *case_node = switch_stmt->data.switch_stmt.cases[i];

    // Add case to switch instruction
    LLVMValueRef case_value =
        codegen_expression(codegen, case_node->data.switch_case.value);
    LLVMAddCase(switch_instr, case_value, case_blocks[i]);

    // Generate case body
    LLVMPositionBuilderAtEnd(codegen->builder, case_blocks[i]);
    for (int j = 0; j < case_node->data.switch_case.statement_count; j++) {
      codegen_statement(codegen, case_node->data.switch_case.statements[j]);
    }
    // Only add branch if block is not already terminated
    if (!LLVMGetBasicBlockTerminator(case_blocks[i])) {
      LLVMBuildBr(codegen->builder, exit_block);
    }
  }

  // Generate default case if exists
  if (switch_stmt->data.switch_stmt.default_case &&
      default_block != exit_block) {
    LLVMPositionBuilderAtEnd(codegen->builder, default_block);
    ASTNode *default_case = switch_stmt->data.switch_stmt.default_case;
    for (int i = 0; i < default_case->data.switch_case.statement_count; i++) {
      codegen_statement(codegen, default_case->data.switch_case.statements[i]);
    }
    // Only add branch if block is not already terminated
    if (!LLVMGetBasicBlockTerminator(default_block)) {
      LLVMBuildBr(codegen->builder, exit_block);
    }
  }

  // Continue with exit block
  LLVMPositionBuilderAtEnd(codegen->builder, exit_block);

  free(case_blocks);
  return NULL; // Control flow statements don't return values
}

LLVMValueRef codegen_match(CodeGen *codegen, ASTNode *match_stmt) {
  // Generate match expression
  LLVMValueRef match_value =
      codegen_expression(codegen, match_stmt->data.match_stmt.expression);
  if (!match_value)
    return NULL;

  // Create basic blocks
  LLVMValueRef function = codegen->current_function;
  LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(function, "match_exit");
  LLVMBasicBlockRef default_block =
      exit_block; // Default to exit if no wildcard

  // Create case blocks
  LLVMBasicBlockRef *case_blocks = malloc(
      sizeof(LLVMBasicBlockRef) * match_stmt->data.match_stmt.case_count);
  int wildcard_index = -1;

  for (int i = 0; i < match_stmt->data.match_stmt.case_count; i++) {
    char block_name[64];
    sprintf(block_name, "match_case_%d", i);
    case_blocks[i] = LLVMAppendBasicBlock(function, block_name);

    // Check if this is a wildcard pattern
    ASTNode *pattern =
        match_stmt->data.match_stmt.cases[i]->data.match_case.pattern;
    if (pattern->type == NODE_IDENTIFIER &&
        strcmp(pattern->data.identifier.name, "_") == 0) {
      wildcard_index = i;
      default_block = case_blocks[i];
    }
  }

  // Create the switch instruction (match is implemented as switch)
  LLVMValueRef switch_instr = LLVMBuildSwitch(
      codegen->builder, match_value, default_block,
      match_stmt->data.match_stmt.case_count - (wildcard_index >= 0 ? 1 : 0));

  // Add cases to switch instruction and generate case bodies
  for (int i = 0; i < match_stmt->data.match_stmt.case_count; i++) {
    ASTNode *case_node = match_stmt->data.match_stmt.cases[i];
    ASTNode *pattern = case_node->data.match_case.pattern;

    // Skip wildcard patterns for switch instruction (they're handled as
    // default)
    if (pattern->type == NODE_IDENTIFIER &&
        strcmp(pattern->data.identifier.name, "_") == 0) {
      // Generate wildcard case body
      LLVMPositionBuilderAtEnd(codegen->builder, case_blocks[i]);
      codegen_statement(codegen, case_node->data.match_case.body);
      // Only add branch if block is not already terminated
      if (!LLVMGetBasicBlockTerminator(case_blocks[i])) {
        LLVMBuildBr(codegen->builder, exit_block);
      }
      continue;
    }

    // Add case to switch instruction
    LLVMValueRef case_value = codegen_expression(codegen, pattern);
    LLVMAddCase(switch_instr, case_value, case_blocks[i]);

    // Generate case body
    LLVMPositionBuilderAtEnd(codegen->builder, case_blocks[i]);
    codegen_statement(codegen, case_node->data.match_case.body);
    // Only add branch if block is not already terminated
    if (!LLVMGetBasicBlockTerminator(case_blocks[i])) {
      LLVMBuildBr(codegen->builder, exit_block);
    }
  }

  // Continue with exit block
  LLVMPositionBuilderAtEnd(codegen->builder, exit_block);

  free(case_blocks);
  return NULL; // Control flow statements don't return values
}

void push_loop_context(CodeGen *codegen, LLVMBasicBlockRef break_target,
                       LLVMBasicBlockRef continue_target) {
  if (codegen->loop_depth < 32) {
    codegen->loop_stack[codegen->loop_depth].break_target = break_target;
    codegen->loop_stack[codegen->loop_depth].continue_target = continue_target;
    codegen->loop_depth++;
  }
}

void pop_loop_context(CodeGen *codegen) {
  if (codegen->loop_depth > 0) {
    codegen->loop_depth--;
  }
}

LLVMValueRef codegen_break(CodeGen *codegen, ASTNode *break_stmt) {
  (void)break_stmt; // Unused parameter

  if (codegen->loop_depth == 0) {
    // Error: break outside of loop
    codegen->has_error = 1;
    return NULL;
  }

  // Jump to the break target of the current loop
  LLVMBasicBlockRef break_target =
      codegen->loop_stack[codegen->loop_depth - 1].break_target;
  LLVMBuildBr(codegen->builder, break_target);

  return NULL;
}

LLVMValueRef codegen_continue(CodeGen *codegen, ASTNode *continue_stmt) {
  (void)continue_stmt; // Unused parameter

  if (codegen->loop_depth == 0) {
    // Error: continue outside of loop
    codegen->has_error = 1;
    return NULL;
  }

  // Jump to the continue target of the current loop
  LLVMBasicBlockRef continue_target =
      codegen->loop_stack[codegen->loop_depth - 1].continue_target;
  LLVMBuildBr(codegen->builder, continue_target);

  return NULL;
}
