#!/bin/bash

# Gloin Compiler Build Script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is required but not installed. Please install CMake 3.12 or later."
        exit 1
    fi
    
    if ! command -v llvm-config &> /dev/null; then
        print_error "LLVM is required but not found. Please install LLVM development packages."
        exit 1
    fi
    
    # Check LLVM version
    LLVM_VERSION=$(llvm-config --version)
    print_status "Found LLVM version: $LLVM_VERSION"
}

# Build function
build_project() {
    BUILD_TYPE=${1:-Release}
    print_status "Building Gloin compiler in $BUILD_TYPE mode..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    
    # Build
    make -j$(nproc)
    
    print_status "Build completed successfully!"
    print_status "Executable: build/gloinc"
}

# Test function
run_tests() {
    print_status "Running tests..."
    if [ ! -d "build" ]; then
        print_error "Build directory not found. Please build first."
        exit 1
    fi
    cd build
    ctest --output-on-failure
}

# Clean function
clean_build() {
    print_status "Cleaning build directory..."
    rm -rf build
    print_status "Clean completed!"
}

# Install function
install_project() {
    print_status "Installing Gloin compiler..."
    cd build
    make install
    print_status "Installation completed!"
}

# Usage function
usage() {
    echo "Gloin Compiler Build Script"
    echo ""
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  build [debug|release]  Build the compiler (default: release)"
    echo "  test                   Run tests"
    echo "  clean                  Clean build directory"
    echo "  install                Install the compiler"
    echo "  help                   Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 build              # Build in release mode"
    echo "  $0 build debug        # Build in debug mode"
    echo "  $0 test               # Run tests"
    echo "  $0 clean              # Clean build files"
}

# Main script
case "${1:-build}" in
    "build")
        check_dependencies
        build_project ${2:-Release}
        ;;
    "test")
        check_dependencies
        if [ ! -d "build" ]; then
            print_warning "Build directory not found. Building first..."
            build_project Release
        fi
        run_tests
        ;;
    "clean")
        clean_build
        ;;
    "install")
        check_dependencies
        if [ ! -d "build" ]; then
            print_warning "Build directory not found. Building first..."
            build_project Release
        fi
        install_project
        ;;
    "help"|"--help"|"-h")
        usage
        ;;
    *)
        print_error "Unknown command: $1"
        usage
        exit 1
        ;;
esac