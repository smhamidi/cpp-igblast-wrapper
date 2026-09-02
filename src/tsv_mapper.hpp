#pragma once

#include <ostream>
#include <string>

// Reads IgBLAST AIRR TSV (outfmt 19), passes all columns through unchanged,
// appends v_family, d_family, j_family, cdr3_aa_length, and writes to an output stream.
void map_airr_to_imgt_summary(const std::string& airr_path, std::ostream& out);

void map_airr_to_imgt_summary(const std::string& airr_path, const std::string& output_path);
