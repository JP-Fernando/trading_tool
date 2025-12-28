from setuptools import setup, Extension, find_packages
import pybind11

cpp_extension = Extension(
    'trading_bot.trading_core',
    sources=['src/trading_core.cpp'],
    include_dirs=[pybind11.get_include(), 'include'],
    language='c++',
    extra_compile_args=['-std=c++20', '-O3', '-fPIC'],
)

setup(
    name='trading_tool',
    version='0.0.1',
    description='A trading indicator engine using C++20 and Python.',
    author='JP-Fernando',
    url='https://github.com/JP-Fernando/trading_tool',
    license='MIT',
    packages=find_packages(),
    ext_modules=[cpp_extension],
    install_requires=[
        'numpy>=1.20.0',
        'pandas>=1.5.0',
    ],
    python_requires='>=3.10',
)