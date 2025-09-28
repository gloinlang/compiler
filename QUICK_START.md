# GLOIN v0.0.1 QUICK START GUIDE

## 🚀 IMMEDIATE NEXT STEPS (Priority Order)

### 1. Fix Parser for `def const` declarations [CRITICAL - 30 min]
**Problem**: Parser currently expects `fn` or `struct` but crashes on `def const PI: f32 = 3.14;`

**Files to modify**:
```bash
parser.c:parse_program()     # Add case for TOKEN_DEF + TOKEN_CONST
ast.h                        # Add NODE_CONST_DECL
```

**Quick fix**:
```c
// In parse_program(), after TOKEN_STRUCT case:
else if (parser->current_token.type == TOKEN_DEF) {
    Token next = peek_next_token(parser);
    if (next.type == TOKEN_CONST) {
        ASTNode *const_decl = parse_const_declaration(parser);
        add_declaration_to_program(program, const_decl);
    }
    // ... handle other def cases
}
```

### 2. Add f32 Type Support [CRITICAL - 45 min]
**Problem**: Parser doesn't recognize f32 as valid type in type parsing

**Files to modify**:
```bash
types.c:string_to_type()     # Add "f32" -> TYPE_F32 mapping
parser.c:parse_type()        # Handle TOKEN_F32
```

### 3. Fix Parser Tests [HIGH - 20 min]
**Problem**: Tests expect old TOKEN_IDENTIFIER but now get TOKEN_I32, etc.

**Files to modify**:
```bash
test_parser.c               # Update type expectations
```

## 🔧 DEVELOPMENT WORKFLOW

### Quick Build & Test Cycle
```bash
# 1. Check current status
./dev_helper.sh status

# 2. Make changes to parser.c, ast.h, etc.

# 3. Build and test
make clean && make
make unit-test

# 4. Test specific features
./dev_helper.sh feature comments
./dev_helper.sh test parser

# 5. Create test file and validate
./dev_helper.sh create-test
./gloinc v0_0_1_test_simple.gloin --debug
```

### Debug Parser Issues
```bash
# See exactly what token is causing issues
./gloinc test_file.gloin --debug

# Show only AST and LLVM IR (no compilation)
./gloinc test_file.gloin --ast

# Test minimal cases
echo "def const PI: f32 = 3.14;" | ./gloinc /dev/stdin --ast
```

## 📁 KEY FILES TO MODIFY

### Phase 1: Critical Parser Fixes
- `parser.c` - Add const declaration parsing
- `ast.h` - Add new AST node types
- `types.c` - Add f32 support
- `test_parser.c` - Fix test expectations

### Phase 2: Code Generation
- `codegen.c` - LLVM IR for new features
- `types.h` - Type system updates

### Phase 3: Advanced Features  
- `parser.c` - defer, enum, static calls
- `ast.c` - AST creation functions

## 🎯 SUCCESS METRICS

### Milestone 1: Basic Parsing (1-2 hours)
- [ ] `def const PI: f32 = 3.14;` parses without error
- [ ] All existing parser tests pass
- [ ] Simple f32 variables can be declared

### Milestone 2: Code Generation (2-4 hours)
- [ ] const declarations compile to LLVM IR
- [ ] f32 literals generate correct code
- [ ] Programs with new features execute correctly

### Milestone 3: Full v0.0.1 (1-2 days)
- [ ] All v0.0.1 syntax examples compile
- [ ] defer statements work
- [ ] Static method calls (Point::new) work
- [ ] Comprehensive test suite passes

## 🚨 CURRENT BLOCKERS

1. **Parser Error**: "Expected import or function declaration at line 4, column 1"
   - **Cause**: parse_program() doesn't handle `def const`
   - **Solution**: Add const declaration parsing branch

2. **Type Error**: f32 not recognized as valid type
   - **Cause**: string_to_type() missing f32 mapping
   - **Solution**: Add f32 to type system

3. **Test Failures**: Parser tests expect old token types  
   - **Cause**: Tests use TOKEN_IDENTIFIER for types that are now TOKEN_I32
   - **Solution**: Update test expectations

## 🎉 QUICK WINS (30 minutes each)

### Win 1: Comment Support Validation
```bash
# Already works! Just verify:
echo "// test comment" | ./gloin_parser /dev/stdin
# Should parse without error
```

### Win 2: Float Literal Parsing
```bash
# Already works in lexer! Just need parser support:
echo "3.14" | ./gloin_parser /dev/stdin  
# Lexer recognizes TOKEN_FLOAT correctly
```

### Win 3: New Keywords Recognition
```bash
# Already works in lexer! Test it:
./dev_helper.sh feature comments
./dev_helper.sh test lexer
```

## 📚 RESOURCES

- **TODO_v0_0_1.md**: Complete implementation roadmap
- **Gloin0.0.1.md**: Specification (fixed structure)
- **dev_helper.sh**: Development utilities
- **existing tests**: Examples of working patterns

Start with fixing `parse_program()` to handle `def const` - that's the biggest immediate blocker!