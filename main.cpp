#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include "WavInfo.h"


int main(int argc, char *argv[]) {
    const std::filesystem::path path = R"(D:\Development\C++\WavScanner)";

    std::filesystem::directory_iterator files{path};

    if (std::filesystem::is_empty(path)) {
        std::cout << "Nothing in this path" << std::endl;

        return -1;
    }

    std::vector<std::string> wavFiles;

    for (auto const &dir_entry: files) {
        if (dir_entry.path().extension() == ".wav") {
            wavFiles.push_back(dir_entry.path().string());
        }
    }

    if (wavFiles.empty()) {
        std::cout << "No wav files found !" << std::endl;
        return -1;
    }

    std::string wavFile = wavFiles.front();
    std::ifstream file{wavFile, std::ios::in |std::ios::binary};

    char riff[4];

    file.read(riff, 4);

    if (std::string(riff, 4) != "RIFF") {
        std::cout << "Not a valid WAV file\n";
        return -1;
    }

    std::string strLength;
    file.read(reinterpret_cast<char *>(&strLength), sizeof(strLength));


    WavInfo wavInfo{"test", "test", "test", "test", 2};

    wavInfo.outputData();
    // for (int i = 0; i == 3; ++i) {
    //     std::string byte = file.
    // }


    return 0;
}
