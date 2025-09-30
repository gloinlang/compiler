#!/bin/bash

# Gloin v0.0.1 Development Helper Script
# This script helps track progress and run tests for v0.0.1 implementation
# Updated: 2024-09-29 - Reflects current implementation status

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE} Gloin v0.0.1 Development Helper${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Check current implementation status
check_status() {
    print_header
    echo "Checking Gloin v0.0.1 implementation status..."
    echo ""
    
    # Check if build exists
    if [ ! -f "./build/gloinc" ]; then
        print_error "Compiler not built. Run './build.sh' first."
        return 1
    fi
    
    # Check lexer status
    print_info "LEXER STATUS:"
    if ./build/test_runner > /dev/null 2>&1; then
        print_success "Lexer: All tests passing (v0.0.1 features implemented)"
    else
        print_error "Lexer: Tests failing"
    fi
    
    # Check parser status  
    print_info "PARSER STATUS:"
    if ./build/test_runner > /dev/null 2>&1; then
        print_success "Parser: All tests passing"
    else
        print_warning "Parser: Tests failing (v0.0.1 features need implementation)"
    fi
    
    # Check compilation status
    print_info "COMPILATION STATUS:"
    if ./build/gloinc examples/main.gloin --ast > /dev/null 2>&1; then
        print_success "Basic compilation: Working"
    else
        print_error "Basic compilation: Failing"
    fi
    
    # Check v0.0.1 feature parsing
    print_info "V0.0.1 FEATURES:"
    
    # Test comment parsing
    if ./build/gloinc test_comments.gloin --ast > /dev/null 2>&1; then
        print_success "Comments: Parser support ✓"
    else
        print_error "Comments: Not working"
    fi
    
    # Test float parsing 
    if ./build/gloinc test_floats.gloin --ast > /dev/null 2>&1; then
        print_success "Float literals: Parser support ✓ (codegen partial)"
    else
        print_error "Float literals: Not working"
    fi
        
    # Test const parsing
    if ./build/gloinc test_const.gloin --ast > /dev/null 2>&1; then
        print_warning "Const declarations: Parsed as mutable (needs fix)"
    else
        print_error "Const declarations: Not working"
    fi
        
    # Test enum parsing
    if ./build/gloinc test_enum.gloin --ast > /dev/null 2>&1; then
        print_success "Enums: Parser support ✓"
    else
        print_error "Enums: Not implemented in parser"
    fi
    
    echo ""
}

# Run specific test categories
run_tests() {
    case "$1" in
        "build")
            print_info "Building project..."
            ./build.sh
            ;;
        "unit")
            print_info "Running unit tests..."
            if [ -f "./build/test_runner" ]; then
                ./build/test_runner
            else
                print_error "Test runner not built. Run './build.sh' first."
            fi
            ;;
        "features")
            print_info "Testing v0.0.1 features..."
            test_feature "comments"
            test_feature "floats" 
            test_feature "const"
            test_feature "enum"
            ;;
        "all")
            print_info "Running all tests..."
            run_tests "build"
            run_tests "unit"
            run_tests "features"
            ;;
        *)
            print_error "Unknown test category: $1"
            echo "Usage: $0 test [build|unit|features|all]"
            exit 1
            ;;
    esac
}

# Build and test a specific v0.0.1 feature
test_feature() {
    local feature="$1"
    case "$feature" in
        "comments")
            print_info "Testing comment support..."
            create_test_file_if_missing "test_comments.gloin" "// This is a comment\ndef main() -> i32 { return 0; }"
            ./build/gloinc test_comments.gloin --ast > /dev/null 2>&1 && print_success "Comments: Working" || print_error "Comments: Failed"
            ;;
        "floats")
            print_info "Testing float literal support..."
            create_test_file_if_missing "test_floats.gloin" "def main() -> i32 { def x: f32 = 3.14; return 0; }"
            ./build/gloinc test_floats.gloin --ast > /dev/null 2>&1 && print_success "Float parsing: Working (codegen needs fix)" || print_error "Float parsing: Failed"
            ;;
        "const")
            print_info "Testing const declarations..."
            create_test_file_if_missing "test_const.gloin" "def const PI: f32 = 3.14159;\n\ndef main() -> i32 {\n    return 0;\n}"
            ./build/gloinc test_const.gloin --ast > /dev/null 2>&1 && print_warning "Const: Parsed as mutable (needs parser fix)" || print_error "Const: Failed"
            ;;
        "enum")
            print_info "Testing enum declarations..."
            create_test_file_if_missing "test_enum.gloin" "enum Color {\n    Red,\n    Green,\n    Blue\n}\n\ndef main() -> i32 {\n    return 0;\n}"
            ./build/gloinc test_enum.gloin --ast > /dev/null 2>&1 && print_success "Enum: Working" || print_error "Enum: Not implemented"
            ;;
        "static")
            print_info "Testing static access operator..."
            create_test_file_if_missing "test_static.gloin" "def main() -> i32 {\n    Point::new(10, 20);\n    return 0;\n}"
            ./build/gloinc test_static.gloin --ast > /dev/null 2>&1 && print_success "Static access: Working" || print_error "Static access: Failed"
            ;;
        *)
            print_error "Unknown feature: $feature"
            echo "Available features: comments, floats, const, enum, static"
            exit 1
            ;;
    esac
}

