# cpp-igblast-wrapper

![C++](https://img.shields.io/badge/C%++-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.16+-success.svg)
![IgBLAST](https://img.shields.io/badge/IgBLAST-1.22.0-orange.svg)

A standalone, zero-runtime-dependency C++ wrapper around NCBI's IgBLAST. 

This tool embeds the `igblastn` executable, human IMGT compiled databases, and auxiliary files directly into a single C++ executable. At runtime, it securely extracts the environment to a temporary directory, runs IgBLAST against an input FASTA file, and processes the output into an enriched AIRR TSV format (adding V/D/J families and CDR3 amino acid lengths).

## Features
* **Zero Runtime Dependencies**: IgBLAST and all required human databases are embedded in the binary.
* **Automated Environment Management**: Safely extracts tools to a temporary directory and cleans up automatically upon completion.
* **Enriched AIRR Output**: Parses standard IgBLAST outfmt 19 (AIRR) and appends `v_family`, `d_family`, `j_family`, and `cdr3_aa_length` columns.

## Prerequisites
To build the project, you will need:
* **CMake** (v3.16 or higher)
* **C++17** compatible compiler (GCC, Clang, or MSVC)
* **xxd** or **Perl** (Used during the CMake build step to convert binaries into C-headers)

## Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/smhamidi/cpp-igblast-wrapper.git
   cd cpp-igblast-wrapper
   ```

2. Generate the build system and compile:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

Upon a successful build, the executable `cpp_igblast` will be available in your `build` directory.

## Usage

Provide an input FASTA file containing nucleotide sequences and specify the desired output TSV file path.

```bash
./build/cpp_igblast <input.fasta> <output.tsv>
```

### Example
```bash
./build/cpp_igblast sample_queries.fasta results_airr_enriched.tsv
```

## License
* This repository is provided under the GNU General Public License v3.0 (GPL-3.0) License.
