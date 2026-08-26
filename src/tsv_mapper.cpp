#include "tsv_mapper.hpp"

#include <fstream>
#include <regex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

constexpr const char* kDerivedColumns[] = {
    "v_family",
    "d_family",
    "j_family",
    "cdr3_aa_length",
};

std::vector<std::string> split_tsv_line(const std::string& line) {
    std::vector<std::string> fields;
    fields.reserve(100);

    std::size_t start = 0;
    while (start <= line.size()) {
        const std::size_t tab = line.find('\t', start);
        if (tab == std::string::npos) {
            fields.emplace_back(line.substr(start));
            break;
        }
        fields.emplace_back(line.substr(start, tab - start));
        start = tab + 1;
    }

    return fields;
}

std::unordered_map<std::string, std::size_t> build_header_index(const std::vector<std::string>& header) {
    std::unordered_map<std::string, std::size_t> index;
    index.reserve(header.size());
    for (std::size_t i = 0; i < header.size(); ++i) {
        index.emplace(header[i], i);
    }
    return index;
}

std::string field_value(const std::vector<std::string>& row,
                        const std::unordered_map<std::string, std::size_t>& header_index,
                        const std::string& column) {
    const auto it = header_index.find(column);
    if (it == header_index.end() || it->second >= row.size()) {
        return "";
    }
    return row[it->second];
}

std::string gene_family(const std::string& gene_call) {
    if (gene_call.empty()) {
        return "";
    }

    static const std::regex family_re(R"(^(IG(?:H|K|L)(?:V|D|J)\d+))");
    std::smatch match;
    if (std::regex_search(gene_call, match, family_re)) {
        return match[1].str();
    }

    return "";
}

std::string derived_value(const std::vector<std::string>& row,
                          const std::unordered_map<std::string, std::size_t>& header_index,
                          const std::string& column) {
    if (column == "v_family") {
        return gene_family(field_value(row, header_index, "v_call"));
    }
    if (column == "d_family") {
        return gene_family(field_value(row, header_index, "d_call"));
    }
    if (column == "j_family") {
        return gene_family(field_value(row, header_index, "j_call"));
    }
    if (column == "cdr3_aa_length") {
        return std::to_string(field_value(row, header_index, "cdr3_aa").size());
    }

    return "";
}

void write_row(std::ostream& out, const std::vector<std::string>& values) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            out << '\t';
        }
        out << values[i];
    }
    out << '\n';
}

std::vector<std::string> build_output_header(const std::vector<std::string>& input_header) {
    std::unordered_set<std::string> seen(input_header.begin(), input_header.end());
    std::vector<std::string> output_header = input_header;

    for (const char* derived : kDerivedColumns) {
        if (seen.insert(derived).second) {
            output_header.emplace_back(derived);
        }
    }

    return output_header;
}

}  // namespace

void map_airr_to_imgt_summary(const std::string& airr_path, const std::string& output_path) {
    std::ifstream in(airr_path);
    if (!in) {
        throw std::runtime_error("Failed to open AIRR TSV: " + airr_path);
    }

    std::string header_line;
    if (!std::getline(in, header_line)) {
        throw std::runtime_error("AIRR TSV is empty: " + airr_path);
    }

    const std::vector<std::string> input_header = split_tsv_line(header_line);
    const std::unordered_map<std::string, std::size_t> header_index = build_header_index(input_header);
    const std::vector<std::string> output_header = build_output_header(input_header);
    const std::size_t input_column_count = input_header.size();

    std::ofstream out(output_path);
    if (!out) {
        throw std::runtime_error("Failed to open output TSV: " + output_path);
    }

    write_row(out, output_header);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        const std::vector<std::string> row = split_tsv_line(line);
        std::vector<std::string> output_row;
        output_row.reserve(output_header.size());

        for (std::size_t i = 0; i < input_column_count; ++i) {
            if (i < row.size()) {
                output_row.push_back(row[i]);
            } else {
                output_row.emplace_back("");
            }
        }

        for (std::size_t i = input_column_count; i < output_header.size(); ++i) {
            output_row.push_back(derived_value(row, header_index, output_header[i]));
        }

        write_row(out, output_row);
    }
}
