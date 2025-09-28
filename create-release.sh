#!/bin/bash

# Gloin Compiler Release Script
# Creates a local release package for testing

set -e

VERSION=${1:-"0.0.1-alpha-$(git rev-parse --short HEAD)"}
PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

if [ "$ARCH" = "x86_64" ]; then
    ARCH="x64"
fi

RELEASE_NAME="gloin-${PLATFORM}-${ARCH}"
RELEASE_DIR="release/${RELEASE_NAME}"

echo "Creating release package: ${RELEASE_NAME}"
echo "Version: ${VERSION}"

# Clean and build
./build.sh clean
./build.sh build

# Create release directory
mkdir -p "${RELEASE_DIR}"

# Copy binary
cp build/gloinc "${RELEASE_DIR}/"

# Copy examples
cp -r examples "${RELEASE_DIR}/"

# Copy documentation
cp README.md "${RELEASE_DIR}/"
cp LICENSE "${RELEASE_DIR}/"
cp RELEASE_NOTES.md "${RELEASE_DIR}/"

# Create quick start script
cat > "${RELEASE_DIR}/quick-start.sh" << 'EOF'
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
EOF

chmod +x "${RELEASE_DIR}/quick-start.sh"

# Create archive
cd release
if command -v tar &> /dev/null; then
    tar -czf "${RELEASE_NAME}.tar.gz" "${RELEASE_NAME}/"
    echo "Created: release/${RELEASE_NAME}.tar.gz"
fi

if command -v zip &> /dev/null; then
    zip -r "${RELEASE_NAME}.zip" "${RELEASE_NAME}/"
    echo "Created: release/${RELEASE_NAME}.zip"
fi

cd ..
echo "Release package created successfully!"
echo "Test with: cd release/${RELEASE_NAME} && ./quick-start.sh"