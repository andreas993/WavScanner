#include <iostream>
#include <filesystem>
#include <vector>

#include "WavInfo.h"


static std::vector<std::string> getWavFiles(const std::filesystem::path& dirPath) {
    std::vector<std::string> wavFiles;

    for (const auto& dir_entry : std::filesystem::directory_iterator(dirPath)) {
        if (dir_entry.path().extension() == ".wav") {
            wavFiles.push_back(dir_entry.path().string());
        }
    }

    return wavFiles;
}

int main(int argc, char *argv[]) {
    const std::filesystem::path path = R"(E:\Development\C++\WavScanner)";

    if (std::filesystem::is_empty(path)) {
        std::cout << "Nothing in this path" << std::endl;
        return -1;
    }

    std::vector<std::string> wavFiles = getWavFiles(path);

    if (wavFiles.empty()) {
        std::cout << "No WAV files found in this path" << std::endl;
        return -1;
    }

    std::cout << "Found " << wavFiles.size() << " WAV file(s):\n" << std::endl;

    for (const auto& wavFile : wavFiles) {
        try {
            WavInfo wavInfo = WavInfo::parse(wavFile);
            wavInfo.outputData();
        } catch (const std::exception& e) {
            std::cerr << "Error parsing " << wavFile << ": " << e.what() << std::endl;
        }
    }

    return 0;
}
