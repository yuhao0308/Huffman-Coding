# Define compiler and flags, allow overriding from the command line
CXX ?= g++
CXXFLAGS ?= -std=c++14

all: archive modified_archive extract test_compression

archive: Compressor.cpp
	$(CXX) $(CXXFLAGS) Compressor.cpp -o archive

modified_archive: Compressor_OpenMP.cpp
	$(CXX) $(CXXFLAGS) -fopenmp Compressor_OpenMP.cpp -o modified_archive

extract: Decompressor.cpp
	$(CXX) $(CXXFLAGS) Decompressor.cpp -o extract

test_compression: test_compression.cpp
	$(CXX) $(CXXFLAGS) -fopenmp test_compression.cpp -o test_compression

clean:
	@rm -f archive
	@rm -f extract
	@rm -f test_compression
	@rm -f modified_archive

.PHONY: all clean