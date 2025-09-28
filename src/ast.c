#define _GNU_SOURCE
#include "ast.h"
#include "types.h"
#include <stdio.h>

ASTNode *create_program_node(void) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_PROGRAM;
  node->data.program.imports = NULL;
  node->data.program.functions = NULL;
  node->data.program.import_count = 0;
  node->data.program.function_count = 0;
  return node;
}

ASTNode *create_import_node(ImportType type, const char *path) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_IMPORT;
  node->data.import.import_type = type;
  node->data.import.path = strdup(path);
  return node;
}

ASTNode *create_function_node(const char *name, const char *return_type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_FUNCTION;
  node->data.function.name = strdup(name);
  node->data.function.return_type = strdup(return_type);
  node->data.function.params = NULL;
  node->data.function.body = NULL;
  node->data.function.param_count = 0;
  return node;
}

ASTNode *create_parameter_node(const char *name, const char *type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_PARAMETER;
  node->data.parameter.name = strdup(name);
  node->data.parameter.type = strdup(type);
  node->data.parameter.resolved_type = string_to_type(type);
  return node;
}

ASTNode *create_variable_decl_node(const char *name, const char *type,
                                   ASTNode *value, int is_mutable) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_VARIABLE_DECL;
  node->data.variable_decl.name = strdup(name);
  node->data.variable_decl.type = strdup(type);
  node->data.variable_decl.value = value;
  node->data.variable_decl.is_mutable = is_mutable;
  node->data.variable_decl.resolved_type = string_to_type(type);
  return node;
}

ASTNode *create_assignment_node(const char *variable_name, ASTNode *value) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_ASSIGNMENT;
  node->data.assignment.variable_name = strdup(variable_name);
  node->data.assignment.value = value;
  return node;
}

ASTNode *create_pointer_assignment_node(ASTNode *target, ASTNode *value) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_POINTER_ASSIGNMENT;
  node->data.pointer_assignment.target = target;
  node->data.pointer_assignment.value = value;
  return node;
}

ASTNode *create_return_node(ASTNode *value) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_RETURN;
  node->data.return_stmt.value = value;
  return node;
}

ASTNode *create_call_node(const char *name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_CALL;
  node->data.call.name = strdup(name);
  node->data.call.args = NULL;
  node->data.call.arg_count = 0;
  return node;
}

ASTNode *create_identifier_node(const char *name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_IDENTIFIER;
  node->data.identifier.name = strdup(name);
  node->data.identifier.resolved_type = TYPE_UNKNOWN; // Will be resolved later
  return node;
}

ASTNode *create_literal_node(const char *value, const char *type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_LITERAL;
  node->data.literal.value = strdup(value);
  node->data.literal.type = strdup(type);
  node->data.literal.resolved_type = string_to_type(type);
  return node;
}

ASTNode *create_binary_op_node(BinaryOperator operator, ASTNode *left,
                               ASTNode *right) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_BINARY_OP;
  node->data.binary_op.operator = operator;
  node->data.binary_op.left = left;
  node->data.binary_op.right = right;
  node->data.binary_op.resolved_type = TYPE_UNKNOWN; // Will be resolved later
  return node;
}

ASTNode *create_unary_op_node(UnaryOperator operator, ASTNode *operand) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_UNARY_OP;
  node->data.unary_op.operator = operator;
  node->data.unary_op.operand = operand;
  node->data.unary_op.resolved_type = TYPE_UNKNOWN; // Will be resolved later
  return node;
}

ASTNode *create_block_node(void) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_BLOCK;
  node->data.block.statements = NULL;
  node->data.block.statement_count = 0;
  return node;
}

ASTNode *create_struct_node(const char *name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_STRUCT;
  node->data.struct_decl.name = strdup(name);
  node->data.struct_decl.fields = NULL;
  node->data.struct_decl.methods = NULL;
  node->data.struct_decl.field_count = 0;
  node->data.struct_decl.method_count = 0;
  return node;
}

ASTNode *create_struct_field_node(const char *name, const char *type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_STRUCT_FIELD;
  node->data.struct_field.name = strdup(name);
  node->data.struct_field.type = strdup(type);
  node->data.struct_field.resolved_type = TYPE_UNKNOWN;
  return node;
}

