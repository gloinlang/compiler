# Gloin Language Examples

This directory contains example programs demonstrating various features of the Gloin programming language.

## 🚀 Quick Start

To run any example:
```bash
# Compile and run
../build/gloinc example_name.gloin
./example_name

# Or with custom output name
../build/gloinc example_name.gloin -o my_program
./my_program
```

## 📚 Examples Overview

### **Container Examples**
- **`vector_cpp_style.gloin`** - Complete C++ std::vector implementation with methods
- **`vector_foundation.gloin`** - Simplified vector showing OOP struct methods
- **`complete_types_test.gloin`** - All integer types: i8, i16, i32, i64, u8, u16, u32, u64
- **`type_casting.gloin`** - Type conversion with cast() function
- **`factorial.gloin`** - Factorial with different integer types
- **`ultimate_factorial.gloin`** - Multi-type factorial showcase

### **Basic Language Features**
- **`hello_world.gloin`** - Simple "Hello, World!" program
- **`hello_std.gloin`** - Standard library demonstration
- **`variables.gloin`** - Variable declarations and mutability

### **Function Examples**  
- **`functions.gloin`** - Function definitions, parameters, and calls
- **`math.gloin`** - Mathematical operations with functions
- **`utils.gloin`** - Utility functions (min/max operations)
- **`fibo.gloin`** - Fibonacci-related calculations

### **Advanced Examples**
- **`main.gloin`** - Function organization patterns
- **`version_test.gloin`** - Version-specific feature testing

## 🛠️ Compilation Modes

### Production Mode (Default)
```bash
../build/gloinc hello_world.gloin    # Silent compilation
```

### Debug Mode
```bash
../build/gloinc hello_world.gloin --debug    # Shows AST, LLVM IR, and compiles
```

### AST Analysis Mode
```bash
../build/gloinc hello_world.gloin --ast      # Shows AST and LLVM IR only
```

## 🎯 Language Features Demonstrated

| Feature | Examples |
|---------|----------|
| **Standard Library** | `hello_std.gloin`, `functions.gloin` |
| **Variables & Mutability** | `variables.gloin`, `utils.gloin` |
| **Functions & Parameters** | `functions.gloin`, `math.gloin` |
| **Control Flow** | `fibo.gloin`, `utils.gloin` |
| **Type System** | All examples |
| **Import System** | All examples (using `@std`) |

## 🔧 Running All Examples

Use the demo script to run all examples:
```bash
../demo.sh
```

This will compile and run all examples, showing their output.

## ✨ Example Output

**hello_world.gloin:**
```
Hello, World!
```

**math.gloin:**
```
Math operations demo:
5 + 3 = 8
5 * 3 = 15
5^2 = 25
```

**variables.gloin:**
```
Name: Gloin
Age: 25  
Score: 150
```

---

**Happy coding with Gloin!** 🎉