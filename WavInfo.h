#ifndef WAVSCANNER_WAVINFO_H
#define WAVSCANNER_WAVINFO_H
#include <string>
#include <cstdint>
#include <filesystem>


struct WavInfo {
private:
    std::string mFilename;
    int mSampleRate;
    int mBitDepth;
    int mChannels;
    double mDurationSeconds;

public:
    WavInfo(std::string fileName, int sampleRate, int bitDepth, int channels,
            double durationSeconds);

    void outputData() const;

    static WavInfo parse(const std::filesystem::path& filepath);
};


#endif //WAVSCANNER_WAVINFO_H
