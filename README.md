# cpp-igblast-wrapper

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.16+-success.svg)
![IgBLAST](https://img.shields.io/badge/IgBLAST-1.22.0-orange.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)

A standalone, zero-runtime-dependency C++ wrapper around NCBI's IgBLAST. 

This tool embeds the `igblastn` executable, human IMGT compiled databases, and auxiliary files directly into a single C++ executable. At runtime, it securely extracts the environment to a temporary directory, runs IgBLAST against an input FASTA file or direct sequence string, and outputs an enriched AIRR TSV format (adding V/D/J families and CDR3 amino acid lengths).

## Features
* **Zero Runtime Dependencies**: IgBLAST and all required human databases are embedded directly in the binary.
* **Multi-Platform Support**: Automatically detects and embeds the correct binary for Linux (x86_64) or macOS (Intel x86_64 & Apple Silicon via Rosetta 2).
* **Automated Environment Management**: Safely extracts tools to an isolated temporary directory and cleans up automatically upon completion.
* **Dual Execution Modes**: Process an input FASTA file to a TSV file, or analyze a single nucleotide sequence directly via the CLI.
* **Enriched AIRR Output**: Parses standard IgBLAST outfmt 19 (AIRR) and appends `v_family`, `d_family`, `j_family`, and `cdr3_aa_length` columns.

## Prerequisites
To build the project, you will need:
* **CMake** (v3.16 or higher)
* **C++17** compatible compiler (GCC, Clang, or Apple Clang)
* **xxd** or **Perl** (Used during the CMake build step to convert binaries into C-headers)

> **Note for macOS Apple Silicon users**: The embedded macOS `igblastn` binary is built for `x86_64`. It runs seamlessly on Apple Silicon (M1/M2/M3/M4) via Rosetta 2. Ensure Rosetta is installed (`softwareupdate --install-rosetta`).

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
   make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
   ```

Upon a successful build, the executable `cpp_igblast` will be available in your `build` directory.

## Usage

### 1. File-to-File Mode
Provide an input FASTA file containing nucleotide sequences and specify the desired output TSV file path:
```bash
./build/cpp_igblast <input.fasta> <output.tsv>
```

**Example:**
```bash
./build/cpp_igblast sample_queries.fasta results_airr_enriched.tsv
```

### 2. Single Sequence CLI Mode
Pass a sequence string directly to `--sequence` (or `-s`). The enriched AIRR TSV header and result row are printed directly to `stdout`:
```bash
./build/cpp_igblast --sequence <nucleotide_sequence>
```

**Example:**
```bash
./build/cpp_igblast --sequence caggtgcagctggtggagtctggggggggcgtggtccagcctgggaggtccctgagactctcctgtgcagcgtctggattcaccttcagtagctatggcatgcactgggtccgccaggctccaggcaaggggctagagtgggtggcagttatatcatatgatggaagtaataaatactacgcagactccgtgaagggccgattcaccatctccagagacaattccaagaacacgctgtatctgcaaatgaacagcctgagagccgaggacacggctgtgtattactgtgcgagattctattgtggtggtgattgctacaactttgactactggggccaagggaccctggtcaccgtctcctcag
```

**Output:**
```tsv
sequence_id	sequence	sequence_aa	locus	stop_codon	vj_in_frame	v_frameshift	productive	rev_comp	complete_vdj	d_frame	v_call	d_call	j_call	sequence_alignment	germline_alignment	sequence_alignment_aa	germline_alignment_aa	v_alignment_start	v_alignment_end	d_alignment_start	d_alignment_end	j_alignment_start	j_alignment_end	v_sequence_alignment	v_sequence_alignment_aa	v_germline_alignment	v_germline_alignment_aa	d_sequence_alignment	d_sequence_alignment_aa	d_germline_alignment	d_germline_alignment_aa	j_sequence_alignment	j_sequence_alignment_aa	j_germline_alignment	j_germline_alignment_aa	fwr1	fwr1_aa	cdr1	cdr1_aa	fwr2	fwr2_aa	cdr2	cdr2_aa	fwr3	fwr3_aa	fwr4	fwr4_aa	cdr3	cdr3_aa	junction	junction_length	junction_aa	junction_aa_length	v_score	d_score	j_score	v_cigar	d_cigar	j_cigar	v_support	d_support	j_support	v_identity	d_identity	j_identity	v_sequence_start	v_sequence_end	v_germline_start	v_germline_end	d_sequence_start	d_sequence_end	d_germline_start	d_germline_end	j_sequence_start	j_sequence_end	j_germline_start	j_germline_end	fwr1_start	fwr1_end	cdr1_start	cdr1_end	fwr2_start	fwr2_end	cdr2_start	cdr2_end	fwr3_start	fwr3_end	fwr4_start	fwr4_end	cdr3_start	cdr3_end	np1	np1_length	np2	np2_length	v_family	d_family	j_family	cdr3_aa_length
input_seq	CAGGTGCAGCTGGTGGAGTCTGGGGGGGGCGTGGTCCAGCCTGGGAGGTCCCTGAGACTCTCCTGTGCAGCGTCTGGATTCACCTTCAGTAGCTATGGCATGCACTGGGTCCGCCAGGCTCCAGGCAAGGGGCTAGAGTGGGTGGCAGTTATATCATATGATGGAAGTAATAAATACTACGCAGACTCCGTGAAGGGCCGATTCACCATCTCCAGAGACAATTCCAAGAACACGCTGTATCTGCAAATGAACAGCCTGAGAGCCGAGGACACGGCTGTGTATTACTGTGCGAGATTCTATTGTGGTGGTGATTGCTACAACTTTGACTACTGGGGCCAAGGGACCCTGGTCACCGTCTCCTCAGQVQLVESGGGVVQPGRSLRLSCAASGFTFSSYGMHWVRQAPGKGLEWVAVISYDGSNKYYADSVKGRFTISRDNSKNTLYLQMNSLRAEDTAVYYCARFYCGGDCYNFDYWGQGTLVTVSS	IGH	F	T	F	T	F	T		IGHV3-30*12	IGHD2-21*01	IGHJ4*03	CAGGTGCAGCTGGTGGAGTCTGGGGGGGGCGTGGTCCAGCCTGGGAGGTCCCTGAGACTCTCCTGTGCAGCGTCTGGATTCACCTTCAGTAGCTATGGCATGCACTGGGTCCGCCAGGCTCCAGGCAAGGGGCTAGAGTGGGTGGCAGTTATATCATATGATGGAAGTAATAAATACTACGCAGACTCCGTGAAGGGCCGATTCACCATCTCCAGAGACAATTCCAAGAACACGCTGTATCTGCAAATGAACAGCCTGAGAGCCGAGGACACGGCTGTGTATTACTGTGCGAGATTCTATTGTGGTGGTGATTGCTACAACTTTGACTACTGGGGCCAAGGGACCCTGGTCACCGTCTCCTCAG	CAGGTGCAGCTGGTGGAGTCTGGGGGGGGCGTGGTCCAGCCTGGGAGGTCCCTGAGACTCTCCTGTGCAGCGTCTGGATTCACCTTCAGTAGCTATGGCATGCACTGGGTCCGCCAGGCTCCAGGCAAGGGGCTAGAGTGGGTGGCAGTTATATCATATGATGGAAGTAATAAATACTACGCAGACTCCGTGAAGGGCCGATTCACCATCTCCAGAGACAATTCCAAGAACACGCTGTATCTGCAAATGAACAGCCTGAGAGCCGAGGACACGGCTGTGTATTACTGTGCGAGANNNTATTGTGGTGGTGATTGCTANNACTTTGACTACTGGGGCCAAGGGACCCTGGTCACCGTCTCCTCAG	QVQLVESGGGVVQPGRSLRLSCAASGFTFSSYGMHWVRQAPGKGLEWVAVISYDGSNKYYADSVKGRFTISRDNSKNTLYLQMNSLRAEDTAVYYCARFYCGGDCYNFDYWGQGTLVTVSS	QVQLVESGGGVVQPGRSLRLSCAASGFTFSSYGMHWVRQAPGKGLEWVAVISYDGSNKYYADSVKGRFTISRDNSKNTLYLQMNSLRAEDTAVYYCARXYCGGDCXXFDYWGQGTLVTVSS	1	294	298	317	320	364	CAGGTGCAGCTGGTGGAGTCTGGGGGGGGCGTGGTCCAGCCTGGGAGGTCCCTGAGACTCTCCTGTGCAGCGTCTGGATTCACCTTCAGTAGCTATGGCATGCACTGGGTCCGCCAGGCTCCAGGCAAGGGGCTAGAGTGGGTGGCAGTTATATCATATGATGGAAGTAATAAATACTACGCAGACTCCGTGAAGGGCCGATTCACCATCTCCAGAGACAATTCCAAGAACACGCTGTATCTGCAAATGAACAGCCTGAGAGCCGAGGACACGGCTGTGTATTACTGTGCGAGA	QVQLVESGGGVVQPGRSLRLSCAASGFTFSSYGMHWVRQAPGKGLEWVAVISYDGSNKYYADSVKGRFTISRDNSKNTLYLQMNSLRAEDTAVYYCAR	CAGGTGCAGCTGGTGGAGTCTGGGGGGGGCGTGGTCCAGCCTGGGAGGTCCCTGAGACTCTCCTGTGCAGCGTCTGGATTCACCTTCAGTAGCTATGGCATGCACTGGGTCCGCCAGGCTCCAGGCAAGGGGCTAGAGTGGGTGGCAGTTATATCATATGATGGAAGTAATAAATACTACGCAGACTCCGTGAAGGGCCGATTCACCATCTCCAGAGACAATTCCAAGAACACGCTGTATCTGCAAATGAACAGCCTGAGAGCCGAGGACACGGCTGTGTATTACTGTGCGAGA	QVQLVESGGGVVQPGRSLRLSCAASGFTFSSYGMHWVRQAPGKGLEWVAVISYDGSNKYYADSVKGRFTISRDNSKNTLYLQMNSLRAEDTAVYYCAR	TATTGTGGTGGTGATTGCTA	YCGGDC	TATTGTGGTGGTGATTGCTA	YCGGDC	ACTTTGACTACTGGGGCCAAGGGACCCTGGTCACCGTCTCCTCAG	FDYWGQGTLVTVSS	ACTTTGACTACTGGGGCCAAGGGACCCTGGTCACCGTCTCCTCAG	FDYWGQGTLVTVSSCAGGTGCAGCTGGTGGAGTCTGGGGGGGGCGTGGTCCAGCCTGGGAGGTCCCTGAGACTCTCCTGTGCAGCGTCT	QVQLVESGGGVVQPGRSLRLSCAAS	GGATTCACCTTCAGTAGCTATGGC	GFTFSSYG	ATGCACTGGGTCCGCCAGGCTCCAGGCAAGGGGCTAGAGTGGGTGGCAGTT	MHWVRQAPGKGLEWVAV	ATATCATATGATGGAAGTAATAAA	ISYDGSNK	TACTACGCAGACTCCGTGAAGGGCCGATTCACCATCTCCAGAGACAATTCCAAGAACACGCTGTATCTGCAAATGAACAGCCTGAGAGCCGAGGACACGGCTGTGTATTACTGT	YYADSVKGRFTISRDNSKNTLYLQMNSLRAEDTAVYYC	TGGGGCCAAGGGACCCTGGTCACCGTCTCCTCA	WGQGTLVTVSS	GCGAGATTCTATTGTGGTGGTGATTGCTACAACTTTGACTAC	ARFYCGGDCYNFDY	TGTGCGAGATTCTATTGTGGTGGTGATTGCTACAACTTTGACTACTGG	48	CARFYCGGDCYNFDYW	16	459.921	39.141	87.208	294M70S2N	297S4N20M47S4N	319S3N45M	2.318e-131	4.158e-07	2.389e-21	100.000	100.000	100.000	1	294	1	294	298	317	5	24	320	364	4	48	1	75	76	99	100	150	151	174	175	288	331	363	289	330	TTC	3	CA	2	IGHV3	IGHD2	IGHJ4	14
```

## License
* This repository is provided under the GNU General Public License v3.0 (GPL-3.0) License.