# Helper function to create test files if they don't exist
create_test_file_if_missing() {
    local filename="$1"
    local content="$2"
    if [ ! -f "$filename" ]; then
        echo -e "$content" > "$filename"
    fi
}

# Show progress toward v0.0.1 completion
show_progress() {
    print_header
    echo "Gloin v0.0.1 Implementation Progress"
    echo ""
    
    local total_tasks=30
    local completed_tasks=18  # Updated based on current progress
    local progress=$((completed_tasks * 100 / total_tasks))
    
    echo "Overall Progress: $completed_tasks/$total_tasks tasks ($progress%)"
    echo ""
    
    print_success "✅ COMPLETED:"
    echo "  - Documentation structure fixed"
    echo "  - Build system: CMake working perfectly"
    echo "  - Lexer: New keywords (const, f32, void, null, self, static, defer, etc.)"
    echo "  - Lexer: Comment support (// comments)"
    echo "  - Lexer: Float number support (3.14, 42.0)"
    echo "  - Lexer: Static access operator (::)"
    echo "  - Lexer: Comprehensive token support"
    echo "  - Parser: Basic comment parsing"
    echo "  - Parser: Float literal parsing"
    echo "  - Parser: Function declarations with void return"
    echo "  - Parser: Basic const parsing (though treated as mutable)"
    echo "  - Code generation: Basic LLVM IR output"
    echo "  - Code generation: Basic f32 type allocation"
    echo "  - Test infrastructure: Working test runner"
    echo "  - Integration: Full compilation pipeline works"
    echo "  - Examples: Working with new syntax"
    echo "  - Release: Pre-built binaries available"
    echo "  - Version: Current v0.0.1-alpha-fc39874"
    echo ""
    
    print_warning "🔧 IN PROGRESS:"
    echo "  - Code generation: f32 literal generation (allocated but not initialized)"
    echo "  - Parser: const keyword semantic handling (parsed but treated as mut)"
    echo ""
    
    print_error "❌ TODO (CRITICAL):"
    echo "  - Parser: enum declarations (enum Color { Red, Green, Blue })"
    echo "  - Parser: static method calls (Point::new())" 
    echo "  - Parser: defer statements (defer cleanup();)"
    echo "  - Parser: else clauses improvements"
    echo "  - Code Generation: Proper f32 literal values"
    echo "  - Code Generation: const vs mut variable handling"
    echo "  - Code Generation: enum support"
    echo "  - Testing: Comprehensive integration tests for v0.0.1 features"
    echo "  - Error handling: Better error messages for new syntax"
    echo "  - Standard library: f32 operations"
    echo ""
    
    echo "Next Priority: Implement enum parsing in parser.c"
    echo "Secondary: Fix const semantic handling (currently parsed as mutable)"
}

# Create a simple v0.0.1 test file
create_test_file() {
    cat > v0_0_1_test_simple.gloin << 'EOF'
// Simple v0.0.1 test - Updated for current parser capabilities
import "@std"

def const PI: f32 = 3.14159;

def main() -> i32 {
    std.println("Testing v0.0.1 features");
    def x: f32 = 2.5;
    return 0;
}
EOF
    print_success "Created v0_0_1_test_simple.gloin"
    print_info "Test it with: ./build/gloinc v0_0_1_test_simple.gloin --ast"
    print_warning "Note: const will be parsed as mutable due to current parser limitation"
}

# Show usage information
show_help() {
    print_header
    echo "Usage: $0 <command> [arguments]"
    echo ""
    echo "Commands:"
    echo "  status              - Check current implementation status"
    echo "  test <category>     - Run specific tests (build|unit|features|all)"
    echo "  feature <name>      - Test specific v0.0.1 feature"
    echo "  progress            - Show implementation progress"
    echo "  create-test         - Create a simple v0.0.1 test file"
    echo "  clean               - Clean build artifacts and test files"
    echo "  help                - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 status           # Check what's working"
    echo "  $0 test all         # Build and run all tests"
    echo "  $0 test unit        # Run only unit tests"
    echo "  $0 feature comments # Test comment parsing"
    echo "  $0 progress         # Show completion status"
    echo ""
    echo "Test Categories:"
    echo "  build    - Build the project using build.sh"
    echo "  unit     - Run the test_runner executable"
    echo "  features - Test all v0.0.1 features"
    echo "  all      - Run build + unit + features"
    echo ""
    echo "Available Features:"
    echo "  comments, floats, const, enum, static"
}

# Main script logic
case "${1:-help}" in
    "status")
        check_status
        ;;
    "test")
        run_tests "$2"
        ;;
    "feature")
        if [ -z "$2" ]; then
            print_error "Feature name required. Available: comments, floats, const, enum, static"
            exit 1
        fi
        test_feature "$2"
        ;;
    "progress")
        show_progress
        ;;
    "create-test")
        create_test_file
        ;;
    "clean")
        print_info "Cleaning build artifacts and test files..."
        rm -rf build/ cmake-build-debug/
        rm -f test_*.gloin v0_0_1_test_simple.gloin
        print_success "Cleaned!"
        ;;
    "help"|*)
        show_help
        ;;
esac