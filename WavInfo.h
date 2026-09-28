#ifndef WAVSCANNER_WAVINFO_H
#define WAVSCANNER_WAVINFO_H
#include <string>


struct WavInfo {
private:
    std::string mFilename;
    std::string mSampleRate;
    std::string mBitDepth;
    std::string mChannels;
    int mDurationSeconds;

public:
    WavInfo(std::string fileName, std::string sampleRate, std::string bitDepth, std::string channels,
            int durationSeconds);

    void outputData();
};


#endif //WAVSCANNER_WAVINFO_H