ASTNode *create_struct_method_node(const char *name, const char *return_type,
                                   Visibility visibility) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_STRUCT_METHOD;
  node->data.struct_method.name = strdup(name);
  node->data.struct_method.return_type = strdup(return_type);
  node->data.struct_method.params = NULL;
  node->data.struct_method.body = NULL;
  node->data.struct_method.param_count = 0;
  node->data.struct_method.visibility = visibility;
  return node;
}

ASTNode *create_field_access_node(ASTNode *object, const char *field_name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_FIELD_ACCESS;
  node->data.field_access.object = object;
  node->data.field_access.field_name = strdup(field_name);
  node->data.field_access.resolved_type = TYPE_UNKNOWN;
  return node;
}

ASTNode *create_method_call_node(ASTNode *object, const char *method_name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_METHOD_CALL;
  node->data.method_call.object = object;
  node->data.method_call.method_name = strdup(method_name);
  node->data.method_call.args = NULL;
  node->data.method_call.arg_count = 0;
  node->data.method_call.resolved_type = TYPE_UNKNOWN;
  return node;
}

ASTNode *create_struct_literal_node(const char *struct_type_name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_STRUCT_LITERAL;
  node->data.struct_literal.struct_type_name = strdup(struct_type_name);
  node->data.struct_literal.field_values = NULL;
  node->data.struct_literal.field_names = NULL;
  node->data.struct_literal.field_count = 0;
  node->data.struct_literal.resolved_type = TYPE_UNKNOWN;
  return node;
}

ASTNode *create_enum_node(const char *name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_ENUM;
  node->data.enum_decl.name = strdup(name);
  node->data.enum_decl.variants = NULL;
  node->data.enum_decl.variant_count = 0;
  return node;
}

ASTNode *create_enum_variant_node(const char *name) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_ENUM_VARIANT;
  node->data.enum_variant.name = strdup(name);
  return node;
}

ASTNode *create_if_node(ASTNode *condition, ASTNode *then_block,
                        ASTNode *else_block) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_IF;
  node->data.if_stmt.condition = condition;
  node->data.if_stmt.then_block = then_block;
  node->data.if_stmt.else_block = else_block;
  return node;
}

ASTNode *create_unless_node(ASTNode *condition, ASTNode *then_block,
                            ASTNode *else_block) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_UNLESS;
  node->data.unless_stmt.condition = condition;
  node->data.unless_stmt.then_block = then_block;
  node->data.unless_stmt.else_block = else_block;
  return node;
}

ASTNode *create_for_node(ASTNode *init, ASTNode *condition, ASTNode *update,
                         ASTNode *body) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_FOR;
  node->data.for_stmt.init = init;
  node->data.for_stmt.condition = condition;
  node->data.for_stmt.update = update;
  node->data.for_stmt.body = body;
  return node;
}

ASTNode *create_while_node(ASTNode *condition, ASTNode *body) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_WHILE;
  node->data.while_stmt.condition = condition;
  node->data.while_stmt.body = body;
  return node;
}

ASTNode *create_switch_node(ASTNode *expression) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_SWITCH;
  node->data.switch_stmt.expression = expression;
  node->data.switch_stmt.cases = NULL;
  node->data.switch_stmt.case_count = 0;
  node->data.switch_stmt.default_case = NULL;
  return node;
}

ASTNode *create_switch_case_node(ASTNode *value) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_SWITCH_CASE;
  node->data.switch_case.value = value;
  node->data.switch_case.statements = NULL;
  node->data.switch_case.statement_count = 0;
  return node;
}

ASTNode *create_match_node(ASTNode *expression) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_MATCH;
  node->data.match_stmt.expression = expression;
  node->data.match_stmt.cases = NULL;
  node->data.match_stmt.case_count = 0;
  return node;
}

ASTNode *create_match_case_node(ASTNode *pattern, ASTNode *body) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_MATCH_CASE;
  node->data.match_case.pattern = pattern;
  node->data.match_case.body = body;
  return node;
}

ASTNode *create_break_node(void) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_BREAK;
  return node;
}

ASTNode *create_continue_node(void) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = NODE_CONTINUE;
  return node;
}

