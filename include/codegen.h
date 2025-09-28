#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include "ast.h"
#include "types.h"

typedef struct {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMValueRef current_function;
    
    // Symbol table for variables (simple array for now)
    struct {
        char *name;
        LLVMValueRef value;
        LLVMTypeRef type;
        int is_mutable;
        TypeKind type_kind;  // Semantic type information
    } variables[256];
    int variable_count;
    
    // Function table
    struct {
        char *name;
        LLVMValueRef function;
    } functions[256];
    int function_count;
    
    // Loop context stack for break/continue
    struct {
        LLVMBasicBlockRef break_target;    // Where to jump on break
        LLVMBasicBlockRef continue_target; // Where to jump on continue
    } loop_stack[32];
    int loop_depth;
    
    // Error flag for stopping compilation
    int has_error;
} CodeGen;

// CodeGen functions
CodeGen *create_codegen(const char *module_name);
void free_codegen(CodeGen *codegen);

// Code generation functions
LLVMValueRef codegen_program(CodeGen *codegen, ASTNode *program);
LLVMValueRef codegen_function(CodeGen *codegen, ASTNode *function);
LLVMValueRef codegen_block(CodeGen *codegen, ASTNode *block);
LLVMValueRef codegen_statement(CodeGen *codegen, ASTNode *statement);
LLVMValueRef codegen_expression(CodeGen *codegen, ASTNode *expression);
LLVMValueRef codegen_variable_decl(CodeGen *codegen, ASTNode *var_decl);
LLVMValueRef codegen_assignment(CodeGen *codegen, ASTNode *assignment);
LLVMValueRef codegen_pointer_assignment(CodeGen *codegen, ASTNode *assignment);
LLVMValueRef codegen_return(CodeGen *codegen, ASTNode *return_stmt);
LLVMValueRef codegen_call(CodeGen *codegen, ASTNode *call);
LLVMValueRef codegen_literal(CodeGen *codegen, ASTNode *literal);
LLVMValueRef codegen_identifier(CodeGen *codegen, ASTNode *identifier);
LLVMValueRef codegen_binary_op(CodeGen *codegen, ASTNode *binary_op);
LLVMValueRef codegen_unary_op(CodeGen *codegen, ASTNode *unary_op);
LLVMValueRef codegen_struct(CodeGen *codegen, ASTNode *struct_decl);
LLVMValueRef codegen_struct_method(CodeGen *codegen, ASTNode *method, const char *struct_name, LLVMTypeRef struct_type);
LLVMValueRef codegen_field_access(CodeGen *codegen, ASTNode *field_access);
LLVMValueRef codegen_method_call(CodeGen *codegen, ASTNode *method_call);
LLVMValueRef codegen_struct_literal(CodeGen *codegen, ASTNode *struct_literal);
LLVMValueRef codegen_if(CodeGen *codegen, ASTNode *if_stmt);
LLVMValueRef codegen_unless(CodeGen *codegen, ASTNode *unless_stmt);
LLVMValueRef codegen_for(CodeGen *codegen, ASTNode *for_stmt);
LLVMValueRef codegen_while(CodeGen *codegen, ASTNode *while_stmt);
LLVMValueRef codegen_switch(CodeGen *codegen, ASTNode *switch_stmt);
LLVMValueRef codegen_match(CodeGen *codegen, ASTNode *match_stmt);
LLVMValueRef codegen_break(CodeGen *codegen, ASTNode *break_stmt);
LLVMValueRef codegen_continue(CodeGen *codegen, ASTNode *continue_stmt);

// Helper functions
void add_builtin_functions(CodeGen *codegen);
LLVMValueRef codegen_std_print(CodeGen *codegen, ASTNode *call);
LLVMValueRef codegen_std_println(CodeGen *codegen, ASTNode *call);
LLVMValueRef codegen_cast(CodeGen *codegen, ASTNode *call);
LLVMTypeRef get_llvm_type(CodeGen *codegen, const char *type_name);
LLVMTypeRef get_llvm_type_from_kind(CodeGen *codegen, TypeKind type_kind);
LLVMValueRef get_variable(CodeGen *codegen, const char *name);
LLVMTypeRef get_variable_type(CodeGen *codegen, const char *name);
int get_variable_mutability(CodeGen *codegen, const char *name);
void set_variable(CodeGen *codegen, const char *name, LLVMValueRef value, LLVMTypeRef type, int is_mutable);
void set_variable_with_type(CodeGen *codegen, const char *name, LLVMValueRef value, LLVMTypeRef type, int is_mutable, TypeKind type_kind);
TypeKind get_variable_type_kind(CodeGen *codegen, const char *name);
TypeKind get_expression_type(CodeGen *codegen, ASTNode *node);
LLVMValueRef get_function(CodeGen *codegen, const char *name);
void set_function(CodeGen *codegen, const char *name, LLVMValueRef function);
void process_local_import(CodeGen *codegen, ASTNode *import);
void process_external_import(CodeGen *codegen, ASTNode *import);
void push_loop_context(CodeGen *codegen, LLVMBasicBlockRef break_target, LLVMBasicBlockRef continue_target);
void pop_loop_context(CodeGen *codegen);

// Output functions
void print_llvm_ir(CodeGen *codegen);
int write_object_file(CodeGen *codegen, const char *filename);
int write_executable(CodeGen *codegen, const char *filename);

#endif
