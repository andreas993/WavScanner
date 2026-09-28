#include "WavInfo.h"

#include <iostream>
#include <ostream>

WavInfo::WavInfo(std::string fileName, std::string sampleRate, std::string bitDepth, std::string channels,
                 int durationSeconds) {
    mFilename = fileName;
    mSampleRate = sampleRate;
    mBitDepth = bitDepth;
    mChannels = channels;
    mDurationSeconds = durationSeconds;
}

void WavInfo::outputData() {
    std::cout << mFilename << std::endl;
    std::cout << mSampleRate << std::endl;
    std::cout << mBitDepth << std::endl;
    std::cout << mChannels << std::endl;
    std::cout << std::to_string(mDurationSeconds) << std::endl;
}
