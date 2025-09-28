# Building Gloin Compiler

This document describes how to build the Gloin programming language compiler.

## Prerequisites

### Required Dependencies
- **CMake** 3.12 or later
- **LLVM** development libraries and headers
- **GCC** or **Clang** C compiler
- **Make** build system

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake llvm-dev clang build-essential
```

#### CentOS/RHEL/Fedora
```bash
# Fedora
sudo dnf install cmake llvm-devel clang gcc make

# CentOS/RHEL (with EPEL)
sudo yum install cmake3 llvm-devel clang gcc make
```

#### macOS
```bash
brew install cmake llvm
```

## Quick Build

The easiest way to build is using the provided build script:

```bash
# Build in release mode (recommended)
./build.sh

# Build in debug mode
./build.sh build debug

# Clean and rebuild
./build.sh clean
./build.sh build
```

## Manual Build

If you prefer to build manually using CMake:

```bash
# Create build directory
mkdir build
cd build

# Configure (Release mode)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
make -j$(nproc)

# The compiler will be available as: build/gloinc
```

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## Running Tests

```bash
# Using build script
./build.sh test

# Or manually
cd build
ctest --output-on-failure
```

## Installation

```bash
# Using build script
./build.sh install

# Or manually
cd build
make install
```

By default, this installs to `/usr/local/bin`. You can change the install prefix:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/gloin ..
make install
```

## Build Artifacts

After building, you'll find:
- `build/gloinc` - The main Gloin compiler executable
- `build/test_runner` - Test executable (if tests are built)
- `build/libgloin_lib.a` - Static library of compiler components

## Troubleshooting

### LLVM Not Found
If CMake can't find LLVM:
```bash
# Set LLVM_DIR to the CMake directory
export LLVM_DIR=/usr/lib/llvm-<version>/lib/cmake/llvm
cmake ..
```

### Missing llvm-config
Make sure LLVM development packages are installed:
```bash
# Ubuntu/Debian
sudo apt install llvm-dev

# Fedora
sudo dnf install llvm-devel
```

### Linking Issues
If you encounter linking issues, you might need to specify additional libraries:
```bash
cmake -DLLVM_LIBS="$(llvm-config --libs --ldflags)" ..
```

## Development

For development builds with debug symbols:
```bash
./build.sh build debug
```

This enables:
- Debug symbols (`-g`)
- No optimization (`-O0`)
- Additional runtime checks
- Verbose error messages

## Cross-compilation

To cross-compile for different architectures:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/toolchain.cmake ..
```

Refer to CMake documentation for cross-compilation setup.