#include <fstream>
#include <glob.h>
#include <iostream>
#include <string>
#include <vector>


// SOURCE: Chris Redford
// https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
std::vector<std::string> load_files(const std::string& pattern) {
    glob_t glob_result;
    glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    std::vector<std::string> files;

    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        files.push_back(std::string(glob_result.gl_pathv[i]));
    }

    globfree(&glob_result);
    return files;
}

const std::vector<std::vector<std::string>> load_levels() {

    std::vector<std::string> filepaths = load_files("src/levels/*");
    std::vector<std::vector<std::string>> levels;

    for (const auto& path : filepaths) {
        // Read from the text file
        std::ifstream file(path);

        if (!file.is_open()) {
            throw std::invalid_argument("File cannot be open.\n");
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

        levels.push_back(level);

        file.close();
    }

    return levels;
}