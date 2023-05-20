//
// Created by Ariel Saldana on 5/19/23.
//

#ifndef SEARCH_AND_DESTROY_CPP_SEARCH_H
#define SEARCH_AND_DESTROY_CPP_SEARCH_H

#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class search {

public:
    [[nodiscard]] static std::unordered_map<std::string, std::vector<std::string>>
    search_directory(const std::string &root_directory);

private:
    [[nodiscard]] bool are_files_same_size(const std::string &first_file, const std::string &second_file);

    [[nodiscard]] static uint64_t file_size(const std::string &file_path);

    [[nodiscard]] static std::vector<std::string> get_directory_files(const std::filesystem::path &path);

    [[nodiscard]] static std::unordered_map<std::string, std::vector<std::string>>
    get_file_hashes(const std::vector<std::string> &files_paths);

    [[nodiscard]] static std::unordered_map<std::string, std::vector<std::string>>
    filter_different_files(std::unordered_map<std::string, std::vector<std::string>> &file_hashes);


};

#endif // SEARCH_AND_DESTROY_CPP_SEARCH_H