void add_import_to_program(ASTNode *program, ASTNode *import) {
  program->data.program.import_count++;
  program->data.program.imports =
      realloc(program->data.program.imports,
              sizeof(ASTNode *) * program->data.program.import_count);
  program->data.program.imports[program->data.program.import_count - 1] =
      import;
}

void add_function_to_program(ASTNode *program, ASTNode *function) {
  program->data.program.function_count++;
  program->data.program.functions =
      realloc(program->data.program.functions,
              sizeof(ASTNode *) * program->data.program.function_count);
  program->data.program.functions[program->data.program.function_count - 1] =
      function;
}

void add_parameter_to_function(ASTNode *function, ASTNode *parameter) {
  function->data.function.param_count++;
  function->data.function.params =
      realloc(function->data.function.params,
              sizeof(ASTNode *) * function->data.function.param_count);
  function->data.function.params[function->data.function.param_count - 1] =
      parameter;
}

void add_statement_to_block(ASTNode *block, ASTNode *statement) {
  block->data.block.statement_count++;
  block->data.block.statements =
      realloc(block->data.block.statements,
              sizeof(ASTNode *) * block->data.block.statement_count);
  block->data.block.statements[block->data.block.statement_count - 1] =
      statement;
}

void add_arg_to_call(ASTNode *call, ASTNode *arg) {
  call->data.call.arg_count++;
  call->data.call.args = realloc(call->data.call.args,
                                 sizeof(ASTNode *) * call->data.call.arg_count);
  call->data.call.args[call->data.call.arg_count - 1] = arg;
}

void add_field_to_struct(ASTNode *struct_node, ASTNode *field) {
  struct_node->data.struct_decl.field_count++;
  struct_node->data.struct_decl.fields =
      realloc(struct_node->data.struct_decl.fields,
              sizeof(ASTNode *) * struct_node->data.struct_decl.field_count);
  struct_node->data.struct_decl
      .fields[struct_node->data.struct_decl.field_count - 1] = field;
}

void add_method_to_struct(ASTNode *struct_node, ASTNode *method) {
  struct_node->data.struct_decl.method_count++;
  struct_node->data.struct_decl.methods =
      realloc(struct_node->data.struct_decl.methods,
              sizeof(ASTNode *) * struct_node->data.struct_decl.method_count);
  struct_node->data.struct_decl
      .methods[struct_node->data.struct_decl.method_count - 1] = method;
}

void add_parameter_to_struct_method(ASTNode *method, ASTNode *parameter) {
  method->data.struct_method.param_count++;
  method->data.struct_method.params =
      realloc(method->data.struct_method.params,
              sizeof(ASTNode *) * method->data.struct_method.param_count);
  method->data.struct_method
      .params[method->data.struct_method.param_count - 1] = parameter;
}

void add_arg_to_method_call(ASTNode *method_call, ASTNode *arg) {
  method_call->data.method_call.arg_count++;
  method_call->data.method_call.args =
      realloc(method_call->data.method_call.args,
              sizeof(ASTNode *) * method_call->data.method_call.arg_count);
  method_call->data.method_call
      .args[method_call->data.method_call.arg_count - 1] = arg;
}

void add_field_to_struct_literal(ASTNode *struct_literal,
                                 const char *field_name, ASTNode *value) {
  int count = struct_literal->data.struct_literal.field_count;
  struct_literal->data.struct_literal.field_count++;

  struct_literal->data.struct_literal.field_values =
      realloc(struct_literal->data.struct_literal.field_values,
              sizeof(ASTNode *) * (count + 1));
  struct_literal->data.struct_literal.field_names =
      realloc(struct_literal->data.struct_literal.field_names,
              sizeof(char *) * (count + 1));

  struct_literal->data.struct_literal.field_values[count] = value;
  struct_literal->data.struct_literal.field_names[count] = strdup(field_name);
}

void add_enum_variant(ASTNode *enum_node, ASTNode *variant) {
  int count = enum_node->data.enum_decl.variant_count;
  enum_node->data.enum_decl.variant_count++;

  enum_node->data.enum_decl.variants =
      realloc(enum_node->data.enum_decl.variants,
              sizeof(ASTNode *) * (count + 1));
  enum_node->data.enum_decl.variants[count] = variant;
}

