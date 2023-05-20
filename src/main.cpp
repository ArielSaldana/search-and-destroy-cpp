#include <string>
#include <iostream>
#include <chrono>
#include "search.h"

int main() {
    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    std::string root_directory = "/Users/ariel/Movies";
    auto files_map = search::search_directory(root_directory);

    size_t count = 0;
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
