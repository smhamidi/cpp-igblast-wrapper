#include "extractor.hpp"
#include "tsv_mapper.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string absolute_path(const std::string& path) {
    if (path.empty() || path[0] == '/') {
        return path;
    }
    char* cwd = getcwd(nullptr, 0);
    if (cwd == nullptr) {
        return path;
    }
    const std::string absolute = std::string(cwd) + "/" + path;
    free(cwd);
    return absolute;
}

int run_igblastn(const TempEnvironment& env, const std::string& input_fasta) {
    const std::string query_path = absolute_path(input_fasta);
    const std::string airr_path = env.temp_airr_path();

    std::ostringstream cmd;
    cmd << "cd " << env.root() << " && "
        << "IGDATA=" << env.root() << " "
        << "./bin/igblastn"
        << " -germline_db_V database/human_V"
        << " -germline_db_D database/human_D"
        << " -germline_db_J database/human_J"
        << " -organism human"
        << " -query " << query_path
        << " -auxiliary_data optional_file/human_gl.aux"
        << " -outfmt 19"
        << " -out " << airr_path
        << " 2>&1";

    const int status = std::system(cmd.str().c_str());
    if (status != 0) {
        std::cerr << "igblastn failed with exit status " << status << '\n';
        return status;
    }
    return 0;
}

void print_usage(const char* program) {
    std::cerr << "Usage: " << program << " <input.fasta> <output.tsv>\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string input_fasta = argv[1];
    const std::string output_tsv = argv[2];

    try {
        TempEnvironment env;

        if (run_igblastn(env, input_fasta) != 0) {
            return 1;
        }

        map_airr_to_imgt_summary(env.temp_airr_path(), output_tsv);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