void add_case_to_switch(ASTNode *switch_stmt, ASTNode *switch_case) {
  switch_stmt->data.switch_stmt.case_count++;
  switch_stmt->data.switch_stmt.cases =
      realloc(switch_stmt->data.switch_stmt.cases,
              sizeof(ASTNode *) * switch_stmt->data.switch_stmt.case_count);
  switch_stmt->data.switch_stmt
      .cases[switch_stmt->data.switch_stmt.case_count - 1] = switch_case;
}

void add_statement_to_switch_case(ASTNode *switch_case, ASTNode *statement) {
  switch_case->data.switch_case.statement_count++;
  switch_case->data.switch_case.statements = realloc(
      switch_case->data.switch_case.statements,
      sizeof(ASTNode *) * switch_case->data.switch_case.statement_count);
  switch_case->data.switch_case
      .statements[switch_case->data.switch_case.statement_count - 1] =
      statement;
}

void add_case_to_match(ASTNode *match_stmt, ASTNode *match_case) {
  match_stmt->data.match_stmt.case_count++;
  match_stmt->data.match_stmt.cases =
      realloc(match_stmt->data.match_stmt.cases,
              sizeof(ASTNode *) * match_stmt->data.match_stmt.case_count);
  match_stmt->data.match_stmt
      .cases[match_stmt->data.match_stmt.case_count - 1] = match_case;
}

void set_switch_default(ASTNode *switch_stmt, ASTNode *default_case) {
  switch_stmt->data.switch_stmt.default_case = default_case;
}

