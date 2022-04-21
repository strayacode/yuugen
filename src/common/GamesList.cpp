#include <fstream>
#include <algorithm>
#include "common/GamesList.h"
#include "common/FileSystem.h"
#include "common/StringHelpers.h"
#include "common/Memory.h"
#include "common/log.h"
#include "common/types.h"

namespace Common {

void GamesList::initialise() {
    load_titles_database("../data/dstdb.txt");

    // TODO: add cache file
    std::vector<std::string> paths = FileSystem::scan_directory("../roms", ".nds");

    for (const std::string& path : paths) {
        create_entry(path);
    }

    // sort entries alphabetically by file name
    std::sort(entries.begin(), entries.end(), [](Entry a, Entry b) {
        int result = Common::to_lower(a.file_name).compare(Common::to_lower(b.file_name));

        return result < 0;
    });
}

void GamesList::create_entry(const std::string& path) {
    Entry entry;

    entry.path = path;

    std::ifstream file(path, std::ios::binary);

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);

    entry.size = FileSystem::get_formatted_size(file.tellg());

    u8 info[0x40];
    
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(info), 0x40);

    u32 game_code = Common::read<u32>(info, 0x0C);

    for (int i = 0; i < 4; i++) {
        entry.game_code += (game_code >> (i * 8)) & 0xFF;
    }

    if (titles_map.count(std::string(entry.game_code))) {
        entry.file_name = titles_map[entry.game_code];
    } else {
        entry.file_name = std::filesystem::path(path).stem();
    }

    switch (info[0xF]) {
    case 'E':
        entry.region = "USA";
        break;
    case 'J':
        entry.region = "Japan";
        break;
    case 'O':
        entry.region = "International";
        break;
    case 'P': case 'W': case 'X': case 'Y': case 'Z':
        entry.region = "Europe";
        break;
    default:
        entry.region = "N/A";
        break;
    }

    file.close();

    entries.push_back(entry);
}

void GamesList::load_titles_database(std::string path) {
    std::ifstream file(path);
    std::string line;
    std::string delimiter = " = ";

    while (std::getline(file, line)) {
        std::string gamecode = line.substr(0, line.find(delimiter));
        std::string title = line.substr(line.find(delimiter));
        title = title.erase(title.find(delimiter), 3);
        titles_map[gamecode] = title;
    }
}

}