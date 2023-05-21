#ifndef SEARCH_AND_DESTROY_CPP_DESTROY_H
#define SEARCH_AND_DESTROY_CPP_DESTROY_H

#include <string>
#include <utility>
#include <vector>

class destroy
{

  public:
    [[nodiscard]] static bool destroy_files (const std::vector<std::pair<std::string, std::string> > &files_to_delete);
};

#endif // SEARCH_AND_DESTROY_CPP_DESTROY_H
