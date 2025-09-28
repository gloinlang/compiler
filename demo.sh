#!/bin/bash

# Gloin Programming Language Demo
# This script demonstrates the capabilities of the Gloin compiler

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}🎯 Gloin Programming Language Demo${NC}"
echo "========================================="
echo

echo -e "${YELLOW}📁 Available Examples:${NC}"
ls -1 examples/*.gloin | sed 's/^/  - /'
echo

echo -e "${YELLOW}🔨 Building and running examples...${NC}"
echo

for example in examples/*.gloin; do
    name=$(basename "$example" .gloin)
    echo -e "${GREEN}▶ Running: $name${NC}"
    
    # Compile (silent)
    ./build/gloinc "$example" >/dev/null 2>&1
    
    # Show source
    echo "   Source:"
    sed 's/^/     /' "$example"
    
    # Run if executable exists
    executable="examples/$name"
    if [[ -x "$executable" ]]; then
        echo "   Output:"
        "./$executable" 2>/dev/null | sed 's/^/     /' || echo "     (execution failed)"
    else
        echo "     (no executable generated)"
    fi
    
    echo
done

echo -e "${YELLOW}🛠️  Compiler Modes Demo:${NC}"
echo

echo -e "${GREEN}▶ AST Analysis Mode (--ast):${NC}"
echo "   Command: ./build/gloinc examples/hello_world.gloin --ast"
echo "   Shows: Source code, AST structure, and LLVM IR"
echo

echo -e "${GREEN}▶ Debug Mode (--debug):${NC}" 
echo "   Command: ./build/gloinc examples/hello_world.gloin --debug"
echo "   Shows: Everything + compiles to executable"
echo

echo -e "${GREEN}▶ Production Mode (default):${NC}"
echo "   Command: ./build/gloinc examples/hello_world.gloin"
echo "   Shows: Nothing (silent compilation)"
echo

echo -e "${YELLOW}📊 Language Features Demonstrated:${NC}"
echo "  ✅ Import system (@std)"
echo "  ✅ Function definitions (def)" 
echo "  ✅ Type annotations (-> i32)"
echo "  ✅ Standard library (std.println, std.print)"
echo "  ✅ Variables (def, def mut)"
echo "  ✅ Type conversions (std.to_string)"
echo "  ✅ LLVM compilation"
echo

echo -e "${BLUE}🎉 Demo Complete! Try building your own Gloin programs.${NC}"