#ifndef WAVSCANNER_FILEHANDLER_H
#define WAVSCANNER_FILEHANDLER_H
#include <filesystem>
#include <string>
#include <vector>


struct FileHandler
{
    private:
        std::filesystem::path mPath;

    public:
        FileHandler(std::filesystem::path path);

        std::vector<std::string> getWavFiles() const;
};


#endif //WAVSCANNER_FILEHANDLER_H
