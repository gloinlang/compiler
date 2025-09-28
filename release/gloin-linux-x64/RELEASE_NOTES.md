# Gloin Compiler v0.0.1-alpha Release

This document describes the first public release of the Gloin programming language compiler.

## Release Information

- **Version**: `v0.0.1-alpha-a074c29`
- **Status**: Alpha Pre-release
- **Platforms**: Linux x64, macOS x64, Windows x64
- **Release Date**: September 2024

## What's Included

### ✅ Core Compiler Features
- **Complete compilation pipeline**: Gloin source → LLVM IR → native executable
- **Multi-platform support**: Linux, macOS, Windows (x64)
- **Three build systems**: CMake (recommended), original Makefile, build script
- **Multiple compiler modes**: Production, AST analysis, debug output

### ✅ Language Features
- **Variables with mutability control**: `def` (immutable) vs `def mut` (mutable)
- **Functions with explicit types**: Parameter and return type checking
- **Structs with methods**: Object-oriented programming support
- **Pointers and memory**: Full pointer operations with safety
- **Control flow**: `if`, `unless`, `while`, `for` statements
- **Import system**: Standard library, local modules, external packages

### ✅ Standard Library
- **I/O functions**: `std.print()`, `std.println()`, `std.input()`
- **Type conversions**: `std.to_int()`, `std.to_string()`, `std.to_i64()`
- **Type safety**: Static type checking and validation

### ✅ Developer Tools
- **Project management**: `gloinc init` creates project structure
- **Development modes**: AST viewer, debug output, parse-only
- **Examples and tests**: 4 working examples, 94 test cases
- **Comprehensive documentation**: Complete README with examples

## Installation

### Download Pre-built Binaries
1. Go to [Releases](https://github.com/gloinlang/compiler/releases)
2. Download the appropriate package:
   - **Linux**: `gloin-linux-x64.tar.gz`
   - **macOS**: `gloin-macos-x64.tar.gz`
   - **Windows**: `gloin-windows-x64.zip`
3. Extract and add to PATH

### Build from Source
```bash
git clone https://github.com/gloinlang/compiler.git
cd compiler
./build.sh
```

## Quick Start

### Hello World
```bash
# Create hello.gloin
echo 'import "@std"
def main() -> i32 {
    std.println("Hello, World!");
    return 0;
}' > hello.gloin

# Compile and run
gloinc hello.gloin
./hello
```

### Interactive Calculator
```bash
# Create calc.gloin  
echo 'import "@std"
def add(a: i32, b: i32) -> i32 {
    return a + b;
}
def main() -> i32 {
    std.print("Enter first number: ");
    def a_str: string = std.input();
    std.print("Enter second number: ");
    def b_str: string = std.input();
    def result: i32 = add(std.to_int(a_str), std.to_int(b_str));
    std.print("Result: ");
    std.println(std.to_string(result));
    return 0;
}' > calc.gloin

# Compile and run
gloinc calc.gloin
./calc
```

## Platform Support

| Platform | Status | Notes |
|----------|---------|--------|
| **Linux x64** | ✅ Full | Primary development platform |
| **macOS x64** | ✅ Full | Homebrew LLVM recommended |
| **Windows x64** | ✅ Full | Visual Studio 2022 support |
| Linux ARM64 | 🚧 Planned | Future release |
| macOS ARM64 | 🚧 Planned | Future release |

## Current Limitations

### 🚧 Alpha Software Notice
- **This is alpha software** - expect rough edges and breaking changes
- **Parser coverage** - Some advanced syntax still in development
- **Error messages** - Functional but can be improved
- **Performance** - Not yet optimized for production use

### 🚧 Language Features (In Development)
- **Enums**: Basic support partially implemented
- **Pattern matching**: Advanced switch statements
- **Arrays**: Not yet implemented
- **Generics**: Planned for future versions

## Performance

- **Compilation speed**: ~5 seconds for typical programs
- **Runtime performance**: LLVM-optimized native code
- **Executable size**: ~15KB for simple programs
- **Memory usage**: Efficient static typing

## Examples Included

The release includes working examples:
- `examples/hello_world.gloin` - Basic hello world
- `examples/variables.gloin` - Variable declarations and mutability
- `examples/functions.gloin` - Function definitions and calls
- `examples/main.gloin` - Function composition

## Feedback and Issues

- **Bug reports**: https://github.com/gloinlang/compiler/issues
- **Feature requests**: https://github.com/gloinlang/compiler/issues
- **Discussions**: https://github.com/gloinlang/compiler/discussions

## Roadmap

### v0.0.1-beta (Next Release)
- Enhanced parser coverage
- Improved error messages  
- Extended standard library
- Performance optimizations

### v0.0.1 (Stable)
- Complete parser implementation
- Comprehensive test coverage
- Documentation improvements
- Bug fixes and stability

### Future Versions
- Arrays and collections
- Enhanced error handling
- Generics system
- Language server protocol

## Building from Source

### Prerequisites
- CMake 3.12+
- LLVM development libraries
- C/C++ compiler (GCC/Clang/MSVC)

### Build Steps
```bash
git clone https://github.com/gloinlang/compiler.git
cd compiler

# Easy build
./build.sh

# Or manual CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## License

MIT License - see [LICENSE](LICENSE) file for details.

---

**Thank you for trying Gloin!** This is the beginning of what we hope will become a powerful and enjoyable programming language. Your feedback is invaluable in shaping its future.