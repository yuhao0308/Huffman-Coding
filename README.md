# Huffman Coding Compression and Decompression

A self-made C++ file archiver and archive extractor program based on Huffman's lossless compression algorithm. This project includes both the original sequential version and a modified version optimized with OpenMP for parallel processing.

## Table of Contents

- [Huffman Coding Compression and Decompression](#huffman-coding-compression-and-decompression)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [How It Works](#how-it-works)
    - [Compressor](#compressor)
    - [Decompressor](#decompressor)
    - [OpenMP Parallelization](#openmp-parallelization)
  - [Compilation and Setup](#compilation-and-setup)
    - [Prerequisites](#prerequisites)
    - [Compilation](#compilation)
      - [Compiler Configuration](#compiler-configuration)
  - [Usage](#usage)
    - [Compressing Files](#compressing-files)
    - [Decompressing Files](#decompressing-files)
  - [Testing and Performance Comparison](#testing-and-performance-comparison)
    - [Understanding the Output](#understanding-the-output)
  - [Troubleshooting](#troubleshooting)
    - [Compiler Errors](#compiler-errors)
    - [Input Issues](#input-issues)
  - [License](#license)
  - [Acknowledgments](#acknowledgments)

## Features

- Compress and decompress files and directories using Huffman coding
- Supports password protection for compressed files
- Includes both the original sequential compressor and an optimized parallel version using OpenMP
- Provides a test suite (`test_compression.cpp`) to compare performance between the original and modified compressors
- Compatible with multiple operating systems (Linux, macOS) with appropriate compiler configurations

## How It Works

### Compressor

The Compressor is a two-pass program that reads input files twice:

**First Pass:**
- Counts the frequency of each unique byte in the input files
- Builds a Huffman tree based on the byte frequencies
- Generates a translation table (Huffman codes) for each unique byte
- Writes the translation information to the compressed file for decompression purposes

**Second Pass:**
- Translates the input files into Huffman codes using the translation table
- Writes the encoded data to the compressed file

### Decompressor

The Decompressor is a one-pass program:
- Reads the translation information from the compressed file and reconstructs the Huffman tree
- Decodes the rest of the compressed file using the Huffman tree
- Reconstructs the original files and directories

### OpenMP Parallelization

The modified compressor (`Compressor_OpenMP.cpp`) uses OpenMP to optimize performance:
- Parallel Byte Frequency Counting: Counts byte frequencies in parallel across files and directories
- Parallel Huffman Tree Construction: Assigns Huffman codes using OpenMP tasks
- Parallel File Compression: Compresses multiple files concurrently
- Thread Safety: Ensures shared variables are protected using critical sections or thread-local storage

## Compilation and Setup

### Prerequisites

- **C++ Compiler:** A compiler that supports C++14 or higher
- **OpenMP Support:** Necessary for compiling the modified compressor and test suite
- **Make:** To use the provided Makefile

### Compilation

Use the provided Makefile to compile all programs:

```bash
make all
```

This will compile:
- `archive`: Original compressor (`Compressor.cpp`)
- `modified_archive`: Modified compressor with OpenMP (`Compressor_OpenMP.cpp`)
- `extract`: Decompressor (`Decompressor.cpp`)
- `test_compression`: Test suite (`test_compression.cpp`)

#### Compiler Configuration

**On Linux:**
- Ensure g++ supports OpenMP
- If necessary, specify the compiler when running make:
```bash
make all CXX=g++ CXXFLAGS='-std=c++14'
```

**On macOS:**
- Install GCC via Homebrew to get OpenMP support:
```bash
brew install gcc
```
- Use the installed GCC compiler (e.g., g++-11):
```bash
make all CXX=g++-11 CXXFLAGS='-std=c++14'
```

**Customizing the Makefile:**
```makefile
# Define compiler and flags, allow overriding from the command line
CXX ?= g++
CXXFLAGS ?= -std=c++14
```
You can override these when invoking make.

## Usage

### Compressing Files

**Original Compressor**

To compress files or directories using the original compressor:
```bash
./archive <input_file_or_directory1> [<input_file_or_directory2> ...]
```

**Modified Compressor (OpenMP Optimized)**

To compress using the modified compressor:
```bash
./modified_archive <input_file_or_directory1> [<input_file_or_directory2> ...]
```

**Password Protection**

During compression, the program will prompt:

*Password Prompt:*
```
If you want a password write any number other than 0
If you do not, write 0
```

*Compression Confirmation:*
```
The size of the sum of ORIGINAL files is: [size] bytes
The size of the COMPRESSED file will be: [size] bytes
Compressed file's size will be [%percentage] of the original file
If you wish to abort this process write 0 and press enter
If you want to continue write any other number and press enter
```

### Decompressing Files

To decompress a compressed file:
```bash
./extract <compressed_file>
```

If the compressed file is password-protected, you will be prompted to enter the password.

## Testing and Performance Comparison

The `test_compression` program automates the testing process by:
- Compressing files using both the original and modified compressors
- Measuring execution time and compression ratios
- Comparing the compressed files to ensure they are identical
- Generating a detailed report of the results

**Running the Test Suite**
```bash
./test_compression <input_file1> [<input_file2> ...]
```

Example:
```bash
./test_compression sample_file.txt
```

### Understanding the Output

After running `test_compression`, you'll see output similar to:

```
Test file: sample_file.txt - Compressed files are identical.

Detailed Report:
File                 Input Size      Orig Size      Mod Size       Orig Time(s)        Mod Time(s)   Speedup     Orig Ratio     Mod Ratio
sample_file.txt           102400         51200         51200               0.50               0.20      2.50         0.5000        0.5000
```

- **File:** Name of the test file
- **Input Size:** Original file size in bytes
- **Orig Size:** Size after compression using the original compressor
- **Mod Size:** Size after compression using the modified compressor
- **Orig Time(s):** Time taken by the original compressor (in seconds)
- **Mod Time(s):** Time taken by the modified compressor (in seconds)
- **Speedup:** How many times faster the modified compressor is compared to the original
- **Orig Ratio:** Compression ratio achieved by the original compressor
- **Mod Ratio:** Compression ratio achieved by the modified compressor

## Troubleshooting

### Compiler Errors

**OpenMP Not Supported:** If you receive an error like `clang++: error: unsupported option '-fopenmp'`:

*On macOS:*
1. Install GCC via Homebrew:
```bash
brew install gcc
```
2. Use the installed GCC compiler when compiling:
```bash
make all CXX=g++-11 CXXFLAGS='-std=c++14'
```

**Undefined Reference to omp_get_wtime:** Ensure you're linking with OpenMP support by including the `-fopenmp` flag during compilation.

### Input Issues

- **Process Aborted During Compression:** Ensure that the compressors are correctly receiving automated inputs if using `test_compression`. If necessary, modify the compressors to accept command-line arguments instead of interactive input.
- **File Not Found Errors:** Ensure that the input files and directories exist and are correctly specified.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

Inspired by the need to understand and implement Huffman's lossless compression algorithm. Special thanks to the contributors and the open-source community.
