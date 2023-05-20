//
// Created by Ariel Saldana on 5/19/23.
//

#include "search.h"
#include "sha256.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <cstdint>

std::vector<std::string> search::get_directory_files(const std::filesystem::path &path) {
    std::vector<std::string> files;
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            if (std::filesystem::is_regular_file(entry)) {
                files.push_back(entry.path().string());
            }
        }
    }
    return files;
}

uint64_t search::file_size(const std::string &file_path) {
    std::ifstream file(file_path, std::ifstream::ate | std::ifstream::binary);
    if (!file) {
        std::cerr << "Failed to open the file: " << file_path << std::endl;
    }
    std::streampos pos = file.tellg();
    auto file_size = static_cast<size_t>(static_cast<std::streamsize>(pos));

    file.close();
    return file_size;
}

bool search::are_files_same_size(const std::string &first_file, const std::string &second_file) {
    std::ifstream f_file(first_file, std::ifstream::ate | std::ifstream::binary);
    if (!f_file) {
        std::cerr << "Failed to open the file: " << first_file << std::endl;
    }
    std::streampos first_file_pos = f_file.tellg();
    auto first_file_size = static_cast<size_t>(static_cast<std::streamsize>(first_file_pos));

    std::ifstream s_file(first_file, std::ifstream::ate | std::ifstream::binary);
    if (!s_file) {
        std::cerr << "Failed to open the file: " << first_file << std::endl;
    }
    std::streampos second_file_pos = s_file.tellg();
    auto second_file_size = static_cast<size_t>(static_cast<std::streamsize>(second_file_pos));

    f_file.close();
    s_file.close();

    return (first_file_size == second_file_size);
}

std::string get_file_hash(const std::string &file_path, bool read_full_file = false) {
    SHA256 sha;
    std::ifstream file(file_path, std::ifstream::ate | std::ifstream::binary);
    if (!file) {
        std::cerr << "Failed to open the file: " << file_path << std::endl;
    }


    std::streampos pos = file.tellg();
    if (pos == -1) {
        throw std::runtime_error("Cannot read file");
    }
    auto size = static_cast<size_t>(static_cast<std::streamsize>(pos));
    u_long buffer_size;
    if (read_full_file) {
        buffer_size = size;
    } else {
        constexpr size_t max_buffer_size = 16384 * 4;
        buffer_size = max_buffer_size < size ? max_buffer_size : size;
    }

    file.clear();
    file.seekg(0, std::ios::beg);
    char* buffer = new char[buffer_size];
    file.read(buffer, buffer_size);

    if (file.fail()) {
        if (file.eof()) {
            std::cout << buffer_size << std::endl;
            std::cout << "EOF reached" << std::endl;
        } else {
            // Some other error occurred (e.g. file not open)
            throw std::runtime_error("Cannot read file");
        }
    }

    sha.update(reinterpret_cast<const uint8_t *>(buffer), buffer_size);
    uint8_t *digest = sha.digest();
    auto hex_str = SHA256::toString(digest);
    delete[] digest;
    delete[] buffer;
    file.close();

    return hex_str;
}

std::unordered_map<std::string, std::vector<std::string>>
search::get_file_hashes(const std::vector<std::string> &files_paths) {
    std::vector<std::string> files_to_process = files_paths;
    std::mutex files_hashes_mutex;
    std::mutex file_paths_mutex;
    auto files_hashes = std::unordered_map<std::string, std::vector<std::string>>();

    size_t number_of_threads = 10;
    std::vector<std::thread> threads;

    threads.reserve(number_of_threads);
    for (int i = 0; i < number_of_threads; ++i) {
        threads.emplace_back([&file_paths_mutex, &files_hashes_mutex, &files_to_process, &files_hashes]() {
            while (true) {
                std::string value;
                {
                    std::lock_guard<std::mutex> lock(file_paths_mutex);
                    if (files_to_process.empty()) {
                        break;
                    }
                    value = files_to_process.back();
                    files_to_process.pop_back();
                }
                auto file_hash = get_file_hash(value);
                {
                    std::lock_guard<std::mutex> lock(files_hashes_mutex);
                    //[] operator creates an empty vector if it doesn't exist,
                    // and then
                    // push_back appends the first value
                    // if it exists it just appends the value.
                    files_hashes[file_hash].push_back(value);
                }
            }
        });
    }

    std::for_each(threads.begin(), threads.end(), [](std::thread &t) { t.join(); });

    return files_hashes;
}

std::unordered_map<std::string, std::vector<std::string>>
search::filter_different_files(std::unordered_map<std::string, std::vector<std::string>> &file_hashes) {
    auto hashed_map = std::unordered_map<std::string, std::vector<std::string>>();

    auto dupe_count = 0;
    for (const auto &pair: file_hashes) {
        if (pair.second.size() > 1) {
            auto dupe_map = std::unordered_map<uint64_t , std::vector<std::string>>();
            for (const auto &file : pair.second) {
                const auto file_size = search::file_size(file);
                dupe_map[file_size].emplace_back(file);
            }

            for (const auto &entry: dupe_map) {
                if (entry.second.size() > 1) {
                    for (const auto &dupe: entry.second) {
                        std::cout << "duplicate suspected: " << dupe << std::endl;
                        dupe_count++;
                    }
                }
            }
        }
    }

    std::cout << dupe_count << std::endl;
    return hashed_map;
}

std::unordered_map<std::string, std::vector<std::string>> search::search_directory(const std::string &root_directory) {
    auto files_list = get_directory_files(root_directory);
    auto files_hashes = get_file_hashes(files_list);
    auto filtered_list = filter_different_files(files_hashes);
    return filtered_list;
}
