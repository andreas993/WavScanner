#include <iostream>
#include <utility>

#include "FileHandler.h"


FileHandler::FileHandler(std::filesystem::path path)
    : mPath(std::filesystem::path(std::move(path)))
{
}

std::vector<std::string> FileHandler::getWavFiles() const
{
    if (!std::filesystem::exists(mPath))
    {
        throw std::runtime_error("Path does not exist: " + mPath.string());
    }

    std::vector<std::string> wavFiles;

    for (const auto &dir_entry: std::filesystem::directory_iterator(mPath))
    {
        if (dir_entry.path().extension() == ".wav")
        {
            wavFiles.push_back(dir_entry.path().string());
        }
    }

    if (wavFiles.empty())
    {
        throw std::runtime_error("No WAV files found in this path: " + mPath.string());
    }

    std::cout << "Found " << wavFiles.size() << " WAV file(s):\n" << std::endl;

    return wavFiles;
}