void free_ast_node(ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case NODE_PROGRAM:
    for (int i = 0; i < node->data.program.import_count; i++) {
      free_ast_node(node->data.program.imports[i]);
    }
    for (int i = 0; i < node->data.program.function_count; i++) {
      free_ast_node(node->data.program.functions[i]);
    }
    free(node->data.program.imports);
    free(node->data.program.functions);
    break;
  case NODE_IMPORT:
    free(node->data.import.path);
    break;
  case NODE_PARAMETER:
    free(node->data.parameter.name);
    free(node->data.parameter.type);
    break;
  case NODE_FUNCTION:
    free(node->data.function.name);
    free(node->data.function.return_type);
    for (int i = 0; i < node->data.function.param_count; i++) {
      free_ast_node(node->data.function.params[i]);
    }
    free(node->data.function.params);
    free_ast_node(node->data.function.body);
    break;
  case NODE_VARIABLE_DECL:
    free(node->data.variable_decl.name);
    free(node->data.variable_decl.type);
    free_ast_node(node->data.variable_decl.value);
    break;
  case NODE_ASSIGNMENT:
    free(node->data.assignment.variable_name);
    free_ast_node(node->data.assignment.value);
    break;
  case NODE_POINTER_ASSIGNMENT:
    free_ast_node(node->data.pointer_assignment.target);
    free_ast_node(node->data.pointer_assignment.value);
    break;
  case NODE_RETURN:
    free_ast_node(node->data.return_stmt.value);
    break;
  case NODE_CALL:
    free(node->data.call.name);
    for (int i = 0; i < node->data.call.arg_count; i++) {
      free_ast_node(node->data.call.args[i]);
    }
    free(node->data.call.args);
    break;
  case NODE_IDENTIFIER:
    free(node->data.identifier.name);
    break;
  case NODE_LITERAL:
    free(node->data.literal.value);
    free(node->data.literal.type);
    break;
  case NODE_BINARY_OP:
    free_ast_node(node->data.binary_op.left);
    free_ast_node(node->data.binary_op.right);
    break;
  case NODE_UNARY_OP:
    free_ast_node(node->data.unary_op.operand);
    break;
  case NODE_BLOCK:
    for (int i = 0; i < node->data.block.statement_count; i++) {
      free_ast_node(node->data.block.statements[i]);
    }
    free(node->data.block.statements);
    break;
  case NODE_STRUCT:
    free(node->data.struct_decl.name);
    for (int i = 0; i < node->data.struct_decl.field_count; i++) {
      free_ast_node(node->data.struct_decl.fields[i]);
    }
    for (int i = 0; i < node->data.struct_decl.method_count; i++) {
      free_ast_node(node->data.struct_decl.methods[i]);
    }
    free(node->data.struct_decl.fields);
    free(node->data.struct_decl.methods);
    break;
  case NODE_STRUCT_FIELD:
    free(node->data.struct_field.name);
    free(node->data.struct_field.type);
    break;
  case NODE_STRUCT_METHOD:
    free(node->data.struct_method.name);
    free(node->data.struct_method.return_type);
    for (int i = 0; i < node->data.struct_method.param_count; i++) {
      free_ast_node(node->data.struct_method.params[i]);
    }
    free(node->data.struct_method.params);
    free_ast_node(node->data.struct_method.body);
    break;
  case NODE_FIELD_ACCESS:
    free_ast_node(node->data.field_access.object);
    free(node->data.field_access.field_name);
    break;
  case NODE_METHOD_CALL:
    free_ast_node(node->data.method_call.object);
    free(node->data.method_call.method_name);
    for (int i = 0; i < node->data.method_call.arg_count; i++) {
      free_ast_node(node->data.method_call.args[i]);
    }
    free(node->data.method_call.args);
    break;
  case NODE_STRUCT_LITERAL:
    free(node->data.struct_literal.struct_type_name);
    for (int i = 0; i < node->data.struct_literal.field_count; i++) {
      free_ast_node(node->data.struct_literal.field_values[i]);
      free(node->data.struct_literal.field_names[i]);
    }
    free(node->data.struct_literal.field_values);
    free(node->data.struct_literal.field_names);
    break;
  case NODE_IF:
    free_ast_node(node->data.if_stmt.condition);
    free_ast_node(node->data.if_stmt.then_block);
    if (node->data.if_stmt.else_block) {
      free_ast_node(node->data.if_stmt.else_block);
    }
    break;
  case NODE_UNLESS:
    free_ast_node(node->data.unless_stmt.condition);
    free_ast_node(node->data.unless_stmt.then_block);
    if (node->data.unless_stmt.else_block) {
      free_ast_node(node->data.unless_stmt.else_block);
    }
    break;
  case NODE_FOR:
    if (node->data.for_stmt.init) {
      free_ast_node(node->data.for_stmt.init);
    }
    if (node->data.for_stmt.condition) {
      free_ast_node(node->data.for_stmt.condition);
    }
    if (node->data.for_stmt.update) {
      free_ast_node(node->data.for_stmt.update);
    }
    free_ast_node(node->data.for_stmt.body);
    break;
  case NODE_WHILE:
    free_ast_node(node->data.while_stmt.condition);
    free_ast_node(node->data.while_stmt.body);
    break;
  case NODE_SWITCH:
    free_ast_node(node->data.switch_stmt.expression);
    for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
      free_ast_node(node->data.switch_stmt.cases[i]);
    }
    free(node->data.switch_stmt.cases);
    if (node->data.switch_stmt.default_case) {
      free_ast_node(node->data.switch_stmt.default_case);
    }
    break;
  case NODE_SWITCH_CASE:
    free_ast_node(node->data.switch_case.value);
    for (int i = 0; i < node->data.switch_case.statement_count; i++) {
      free_ast_node(node->data.switch_case.statements[i]);
    }
    free(node->data.switch_case.statements);
    break;
  case NODE_MATCH:
    free_ast_node(node->data.match_stmt.expression);
    for (int i = 0; i < node->data.match_stmt.case_count; i++) {
      free_ast_node(node->data.match_stmt.cases[i]);
    }
    free(node->data.match_stmt.cases);
    break;
  case NODE_MATCH_CASE:
    free_ast_node(node->data.match_case.pattern);
    free_ast_node(node->data.match_case.body);
    break;
  case NODE_BREAK:
  case NODE_CONTINUE:
    // No sub-nodes to free
    break;
  }
  free(node);
}

