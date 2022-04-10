#pragma once

#include <string>
#include <vector>

namespace Common {

// a generic class that can be used for any frontend
// to keep track of the games in a directory
class GamesList {
public:
    void initialise();

    struct Entry {
        std::string path;
        std::string file_name;
        std::string region;
        std::string size;
    };

    using EntryList = std::vector<Entry>;

    EntryList& get_entries() { return entries; }
    int get_num_entries() { return entries.size(); }
    Entry& get_entry(int entry) { return entries[entry]; }

private:
    void create_entry(const std::string& path);

    EntryList entries;
};

}