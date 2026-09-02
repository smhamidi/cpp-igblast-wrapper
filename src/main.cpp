#include "extractor.hpp"
#include "tsv_mapper.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

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
    const std::string log_path = (fs::path(env.root()) / "igblast.log").string();

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
        << " > \"" << log_path << "\" 2>&1";

    const int status = std::system(cmd.str().c_str());
    if (status != 0) {
        std::cerr << "igblastn failed with exit status " << status << '\n';
        std::ifstream log_file(log_path);
        if (log_file) {
            std::cerr << log_file.rdbuf() << '\n';
        }
        return status;
    }
    return 0;
}

void print_usage(const char* program) {
    std::cerr << "Usage:\n"
              << "  " << program << " <input.fasta> <output.tsv>\n"
              << "  " << program << " --sequence <nucleotide_sequence>\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string first_arg = argv[1];
    if (first_arg == "-h" || first_arg == "--help") {
        print_usage(argv[0]);
        return 0;
    }

    bool sequence_mode = false;
    std::string sequence;
    std::string input_fasta;
    std::string output_tsv;

    if (first_arg == "--sequence" || first_arg == "-s") {
        if (argc != 3) {
            print_usage(argv[0]);
            return 1;
        }
        sequence_mode = true;
        sequence = argv[2];
    } else if (argc == 3) {
        input_fasta = argv[1];
        output_tsv = argv[2];
    } else {
        print_usage(argv[0]);
        return 1;
    }

    try {
        TempEnvironment env;
        std::string query_fasta_path;

        if (sequence_mode) {
            query_fasta_path = (fs::path(env.root()) / "input_query.fasta").string();
            std::ofstream qfile(query_fasta_path);
            if (!qfile) {
                throw std::runtime_error("Failed to create temporary FASTA file: " + query_fasta_path);
            }
            qfile << ">input_seq\n" << sequence << "\n";
            qfile.close();
        } else {
            query_fasta_path = input_fasta;
        }

        if (run_igblastn(env, query_fasta_path) != 0) {
            return 1;
        }

        if (sequence_mode) {
            map_airr_to_imgt_summary(env.temp_airr_path(), std::cout);
        } else {
            map_airr_to_imgt_summary(env.temp_airr_path(), output_tsv);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
