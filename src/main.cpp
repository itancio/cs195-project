#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

#include "sokoban.hpp"

//#include <emscripten/emscripten.h>

static Sokoban soko({{
    "#######",
    "#  $ .#",
    "#@  ###",
    "#####",
}});
static std::string joined_board;
static std::string sequence_str;
static std::vector<std::vector<std::string>> levels;

extern "C" {
void sokoban_initialize() {
    /* Read files from directory */
    const std::string source_path = "src/assets/levels";
    std::filesystem::path get_levels_dir = std::filesystem::directory_entry(source_path);
    std::vector<std::filesystem::path> get_map_path; // save path into vector element

    for (const auto& entry : std::filesystem::directory_iterator(get_levels_dir)) {
        get_map_path.push_back(entry.path());
    }

    // opening map files stored in vector directory
    for (unsigned int i = 0; i < get_map_path.size(); i++) {
        std::ifstream open_map_file(get_map_path[i].c_str());
        std::string store_line;
        std::vector<std::string> store_map;

        if (!open_map_file) {
            std::cerr << "Cannot open the File : " << get_map_path[i] << std::endl;
        }

        std::regex valid_elems("[#@$*.+]+");
        std::regex invalid_elems("[a-zA-z0-9]+");
        while (std::getline(open_map_file, store_line)) {
            if (std::regex_search(store_line, valid_elems)) {
                store_map.push_back(store_line);
            }
            if (std::regex_search(store_line, invalid_elems) and !store_map.empty()) {
                break;
            }

        }
        levels.push_back(store_map);
    };

    soko = { levels };
}

const char *sokoban_board_to_string() {
    auto board = soko.board();
    joined_board = std::accumulate(
        std::begin(board), std::end(board), std::string(),
        [](std::string &ss, std::string &s)
        { return ss.empty() ? s : ss + "\n" + s; }
    );
    return joined_board.c_str();
}


bool sokoban_move(char *s) {
    return soko.move((Sokoban::Direction) *s);
}

bool sokoban_goto(int row, int col) {
    return soko.move(row, col);
}

bool sokoban_solved() {
    return soko.solved();
}

bool sokoban_undo() {
    return soko.rewind();
}

bool sokoban_redo() {
    return soko.redo();
}

const char *sokoban_sequence() {
    sequence_str = soko.sequence();
    return sequence_str.c_str();
}

int sokoban_level() {
    return soko.level();
}

void sokoban_change_level(int level) {
    soko.change_level(level);
}

int sokoban_levels_size() {
    return levels.size();
}
}

int main() {
    std::cout << "Hello Emscripten!\n";
}

