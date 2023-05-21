#include "destroy.h"
#include <filesystem>
#include <iostream>

bool
destroy::destroy_files(const std::vector<std::pair<std::string, std::string>>& files_to_delete)
{

    std::cout << "\033[31mFor now instead of doing destructive operations we "
                 "will just be renaming files suffixed with "
                 ".TODELETE. Safety first. 👾\033[0m"
              << std::endl;
    for (const auto& delete_pair : files_to_delete)
    {
        const auto source_path = delete_pair.first;
        const auto destination_path = source_path + ".TODELETE";

        std::cout << "moving file: " << source_path << " to: " << destination_path << std::endl;
        auto result = std::rename(source_path.c_str(), destination_path.c_str());

        if (result != 0)
        {
            // failure
            return false;
        }
    }
    return true;
}
