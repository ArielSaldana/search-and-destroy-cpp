//
// Created by Ariel Saldana on 5/19/23.
//

#include "search.h"
#include "sha256.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

std::vector<std::string>
search::get_directory_files(const std::filesystem::path& path)
{
    std::vector<std::string> files;
    try
    {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
        {
            for (const auto& entry : std::filesystem::directory_iterator(path))
            {
                if (std::filesystem::is_regular_file(entry))
                {
                    files.push_back(entry.path().string());
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return files;
}

uint64_t
search::file_size(const std::string& file_path)
{
    std::ifstream file(file_path, std::ifstream::ate | std::ifstream::binary);
    if (!file)
    {
        std::cerr << "Failed to open the file: " << file_path << std::endl;
    }
    std::streampos pos = file.tellg();
    auto file_size = static_cast<size_t>(static_cast<std::streamsize>(pos));

    file.close();
    return file_size;
}

std::string
get_file_hash(const std::string& file_path, bool read_full_file = false)
{
    SHA256 sha;
    std::ifstream file(file_path, std::ifstream::ate | std::ifstream::binary);
    if (!file)
    {
        std::cerr << "Failed to open the file: " << file_path << std::endl;
    }

    std::streampos pos = file.tellg();
    if (pos == -1)
    {
        throw std::runtime_error("Cannot read file");
    }
    auto size = static_cast<size_t>(static_cast<std::streamsize>(pos));
    u_long buffer_size;
    if (read_full_file)
    {
        buffer_size = size;
    }
    else
    {
        constexpr size_t max_buffer_size = 16384 * 4;
        buffer_size = max_buffer_size < size ? max_buffer_size : size;
    }

    file.clear();
    file.seekg(0, std::ios::beg);
    char* buffer = new char[buffer_size];
    file.read(buffer, buffer_size);

    if (file.fail())
    {
        if (file.eof())
        {
            std::cout << buffer_size << std::endl;
            std::cout << "EOF reached" << std::endl;
        }
        else
        {
            // Some other error occurred (e.g. file not open)
            throw std::runtime_error("Cannot read file");
        }
    }

    sha.update(reinterpret_cast<const uint8_t*>(buffer), buffer_size);
    uint8_t* digest = sha.digest();
    auto hex_str = SHA256::toString(digest);
    delete[] digest;
    delete[] buffer;
    file.close();

    return hex_str;
}

std::unordered_map<std::string, std::vector<std::string>>
search::get_file_hashes(const std::vector<std::string>& files_paths)
{
    std::vector<std::string> files_to_process = files_paths;
    std::mutex files_hashes_mutex;
    std::mutex file_paths_mutex;
    auto files_hashes = std::unordered_map<std::string, std::vector<std::string>>();

    // The number of threads is equal to as many system cores that are available
    size_t number_of_threads = std::thread::hardware_concurrency();

    if (number_of_threads == 0)
    {
        // commenting out the std::format code, at the time of writing it's not
        // available on Apple Clang.
        //        std:: cout << std::format("warning: unable to get a read on how
        //        many cores are running \"\n"
        //            "                     \"on the system, defaulting to {}.",
        //            search::default_number_of_threads) << std::endl;
        std::cout << "warning: unable to get a read on how many cores are running "
                     "on the system, defaulting to "
                  << default_number_of_threads << "." << std::endl;
        ;
        number_of_threads = 1;
    }

    std::vector<std::thread> threads;

    threads.reserve(number_of_threads);
    for (int i = 0; i < number_of_threads; ++i)
    {
        threads.emplace_back([&file_paths_mutex, &files_hashes_mutex, &files_to_process, &files_hashes]() {
            while (true)
            {
                std::string value;
                {
                    std::lock_guard<std::mutex> lock(file_paths_mutex);
                    if (files_to_process.empty())
                    {
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

    std::for_each(threads.begin(), threads.end(), [](std::thread& t) { t.join(); });

    return files_hashes;
}

std::vector<std::pair<std::string, std::string>>
search::filter_different_files(std::unordered_map<std::string, std::vector<std::string>>& file_hashes)
{
    auto hashed_map = std::unordered_map<std::string, std::vector<std::string>>();
    auto files_to_delete = std::vector<std::pair<std::string, std::string>>();

    for (const auto& pair : file_hashes)
    {
        if (pair.second.size() > 1)
        {
            auto dupe_map = std::unordered_map<uint64_t, std::vector<std::string>>();
            for (const auto& file : pair.second)
            {
                const auto file_size = search::file_size(file);
                dupe_map[file_size].emplace_back(file);
            }

            // after going through the file size check, if any duplicates are
            // suspected do full scans on the files
            auto hashes = std::unordered_map<std::string, std::string>();

            for (const auto& entry : dupe_map)
            {
                if (entry.second.size() > 1)
                {
                    for (const auto& dupe : entry.second)
                    {
                        auto file_hash = get_file_hash(dupe, true);
                        // exact file hash exists, mark for deletion
                        auto hash_iterator = hashes.find(file_hash);
                        if (hash_iterator != hashes.end())
                        {
                            auto original_file = hash_iterator->first;
                            auto delete_pair = std::pair<std::string, std::string>(dupe, original_file);
                            files_to_delete.emplace_back(delete_pair);
                        }
                        else
                        {
                            hashes[file_hash] = dupe;
                        }
                    }
                }
            }
        }
    }
    return files_to_delete;
}

std::vector<std::pair<std::string, std::string>>
search::search_directory(const std::string& root_directory)
{
    auto files_list = get_directory_files(root_directory);
    auto files_hashes = get_file_hashes(files_list);
    auto filtered_list = filter_different_files(files_hashes);
    return filtered_list;
}
