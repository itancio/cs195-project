#include <cctype>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <dirent.h>
#include "sokoban.hpp"

//#include <emscripten/emscripten.h>

static Sokoban soko({{
    "#######",
    "#  $ .#",
    "#@  ###",
    "#####",
}});
static std::string joined_board;

extern "C" {
void sokoban_initialize() {
    std::vector<std::vector<std::string>> levels{{
        // GRIGoRusha: Shito-Krito #86
        "  #########",
        "  #   #  @#",
        " ##  .$.# #",
        " #  ##  $ #",
        "##  .$*#$##",
        "#  #$#   # ",
        "#  . .# ## ",
        "####    #  ",
        "   ######  ",
    }};
    soko = {levels};
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
    return soko.move((Sokoban::Direction)*s);
}

bool sokoban_undo() {
    return soko.undo();
}

int sokoban_level() {
    return soko.level();
}
}

int main() {
    std::cout << "Hello Emscripten!\n";

    // Open directory
    // https://www.quora.com/How-do-I-read-all-files-in-a-folder-in-C
    // https://www.tutorialspoint.com/How-can-I-get-the-list-of-files-in-a-directory-using-C-Cplusplus
    struct dirent *entry;
    DIR *dir = opendir("src/levels/");
   
    if (dir == NULL) {
      return;
    }
    while ((entry = readdir(dir)) != NULL) {
        std::cout << entry->d_name << std::endl;
    }
    closedir(dir);




    // Read from the text file
    std::ifstream file("src/hello.txt");

    if (!file.is_open()) {
        std::cout << "File cannot be open.\n";
        return 1;
    }
    std::string line;

    std::vector<std::string> level;
    // Read file line by line
    while (std::getline(file, line)) {
        if (isalpha(line.at(0))) {
            break;
        }
        level.push_back(line);
    }

    for (auto &str : level) {
        std::cout << str << "\n";
    }

    // Close the file
    file.close();

    return;
}