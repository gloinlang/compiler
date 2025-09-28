#!/bin/bash

# Gloin v0.0.1 Development Helper Script
# This script helps track progress and run tests for v0.0.1 implementation

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
    
    # Check lexer status
    print_info "LEXER STATUS:"
    if make unit-test 2>&1 | grep -q "Lexer Tests Results: .* passed, 0 failed"; then
        print_success "Lexer: All tests passing (v0.0.1 features implemented)"
    else
        print_error "Lexer: Tests failing"
    fi
    
    # Check parser status  
    print_info "PARSER STATUS:"
    if make unit-test 2>&1 | grep -q "Parser Tests Results: .* passed, 0 failed"; then
        print_success "Parser: All tests passing"
    else
        print_warning "Parser: Tests failing (v0.0.1 features need implementation)"
    fi
    
    # Check compilation status
    print_info "COMPILATION STATUS:"
    if ./gloinc examples/main.gloin > /dev/null 2>&1; then
        print_success "Basic compilation: Working"
    else
        print_error "Basic compilation: Failing"
    fi
    
    # Check v0.0.1 feature parsing
    print_info "V0.0.1 FEATURES:"
    
    # Test comment parsing
    echo "// test comment" | ./gloinc /dev/stdin > /dev/null 2>&1 && \
        print_success "Comments: Lexer support ✓" || \
        print_error "Comments: Not working"
    
    # Test float parsing 
    echo "3.14" | ./gloinc /dev/stdin > /dev/null 2>&1 && \
        print_success "Float literals: Lexer support ✓" || \
        print_error "Float literals: Not working"
        
    # Test new keywords
    echo "const static self" | ./gloinc /dev/stdin > /dev/null 2>&1 && \
        print_success "New keywords: Lexer support ✓" || \
        print_error "New keywords: Not working"
        
    # Test :: operator
    echo "Point::new" | ./gloinc /dev/stdin > /dev/null 2>&1 && \
        print_success "Static access (::): Lexer support ✓" || \
        print_error "Static access (::): Not working"
    
    echo ""
}

# Run specific test categories
run_tests() {
    case "$1" in
        "lexer")
            print_info "Running lexer tests only..."
            make clean && make && ./test_runner | grep -A 20 "=== Running Lexer Tests ==="
            ;;
        "parser")
            print_info "Running parser tests only..."
            make clean && make && ./test_runner | grep -A 20 "=== Running Parser Tests ==="
            ;;
        "all")
            print_info "Running all tests..."
            make unit-test
            ;;
        *)
            print_error "Unknown test category: $1"
            echo "Usage: $0 test [lexer|parser|all]"
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
            echo -e "// This is a comment\nfn main() -> i32 { return 0; }" > test_comments.gloin
            ./gloinc test_comments.gloin --ast > /dev/null 2>&1 && print_success "Comments: Working" || print_error "Comments: Failed"
            rm -f test_comments.gloin test_comments
            ;;
        "floats")
            print_info "Testing float literal support..."
            echo -e "fn main() -> i32 { def x: f32 = 3.14; return 0; }" > test_floats.gloin
            ./gloinc test_floats.gloin --ast > /dev/null 2>&1 && print_success "Float parsing: Working" || print_error "Float parsing: Failed"
            rm -f test_floats.gloin test_floats
            ;;
        "static")
            print_info "Testing static access operator..."
            echo -e "Point::new(10, 20)" > test_static.gloin
            ./gloinc test_static.gloin --ast > /dev/null 2>&1 && print_success "Static access: Working" || print_error "Static access: Failed"
            rm -f test_static.gloin test_static
            ;;
        *)
            print_error "Unknown feature: $feature"
            echo "Available features: comments, floats, static"
            exit 1
            ;;
    esac
}

# Show progress toward v0.0.1 completion
show_progress() {
    print_header
    echo "Gloin v0.0.1 Implementation Progress"
    echo ""
    
    local total_tasks=30
    local completed_tasks=6  # Lexer tasks completed
    local progress=$((completed_tasks * 100 / total_tasks))
    
    echo "Overall Progress: $completed_tasks/$total_tasks tasks ($progress%)"
    echo ""
    
    print_success "✅ COMPLETED:"
    echo "  - Documentation structure fixed"
    echo "  - Lexer: New keywords (const, f32, void, null, self, static, defer, etc.)"
    echo "  - Lexer: Comment support (// comments)"
    echo "  - Lexer: Float number support (3.14, 42.0)"
    echo "  - Lexer: Static access operator (::)"
    echo "  - Lexer: Comprehensive test suite (13/13 tests passing)"
    echo ""
    
    print_warning "🔧 IN PROGRESS:"
    echo "  - Parser: AST node extensions for new features"
    echo "  - Parser: Type system updates (f32, void, tuples)"
    echo "  - Parser: Function parsing updates"
    echo ""
    
    print_error "❌ TODO (CRITICAL):"
    echo "  - Parser: const declarations (def const PI: f32 = 3.14;)"
    echo "  - Parser: enum declarations (enum Color { Red, Green, Blue })"
    echo "  - Parser: static method calls (Point::new())"
    echo "  - Parser: defer statements (defer cleanup();)"
    echo "  - Parser: else clauses (if x { } else { })"
    echo "  - Code Generation: LLVM IR for new AST nodes"
    echo "  - Code Generation: f32 type support"
    echo "  - Testing: Integration tests for v0.0.1 features"
    echo ""
    
    echo "Next Priority: Fix parser to handle 'def const' declarations"
    echo "Current Blocker: Parser expects 'fn' or 'struct' but not 'def const'"
}

# Create a simple v0.0.1 test file
create_test_file() {
    cat > v0_0_1_test_simple.gloin << 'EOF'
// Simple v0.0.1 test
import "@std"

def const PI: f32 = 3.14159;

def main() -> i32 {
    std.println("Testing v0.0.1 features");
    return 0;
}
EOF
    print_success "Created v0_0_1_test_simple.gloin"
    print_info "Test it with: ./gloinc v0_0_1_test_simple.gloin --ast"
}

# Show usage information
show_help() {
    print_header
    echo "Usage: $0 <command> [arguments]"
    echo ""
    echo "Commands:"
    echo "  status              - Check current implementation status"
    echo "  test <category>     - Run specific tests (lexer|parser|all)"
    echo "  feature <name>      - Test specific v0.0.1 feature"
    echo "  progress            - Show implementation progress"
    echo "  create-test         - Create a simple v0.0.1 test file"
    echo "  help                - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 status           # Check what's working"
    echo "  $0 test lexer       # Run only lexer tests"
    echo "  $0 feature comments # Test comment parsing"
    echo "  $0 progress         # Show completion status"
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
        test_feature "$2"
        ;;
    "progress")
        show_progress
        ;;
    "create-test")
        create_test_file
        ;;
    "help"|*)
        show_help
        ;;
esac