"""
Trading Tool - Setup Configuration
Uses CMake for C++ compilation with scikit-build-core
"""
import os
import sys
import subprocess
from pathlib import Path
from setuptools import setup, find_packages

# ============================================================================
# Project Metadata
# ============================================================================
NAME = "trading_tool"
VERSION = "0.0.1"
DESCRIPTION = "High-performance trading indicator engine using C++20 and Python"
AUTHOR = "JP-Fernando"
URL = "https://github.com/JP-Fernando/trading_tool"
LICENSE = "MIT"

# ============================================================================
# Read long description from README
# ============================================================================
def read_readme():
    readme_path = Path(__file__).parent / "README.md"
    if readme_path.exists():
        return readme_path.read_text(encoding="utf-8")
    return DESCRIPTION

# ============================================================================
# Check CMake availability
# ============================================================================
def check_cmake():
    """Verify CMake is installed and meets minimum version requirement."""
    try:
        result = subprocess.run(
            ["cmake", "--version"],
            capture_output=True,
            text=True,
            check=True
        )
        version_line = result.stdout.split('\n')[0]
        print(f"✓ Found {version_line}")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("✗ CMake not found. Please install CMake >= 3.18", file=sys.stderr)
        print("  Ubuntu/Debian: sudo apt-get install cmake")
        print("  macOS: brew install cmake")
        print("  Windows: https://cmake.org/download/")
        return False

# ============================================================================
# Python Dependencies
# ============================================================================
INSTALL_REQUIRES = [
    "numpy>=1.20.0",
    "pandas>=1.5.0",
    "ccxt>=4.0.0",              # Exchange connectivity
    "pyyaml>=6.0",              # Config file parsing
    "python-dotenv>=1.0.0",     # Environment variables
]

EXTRAS_REQUIRE = {
    "test": [
        "pytest>=7.0.0",
        "pytest-asyncio>=0.21.0",
        "pytest-cov>=4.0.0",
    ],
    "backtest": [
        "matplotlib>=3.5.0",
        "seaborn>=0.12.0",
        "plotly>=5.0.0",
    ],
    "ml": [
        "torch>=2.0.0",
        "scikit-learn>=1.2.0",
    ],
    "storage": [
        "psycopg2-binary>=2.9.0",   # PostgreSQL
        "sqlalchemy>=2.0.0",
        "alembic>=1.10.0",          # DB migrations
    ],
    "dev": [
        "black>=23.0.0",
        "ruff>=0.1.0",
        "mypy>=1.0.0",
        "pre-commit>=3.0.0",
    ],
}

# All extras combined
EXTRAS_REQUIRE["all"] = list(set(sum(EXTRAS_REQUIRE.values(), [])))

# ============================================================================
# Build Configuration
# ============================================================================

# Try to use scikit-build-core for modern CMake integration
try:
    from skbuild import setup as cmake_setup
    USE_SCIKIT_BUILD = True
    print("✓ Using scikit-build for CMake integration")
except ImportError:
    USE_SCIKIT_BUILD = False
    print("! scikit-build not found, falling back to setuptools")
    print("  For better CMake integration: pip install scikit-build")

if USE_SCIKIT_BUILD:
    # Modern approach with scikit-build
    setup(
        name=NAME,
        version=VERSION,
        description=DESCRIPTION,
        long_description=read_readme(),
        long_description_content_type="text/markdown",
        author=AUTHOR,
        url=URL,
        license=LICENSE,
        packages=find_packages(exclude=["tests", "tests.*", "benchmarks", "docs"]),
        cmake_install_dir="trading_bot",  # Install C++ module here
        install_requires=INSTALL_REQUIRES,
        extras_require=EXTRAS_REQUIRE,
        python_requires=">=3.10",
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Intended Audience :: Financial and Insurance Industry",
            "License :: OSI Approved :: MIT License",
            "Programming Language :: C++",
            "Programming Language :: Python :: 3.10",
            "Programming Language :: Python :: 3.11",
            "Programming Language :: Python :: 3.12",
            "Topic :: Office/Business :: Financial :: Investment",
        ],
        zip_safe=False,
    )

else:
    # Fallback: Manual CMake build with setuptools
    from setuptools import Extension
    from setuptools.command.build_ext import build_ext
    
    class CMakeExtension(Extension):
        def __init__(self, name, sourcedir=""):
            super().__init__(name, sources=[])
            self.sourcedir = Path(sourcedir).resolve()

    class CMakeBuild(build_ext):
        def run(self):
            # Check CMake is available
            if not check_cmake():
                sys.exit(1)
            
            for ext in self.extensions:
                self.build_extension(ext)

        def build_extension(self, ext):
            if not isinstance(ext, CMakeExtension):
                super().build_extension(ext)
                return

            extdir = Path(self.get_ext_fullpath(ext.name)).parent.resolve()
            
            # CMake configuration arguments
            cmake_args = [
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
                f"-DPYTHON_EXECUTABLE={sys.executable}",
                f"-DCMAKE_BUILD_TYPE={'Debug' if self.debug else 'Release'}",
            ]

            # Build arguments
            build_args = ["--config", "Debug" if self.debug else "Release"]
            
            # Parallel build
            if hasattr(self, "parallel") and self.parallel:
                build_args += ["-j", str(self.parallel)]
            else:
                # Default to number of CPUs
                import multiprocessing
                build_args += ["-j", str(multiprocessing.cpu_count())]

            # Build directory
            build_temp = Path(self.build_temp)
            build_temp.mkdir(parents=True, exist_ok=True)

            print(f"Building C++ extension in {build_temp}")
            
            # Configure
            subprocess.run(
                ["cmake", str(ext.sourcedir)] + cmake_args,
                cwd=build_temp,
                check=True
            )
            
            # Build
            subprocess.run(
                ["cmake", "--build", "."] + build_args,
                cwd=build_temp,
                check=True
            )

    setup(
        name=NAME,
        version=VERSION,
        description=DESCRIPTION,
        long_description=read_readme(),
        long_description_content_type="text/markdown",
        author=AUTHOR,
        url=URL,
        license=LICENSE,
        packages=find_packages(exclude=["tests", "tests.*", "benchmarks", "docs"]),
        ext_modules=[CMakeExtension("trading_bot.trading_core")],
        cmdclass={"build_ext": CMakeBuild},
        install_requires=INSTALL_REQUIRES,
        extras_require=EXTRAS_REQUIRE,
        python_requires=">=3.10",
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Intended Audience :: Financial and Insurance Industry",
            "License :: OSI Approved :: MIT License",
            "Programming Language :: C++",
            "Programming Language :: Python :: 3.10",
            "Programming Language :: Python :: 3.11",
            "Programming Language :: Python :: 3.12",
            "Topic :: Office/Business :: Financial :: Investment",
        ],
        zip_safe=False,
    )