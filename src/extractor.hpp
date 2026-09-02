#pragma once

#include "embed_manifest.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

class TempEnvironment {
public:
    TempEnvironment() {
        std::string tmpl = "/tmp/igblast_env_XXXXXX";
        std::vector<char> buf(tmpl.begin(), tmpl.end());
        buf.push_back('\0');

        if (mkdtemp(buf.data()) == nullptr) {
            throw std::runtime_error(
                std::string("Failed to create temporary directory: ") + std::strerror(errno));
        }

        root_ = buf.data();
        extract_embedded_files(root_);
    }

    TempEnvironment(const TempEnvironment&) = delete;
    TempEnvironment& operator=(const TempEnvironment&) = delete;

    ~TempEnvironment() {
        cleanup();
    }

    const std::string& root() const { return root_; }

    std::string igblastn_path() const { return (fs::path(root_) / "bin" / "igblastn").string(); }

    std::string temp_airr_path() const {
        return (fs::path(root_) / "temp_airr.tsv").string();
    }

private:
    static void write_file(const fs::path& path, const unsigned char* data, unsigned int size,
                           bool executable) {
        fs::create_directories(path.parent_path());

        std::ofstream out(path, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Failed to open for writing: " + path.string());
        }

        out.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        if (!out) {
            throw std::runtime_error("Failed to write: " + path.string());
        }

        if (executable) {
            fs::permissions(path, fs::perms::owner_all | fs::perms::group_exec | fs::perms::others_exec);
        }
    }

    static void extract_embedded_files(const std::string& root) {
        for (std::size_t i = 0; i < kEmbeddedFileCount; ++i) {
            const EmbeddedFile& entry = kEmbeddedFiles[i];
            const fs::path dest = fs::path(root) / entry.relative_path;
            write_file(dest, entry.data, entry.size, entry.executable);
        }
    }

    void cleanup() {
        if (root_.empty()) {
            return;
        }

        std::error_code ec;
        fs::remove_all(root_, ec);
        root_.clear();
    }

    std::string root_;
};
