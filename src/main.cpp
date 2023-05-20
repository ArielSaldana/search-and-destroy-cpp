#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <thread>
#include <mutex>
#include "sha256.h"
#include <chrono>
#include <unordered_map>

namespace fs = std::filesystem;

std::vector<std::string> files_to_process;
std::mutex vector_mutex;
std::unordered_map<std::string, std::vector<std::string>> files_map;
std::mutex files_map_mutex;

void get_directory_files(const std::filesystem::path &path) {
    std::lock_guard<std::mutex> lock(vector_mutex);

    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            if (std::filesystem::is_regular_file(entry)) {
                files_to_process.push_back(entry.path().string());
            }
        }
    }
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

void search_for_duplicates(size_t thread_number) {
    while (true) {
        std::string value;
        {
            std::lock_guard<std::mutex> lock(vector_mutex);
            if (files_to_process.empty()) {
                break;
            }
            value = files_to_process.back();
            files_to_process.pop_back();
        }

        auto file_hash = get_file_hash(value);

        {
            std::lock_guard<std::mutex> lock(files_map_mutex);
            //[] operator creates an empty vector if it doesn't exist, and then push_back appends the first value
            // if it exists it just appends the value.
            files_map[file_hash].push_back(value);
        }
    }
}

void start_threads() {
    size_t number_of_threads = 10;
    std::vector<std::thread> threads;

    // Create the threads.
    threads.reserve(number_of_threads);
    for (int i = 0; i < number_of_threads; ++i) {
        threads.emplace_back(search_for_duplicates, i);
    }

    // Join the threads to the main threads
    for (auto &thread: threads) {
        thread.join();
    }
}

int main() {
    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    std::string path = "/Users/ariel/Movies";
    get_directory_files(path);
    std::cout << "Total number of files to process: " << files_to_process.size() << std::endl;

    start_threads();

    auto count = 0;
    for (const auto &pair: files_map) {
        if (pair.second.size() > 1) {
            count++;
        }
    }

    std::cout << "Total number of suspected dupes: " << count << std::endl;

    // Get the end time
    auto end = std::chrono::high_resolution_clock::now();
    // Compute the difference between the two times in microseconds
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Elapsed time: " << elapsed.count() / 1000 << " ms" << std::endl;

    std::cout << "Suspected dupes" << std::endl;

    for (auto const &map_pair: files_map) {
        if (map_pair.second.size() > 1) {
            for (const auto &file: map_pair.second) {
                std::cout << map_pair.first << " / " << file << std::endl;
            }
        }
    }

    return 0;
}
