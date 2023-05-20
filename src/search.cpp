//
// Created by Ariel Saldana on 5/19/23.
//

#include "search.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include "sha256.h"
#include <unordered_map>

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

std::string get_file_hash(const std::string &file_path) {
    std::ifstream file(file_path, std::ifstream::ate | std::ifstream::binary);
    if (!file) {
        std::cerr << "Failed to open the file: " << file_path << std::endl;
    }

    constexpr size_t max_buffer_size = 16384 * 4;

    std::streampos pos = file.tellg();
    if (pos == -1) {
        throw std::runtime_error("Cannot read file");
    }
    auto size = static_cast<size_t>(static_cast<std::streamsize>(pos));
    auto buffer_size = max_buffer_size < size ? max_buffer_size : size;

    file.clear();
    file.seekg(0, std::ios::beg);
    char buffer[buffer_size];
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

    SHA256 sha;
    sha.update(reinterpret_cast<const uint8_t *>(buffer), buffer_size);
    uint8_t *digest = sha.digest();
    auto hex_str = SHA256::toString(digest);
    delete[] digest;
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
                    //[] operator creates an empty vector if it doesn't exist, and then push_back appends the first value
                    // if it exists it just appends the value.
                    files_hashes[file_hash].push_back(value);
                }
            }
        });
    }

    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });

    return files_hashes;
}

std::unordered_map<std::string, std::vector<std::string>> search::search_directory(const std::string &root_directory) {
    auto files_list = get_directory_files(root_directory);
    return get_file_hashes(files_list);
}


