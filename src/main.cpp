#include "destroy.h"
#include "search.h"
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

int
main(int argc, char* argv[])
{
    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    // convert the args into a vector of strings
    std::vector<std::string_view> args(argv, argv + argc);

    // Run the search function to get the vector of pairs
    // <duplicate_file_to_delete, original_file>
    const std::string root_directory = args[1].data();
    std::cout << "Root directory set to: " << root_directory.c_str() << std::endl;
    auto files_to_delete = search::search_directory(root_directory);

    // Get the end time
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Print things
    std::cout << "Elapsed time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Total number of suspected duplicates: " << files_to_delete.size() << std::endl;

    for (const auto& file_to_delete : files_to_delete)
    {
        std::cout << file_to_delete.first << " / " << file_to_delete.second << std::endl;
    }

    // Attempting to destroy files
    auto destroy_result = destroy::destroy_files(files_to_delete);
    if (!destroy_result)
    {
        std::cout << "some error occured destroying files" << std::endl;
    }

    return 0;
}