// Type analysis functions
void resolve_types(ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case NODE_PROGRAM:
    for (int i = 0; i < node->data.program.function_count; i++) {
      resolve_types(node->data.program.functions[i]);
    }
    break;
  case NODE_FUNCTION:
    // Resolve parameter types
    for (int i = 0; i < node->data.function.param_count; i++) {
      resolve_types(node->data.function.params[i]);
    }
    // Resolve body
    resolve_types(node->data.function.body);
    break;
  case NODE_BLOCK:
    for (int i = 0; i < node->data.block.statement_count; i++) {
      resolve_types(node->data.block.statements[i]);
    }
    break;
  case NODE_VARIABLE_DECL:
    // Re-resolve variable type in case it's a struct type that was registered
    // after creation
    node->data.variable_decl.resolved_type =
        string_to_type(node->data.variable_decl.type);
    if (node->data.variable_decl.value) {
      resolve_types(node->data.variable_decl.value);
    }
    break;
  case NODE_ASSIGNMENT:
    resolve_types(node->data.assignment.value);
    break;
  case NODE_POINTER_ASSIGNMENT:
    resolve_types(node->data.pointer_assignment.target);
    resolve_types(node->data.pointer_assignment.value);
    break;
  case NODE_RETURN:
    if (node->data.return_stmt.value) {
      resolve_types(node->data.return_stmt.value);
    }
    break;
  case NODE_CALL:
    for (int i = 0; i < node->data.call.arg_count; i++) {
      resolve_types(node->data.call.args[i]);
    }
    break;
  case NODE_BINARY_OP:
    // Resolve operand types first
    resolve_types(node->data.binary_op.left);
    resolve_types(node->data.binary_op.right);

    // Determine result type
    TypeKind left_type = get_node_type(node->data.binary_op.left);
    TypeKind right_type = get_node_type(node->data.binary_op.right);

    // Check if comparison operation
    int is_comparison = (node->data.binary_op.operator >= OP_EQ &&
                         node->data.binary_op.operator <= OP_GE);

    node->data.binary_op.resolved_type =
        get_binary_result_type(left_type, right_type, is_comparison);
    break;
  case NODE_UNARY_OP:
    // Resolve operand type first
    resolve_types(node->data.unary_op.operand);

    // Determine result type based on unary operation
    TypeKind operand_type = get_node_type(node->data.unary_op.operand);
    if (node->data.unary_op.operator == UNARY_ADDRESS_OF) {
      // Address-of returns pointer to operand type
      node->data.unary_op.resolved_type = make_pointer_type(operand_type);
    } else if (node->data.unary_op.operator == UNARY_DEREFERENCE) {
      // Dereference returns the pointed-to type
      if (is_pointer_type(operand_type)) {
        node->data.unary_op.resolved_type = get_pointed_type(operand_type);
      } else {
        node->data.unary_op.resolved_type = TYPE_UNKNOWN; // Error case
      }
    }
    break;
  case NODE_LITERAL:
    // Literal type already resolved during creation
    break;
  case NODE_IDENTIFIER:
    // Identifier type will be resolved during semantic analysis
    // For now, mark as unknown - this should be resolved by a symbol table
    break;
  case NODE_STRUCT: {
    // First resolve field types
    for (int i = 0; i < node->data.struct_decl.field_count; i++) {
      resolve_types(node->data.struct_decl.fields[i]);
    }

    // Register the struct type
    StructField *fields =
        malloc(node->data.struct_decl.field_count * sizeof(StructField));
    for (int i = 0; i < node->data.struct_decl.field_count; i++) {
      ASTNode *field = node->data.struct_decl.fields[i];
      fields[i].name = field->data.struct_field.name;
      fields[i].type = field->data.struct_field.resolved_type;
      fields[i].offset = 0; // Will be calculated in register_struct_type
    }

    TypeKind struct_type_id =
        register_struct_type(node->data.struct_decl.name, fields,
                             node->data.struct_decl.field_count);

    free(fields); // register_struct_type makes its own copy

    // Resolve method types
    for (int i = 0; i < node->data.struct_decl.method_count; i++) {
      resolve_types(node->data.struct_decl.methods[i]);
    }
    break;
  }
  case NODE_STRUCT_FIELD:
    // Resolve the field type from string to TypeKind
    node->data.struct_field.resolved_type =
        string_to_type(node->data.struct_field.type);
    break;
  case NODE_STRUCT_METHOD:
    for (int i = 0; i < node->data.struct_method.param_count; i++) {
      resolve_types(node->data.struct_method.params[i]);
    }
    if (node->data.struct_method.body) {
      resolve_types(node->data.struct_method.body);
    }
    break;
  case NODE_FIELD_ACCESS: {
    // Resolve the object type first
    resolve_types(node->data.field_access.object);
    TypeKind object_type = get_node_type(node->data.field_access.object);

    // Get the field type from the struct
    if (is_struct_type(object_type)) {
      TypeKind field_type =
          get_field_type(object_type, node->data.field_access.field_name);
      node->data.field_access.resolved_type = field_type;
    } else {
      node->data.field_access.resolved_type = TYPE_UNKNOWN;
    }
    break;
  }
  case NODE_METHOD_CALL:
    // Resolve the object and arguments
    resolve_types(node->data.method_call.object);
    for (int i = 0; i < node->data.method_call.arg_count; i++) {
      resolve_types(node->data.method_call.args[i]);
    }
    // Method call type resolution would need method signature lookup
    // For now, mark as unknown
    node->data.method_call.resolved_type = TYPE_UNKNOWN;
    break;
  case NODE_STRUCT_LITERAL: {
    // Resolve field values
    for (int i = 0; i < node->data.struct_literal.field_count; i++) {
      resolve_types(node->data.struct_literal.field_values[i]);
    }

    // Resolve the struct type
    StructType *st =
        find_struct_by_name(node->data.struct_literal.struct_type_name);
    if (st) {
      node->data.struct_literal.resolved_type = st->type_id;
    } else {
      node->data.struct_literal.resolved_type = TYPE_UNKNOWN;
    }
    break;
  }
  case NODE_IF:
    resolve_types(node->data.if_stmt.condition);
    resolve_types(node->data.if_stmt.then_block);
    if (node->data.if_stmt.else_block) {
      resolve_types(node->data.if_stmt.else_block);
    }
    break;
  case NODE_UNLESS:
    resolve_types(node->data.unless_stmt.condition);
    resolve_types(node->data.unless_stmt.then_block);
    if (node->data.unless_stmt.else_block) {
      resolve_types(node->data.unless_stmt.else_block);
    }
    break;
  case NODE_FOR:
    if (node->data.for_stmt.init) {
      resolve_types(node->data.for_stmt.init);
    }
    if (node->data.for_stmt.condition) {
      resolve_types(node->data.for_stmt.condition);
    }
    if (node->data.for_stmt.update) {
      resolve_types(node->data.for_stmt.update);
    }
    resolve_types(node->data.for_stmt.body);
    break;
  case NODE_WHILE:
    resolve_types(node->data.while_stmt.condition);
    resolve_types(node->data.while_stmt.body);
    break;
  case NODE_SWITCH:
    resolve_types(node->data.switch_stmt.expression);
    for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
      resolve_types(node->data.switch_stmt.cases[i]);
    }
    if (node->data.switch_stmt.default_case) {
      resolve_types(node->data.switch_stmt.default_case);
    }
    break;
  case NODE_SWITCH_CASE:
    resolve_types(node->data.switch_case.value);
    for (int i = 0; i < node->data.switch_case.statement_count; i++) {
      resolve_types(node->data.switch_case.statements[i]);
    }
    break;
  case NODE_MATCH:
    resolve_types(node->data.match_stmt.expression);
    for (int i = 0; i < node->data.match_stmt.case_count; i++) {
      resolve_types(node->data.match_stmt.cases[i]);
    }
    break;
  case NODE_MATCH_CASE:
    resolve_types(node->data.match_case.pattern);
    resolve_types(node->data.match_case.body);
    break;
  case NODE_BREAK:
  case NODE_CONTINUE:
    // No sub-nodes to resolve
    break;
  default:
    break;
  }
}

TypeKind get_node_type(ASTNode *node) {
  if (!node)
    return TYPE_UNKNOWN;

  switch (node->type) {
  case NODE_LITERAL:
    return node->data.literal.resolved_type;
  case NODE_IDENTIFIER:
    // For identifiers, we need the current symbol table context
    // This is a limitation of our current design - ideally we'd pass the
    // codegen context
    return node->data.identifier.resolved_type;
  case NODE_BINARY_OP:
    return node->data.binary_op.resolved_type;
  case NODE_UNARY_OP:
    return node->data.unary_op.resolved_type;
  case NODE_VARIABLE_DECL:
    return node->data.variable_decl.resolved_type;
  case NODE_PARAMETER:
    return node->data.parameter.resolved_type;
  default:
    return TYPE_UNKNOWN;
  }
}
