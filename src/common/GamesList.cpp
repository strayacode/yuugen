#include <fstream>
#include "common/GamesList.h"
#include "common/FileSystem.h"
#include "common/log.h"
#include "common/types.h"

namespace Common {

void GamesList::initialise() {
    // TODO: add cache file
    std::vector<std::string> paths = FileSystem::scan_directory("../roms", ".nds");

    for (const std::string& path : paths) {
        create_entry(path);
    }
}

void GamesList::create_entry(const std::string& path) {
    Entry entry;

    entry.path = path;
    entry.file_name = std::filesystem::path(path).stem();

    std::ifstream file(path, std::ios::binary);

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);

    entry.size = FileSystem::get_formatted_size(file.tellg());
    file.close();

    entries.push_back(entry);
}

}