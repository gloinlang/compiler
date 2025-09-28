# Gloin v0.0.1 Implementation TODO List

This document outlines the step-by-step tasks needed to make Gloin v0.0.1 fully functional.

## ✅ COMPLETED TASKS

### Phase 1: Documentation & Lexer ✅
- [x] **Fix Gloin0.0.1.md structure** - Proper markdown formatting and organization
- [x] **Lexer: Add new keywords** - const, f32, void, null, self, static, defer, deferred, spawnable, run, enum
- [x] **Lexer: Add new operators** - :: (double colon for static access)
- [x] **Lexer: Add comment support** - // single-line comments
- [x] **Lexer: Add float number support** - 3.14, 42.0 parsing
- [x] **Lexer: Update test suite** - 13/13 tests passing with v0.0.1 features

## 🔧 IN PROGRESS TASKS

### Phase 2: Parser & AST Updates [CRITICAL - BLOCKING COMPILATION]

#### 2.1 AST Node Extensions 🔴 HIGH PRIORITY
- [ ] **Add new AST node types** for v0.0.1 features:
  ```c
  // Add to ast.h NodeType enum:
  NODE_CONST_DECL,     // const declarations
  NODE_DEFER,          // defer statements  
  NODE_STATIC_METHOD,  // static method declarations
  NODE_SELF_ACCESS,    // self.field access
  NODE_ENUM,           // enum declarations
  NODE_ENUM_VARIANT,   // enum variants
  NODE_STATIC_CALL,    // Type::method() calls
  NODE_TUPLE,          // tuple types and literals
  NODE_ELSE,           // else clauses
  ```

#### 2.2 Type System Updates 🔴 HIGH PRIORITY
- [ ] **Add f32 type support** in types.h/types.c:
  - Update TYPE_F32 handling in get_type_info()
  - Add f32 literal parsing and validation
  - Ensure f32 compatibility in type checking

- [ ] **Add void type support**:
  - Update function return type parsing for void
  - Handle void in type compatibility checks

- [ ] **Add tuple type support**:
  - Create tuple type structure in types.h
  - Add tuple literal parsing: (10, 20.5)
  - Implement tuple field access

#### 2.3 Parser Function Updates 🔴 HIGH PRIORITY  
- [ ] **Update parse_program()** to handle:
  - `def const` declarations
  - `enum` declarations
  - `def struct` with static methods

- [ ] **Add parse_const_declaration()**:
  ```c
  // Parse: def const PI: f32 = 3.14159;
  ASTNode *parse_const_declaration(Parser *parser);
  ```

- [ ] **Add parse_enum_declaration()**:
  ```c
  // Parse: enum Color { Red, Green, Blue }
  ASTNode *parse_enum_declaration(Parser *parser);
  ```

- [ ] **Update parse_function()** to handle:
  - `def static` methods
  - `def deferred` functions  
  - `def spawnable` functions
  - `void` return types

- [ ] **Add parse_defer_statement()**:
  ```c
  // Parse: defer cleanup();
  ASTNode *parse_defer_statement(Parser *parser);
  ```

- [ ] **Update parse_primary()** to handle:
  - `null` literals
  - `self` keyword
  - Float literals (3.14)
  - Tuple literals (10, 20)

- [ ] **Add parse_static_call()**:
  ```c
  // Parse: Point::new(10, 20)
  ASTNode *parse_static_call(Parser *parser);
  ```

- [ ] **Update parse_if_statement()** to support `else` clauses

#### 2.4 Parser Test Updates 🟡 MEDIUM PRIORITY
- [ ] **Fix existing parser test failures**:
  - Current issue: "Expected return type at line 1, column 14"
  - Update test cases to use new type tokens (TOKEN_I32, TOKEN_VOID, etc.)

- [ ] **Add new parser tests** for v0.0.1 features:
  - const declarations test
  - enum declarations test  
  - defer statements test
  - static method calls test
  - float literals test
  - else clauses test

### Phase 3: Code Generation Updates [REQUIRED FOR COMPILATION]

#### 3.1 LLVM IR Generation 🔴 HIGH PRIORITY
- [ ] **Update codegen.c** to handle new AST nodes:
  - `NODE_CONST_DECL` - Generate const globals
  - `NODE_DEFER` - Generate deferred call instructions
  - `NODE_STATIC_CALL` - Generate static method calls
  - `NODE_ENUM` - Generate enum constants
  - `NODE_FLOAT` - Generate f32 literals
  - `NODE_TUPLE` - Generate tuple structures

- [ ] **Add f32 type support** in LLVM:
  ```c
  // Add to codegen functions:
  LLVMTypeRef get_f32_type();
  LLVMValueRef codegen_f32_literal(float value);
  ```

- [ ] **Update type mapping** from Gloin types to LLVM:
  - TYPE_F32 → LLVMFloatType()
  - TYPE_VOID → LLVMVoidType() 
  - Handle tuple types as LLVM struct types

#### 3.2 Built-in Functions 🟡 MEDIUM PRIORITY
- [ ] **Update standard library** functions for new types:
  - std.to_string() for f32 values
  - std.to_f32() conversion function
  - std.println() for all new types

### Phase 4: Advanced v0.0.1 Features [FEATURE COMPLETE]

