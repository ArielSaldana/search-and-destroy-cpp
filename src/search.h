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
private:
  static std::vector<std::string>
  get_directory_files(const std::filesystem::path &path);

  static std::unordered_map<std::string, std::vector<std::string>>
  get_file_hashes(const std::vector<std::string> &files_paths);

public:
  static std::unordered_map<std::string, std::vector<std::string>>
  search_directory(const std::string &root_directory);
};

#endif // SEARCH_AND_DESTROY_CPP_SEARCH_H
