#!/bin/bash
echo "Gloin Compiler Quick Start"
echo "=========================="
echo ""
echo "1. Create a hello world program:"
echo 'import "@std"
def main() -> i32 {
    std.println("Hello, World!");
    return 0;
}' > hello.gloin

echo ""
echo "2. Compile and run:"
echo "./gloinc hello.gloin"
echo "./hello"
echo ""
echo "3. Try the examples:"
echo "./gloinc examples/variables.gloin && ./variables"
echo ""
echo "4. See help:"
echo "./gloinc --help"
echo ""
echo "Happy coding with Gloin!"
