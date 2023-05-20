#include "search.h"
#include <chrono>
#include <iostream>
#include <string>

int main() {
    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    const std::string root_directory = "/Users/ariel/Movies";
    auto files_map = search::search_directory(root_directory);

    // Get the end time
    auto end = std::chrono::high_resolution_clock::now();
    // Compute the difference between the two times in microseconds
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Elapsed time: " << elapsed.count() / 1000 << " ms" << std::endl;
    std::cout << "Suspected dupes" << std::endl;

    size_t count = 0;
    for (auto const &map_pair : files_map) {
        if (map_pair.second.size() > 5) {
            for (const auto &file : map_pair.second) {
                std::cout << map_pair.first << " / " << file << std::endl;
                count++;
            }
        }
    }
    std::cout << "Total number of suspected dupes: " << count << std::endl;

    return 0;
}
