#!/bin/bash
# ============================================================================
# Trading Tool - Development Build Script
# ============================================================================
set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Build configuration
BUILD_TYPE="${1:-Release}"  # Default to Release, accept Debug as argument
BUILD_DIR="build"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Trading Tool Build System${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# ============================================================================
# Check Dependencies
# ============================================================================
echo -e "${YELLOW}[1/5] Checking dependencies...${NC}"

# CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}✗ CMake not found${NC}"
    echo "  Install: sudo apt-get install cmake  (Ubuntu/Debian)"
    echo "           brew install cmake           (macOS)"
    exit 1
fi
CMAKE_VERSION=$(cmake --version | head -n1)
echo -e "${GREEN}✓ ${CMAKE_VERSION}${NC}"

# C++ Compiler
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    echo -e "${GREEN}✓ ${GCC_VERSION}${NC}"
elif command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang++ --version | head -n1)
    echo -e "${GREEN}✓ ${CLANG_VERSION}${NC}"
else
    echo -e "${RED}✗ No C++ compiler found${NC}"
    exit 1
fi

# Python
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}✗ Python3 not found${NC}"
    exit 1
fi
PYTHON_VERSION=$(python3 --version)
echo -e "${GREEN}✓ ${PYTHON_VERSION}${NC}"

# pybind11
if ! python3 -c "import pybind11" 2>/dev/null; then
    echo -e "${RED}✗ pybind11 not found${NC}"
    echo "  Install: pip install pybind11"
    exit 1
fi
echo -e "${GREEN}✓ pybind11 installed${NC}"

echo ""

# ============================================================================
# Clean Previous Build (optional)
# ============================================================================
if [ "$2" == "clean" ]; then
    echo -e "${YELLOW}[2/5] Cleaning previous build...${NC}"
    rm -rf ${BUILD_DIR}
    rm -rf trading_bot/trading_core*.so
    rm -rf trading_bot/trading_core*.dylib
    rm -rf trading_bot/trading_core*.pyd
    echo -e "${GREEN}✓ Clean complete${NC}"
    echo ""
fi

# ============================================================================
# Configure CMake
# ============================================================================
echo -e "${YELLOW}[2/5] Configuring CMake (${BUILD_TYPE})...${NC}"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DPYTHON_EXECUTABLE=$(which python3) \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Configuration successful${NC}"
else
    echo -e "${RED}✗ Configuration failed${NC}"
    exit 1
fi
echo ""

# ============================================================================
# Build C++ Extension
# ============================================================================
echo -e "${YELLOW}[3/5] Building C++ extension (${JOBS} parallel jobs)...${NC}"
cmake --build . --config ${BUILD_TYPE} -j${JOBS}

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Build successful${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi
echo ""

# ============================================================================
# Copy Module to Python Package
# ============================================================================
echo -e "${YELLOW}[4/5] Installing module...${NC}"
cd ..

# Find the built .so/.dylib/.pyd file
if [ "$(uname)" == "Darwin" ]; then
    EXT="*.so"  # macOS
elif [ "$(uname)" == "Linux" ]; then
    EXT="*.so"  # Linux
else
    EXT="*.pyd"  # Windows
fi

MODULE=$(find ${BUILD_DIR}/lib -name "trading_core${EXT}" 2>/dev/null | head -n1)

if [ -z "$MODULE" ]; then
    echo -e "${RED}✗ Built module not found${NC}"
    exit 1
fi

cp "$MODULE" trading_bot/
echo -e "${GREEN}✓ Module installed to trading_bot/${NC}"
echo ""

# ============================================================================
# Run Basic Tests
# ============================================================================
echo -e "${YELLOW}[5/5] Running sanity checks...${NC}"

# Test Python import
python3 -c "from trading_bot import trading_core; print('✓ Module import successful')" 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Python module loads correctly${NC}"
    
    # Show available functions
    echo ""
    echo -e "${BLUE}Available functions:${NC}"
    python3 -c "from trading_bot import trading_core; print('  -', '\n  - '.join([x for x in dir(trading_core) if not x.startswith('_')]))"
else
    echo -e "${RED}✗ Module import failed${NC}"
    exit 1
fi

echo ""

# ============================================================================
# Summary
# ============================================================================
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Build Complete! 🚀${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Build type: ${BUILD_TYPE}"
echo "Build dir:  ${BUILD_DIR}/"
echo "Module:     trading_bot/trading_core.so"
echo ""
echo "Next steps:"
echo "  • Run tests:     pytest tests/python/"
echo "  • Start monitor: python scripts/monitor.py"
echo "  • Run backtest:  python scripts/run_backtest.py"
echo ""
echo "Build options:"
echo "  • Debug build:   ./build.sh Debug"
echo "  • Clean build:   ./build.sh Release clean"
echo ""