#include <iostream>
#include <filesystem>
#include <vector>

#include "WavInfo.h"
#include "FileHandler.h"


int main()
{
    std::cout << "Enter the path to wav files: " << std::endl;

    std::string input;
    std::getline(std::cin, input);

    const std::filesystem::path path(input);

    const auto fileHandler = FileHandler(path);

    std::vector<std::string> wavFiles = fileHandler.getWavFiles();

    for (const auto &wavFile: wavFiles)
    {
        try
        {
            WavInfo wavInfo = WavInfo::parse(wavFile);
            wavInfo.outputData();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing " << wavFile << ": " << e.what() << std::endl;
        }
    }

    return 0;
}