#### 4.1 Memory Management 🟡 MEDIUM PRIORITY
- [ ] **Implement defer mechanism**:
  - Track deferred calls per function scope
  - Generate cleanup code before returns
  - Handle defer in control flow (if/else/loops)

#### 4.2 Concurrency Primitives 🔵 LOW PRIORITY
- [ ] **Add basic concurrency support**:
  - `deferred` function attribute handling
  - `spawnable` function attribute handling  
  - `run` keyword for launching functions
  - Basic Result<T, E> type for async operations

#### 4.3 Enhanced Type System 🔵 LOW PRIORITY
- [ ] **Implement tuple operations**:
  - Tuple indexing: point.0, point.1
  - Tuple destructuring: def (x, y) = point;
  - Tuple type compatibility

- [ ] **Enum variant access**:
  - Color::Red access
  - Enum variant type checking
  - Switch statement enum support

## 🧪 TESTING & VALIDATION

### Phase 5: Integration Testing [CONTINUOUS]

#### 5.1 Unit Tests 🟡 MEDIUM PRIORITY
- [ ] **Fix all failing tests**:
  - Parser tests (currently failing)
  - AST tests for new node types
  - Type system tests for new types

- [ ] **Add comprehensive v0.0.1 tests**:
  - End-to-end compilation tests
  - Runtime execution tests
  - Error handling tests

#### 5.2 Example Programs 🟡 MEDIUM PRIORITY
- [ ] **Create v0.0.1 example programs**:
  ```gloin
  // examples/v0_0_1_showcase.gloin
  import "@std"
  
  def const PI: f32 = 3.14159;
  
  enum Color {
      Red,
      Green,
      Blue
  }
  
  def struct Point {
      x: f32,
      y: f32,
      
      def static new(x: f32, y: f32) -> Point {
          return Point{ x: x, y: y };
      }
      
      def area() -> f32 {
          return self.x * self.y;
      }
  }
  
  def main() -> i32 {
      def p: Point = Point::new(10.5, 20.3);
      defer cleanup();
      
      if p.area() > 100.0 {
          std.println("Large area");
      } else {
          std.println("Small area");
      }
      
      return 0;
  }
  ```

#### 5.3 Regression Testing 🟡 MEDIUM PRIORITY
- [ ] **Ensure backward compatibility**:
  - All pre-v0.0.1 programs still compile
  - No performance regressions
  - Memory usage within acceptable bounds

## 📋 TASK PRIORITY MATRIX

### 🔴 CRITICAL (Required for basic functionality)
1. **Parser Updates** - Without these, nothing new will parse
2. **AST Node Extensions** - Foundation for all new features  
3. **Type System Updates** - f32, void, tuple support
4. **Basic Code Generation** - LLVM IR for new constructs

### 🟡 IMPORTANT (Required for feature completeness)  
1. **Parser Tests** - Ensure reliability
2. **Advanced Code Generation** - Full v0.0.1 feature support
3. **Integration Testing** - End-to-end validation
4. **Example Programs** - Demonstrate capabilities

### 🔵 NICE TO HAVE (Future enhancements)
1. **Concurrency Primitives** - Advanced async/parallel features
2. **Enhanced Tuple Operations** - Full tuple feature set
3. **Performance Optimizations** - Code generation improvements

## 🚀 IMPLEMENTATION ROADMAP

### Week 1: Parser Foundation
- [ ] Day 1-2: AST node extensions
- [ ] Day 3-4: Type system updates  
- [ ] Day 5-7: Core parser function updates

### Week 2: Code Generation
- [ ] Day 1-3: LLVM IR generation for new nodes
- [ ] Day 4-5: Type mapping and built-ins
- [ ] Day 6-7: Testing and debugging

### Week 3: Integration & Testing
- [ ] Day 1-3: Fix all test failures
- [ ] Day 4-5: Create comprehensive test suite
- [ ] Day 6-7: Example programs and documentation

### Week 4: Advanced Features & Polish
- [ ] Day 1-3: Defer mechanism implementation
- [ ] Day 4-5: Advanced type operations
- [ ] Day 6-7: Performance testing and optimization

## ✅ SUCCESS CRITERIA

The Gloin v0.0.1 implementation will be considered complete when:

1. **All Tests Pass**: Unit tests, parser tests, integration tests
2. **Full Compilation**: Complex v0.0.1 programs compile to working executables  
3. **Runtime Execution**: Generated programs run correctly with new features
4. **Backward Compatibility**: Existing programs continue to work
5. **Documentation**: Updated examples and specification reflect actual capabilities

## 🛠️ DEVELOPMENT COMMANDS

```bash
# Build and test cycle
make clean && make                    # Build compiler
make unit-test                       # Run all tests
./gloin_parser example.gloin         # Parse example
./gloin_parser example.gloin --compile example  # Compile example
./example                           # Run compiled program

# Development workflow
git commit -m "feat: implement X"   # Commit changes
make unit-test                       # Verify no regressions
./test_v0_0_1_features.sh           # Run v0.0.1 specific tests
```

This TODO list provides a complete roadmap for implementing Gloin v0.0.1. The priority system ensures that critical blocking issues are resolved first, followed by feature completeness, and finally polish and advanced features.