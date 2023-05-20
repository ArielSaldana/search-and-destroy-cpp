#include "destroy.h"
#include "search.h"
#include <chrono>
#include <iostream>
#include <string>

int main() {
    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    // Run the search function to get the vector of pairs <duplicate_file_to_delete, original_file>
    const std::string root_directory = "/Users/ariel/Movies";
    auto files_to_delete = search::search_directory(root_directory);

    // Get the end time
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Print things
    std::cout << "Elapsed time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Total number of suspected dupes: " << files_to_delete.size() << std::endl;

    // Attempting to destroy files
    auto destroy_result = destroy::destroy_files(files_to_delete);
    if (!destroy_result) {
        std::cout << "some error occured destroying files" << std::endl;
    }

    return 0;
}
