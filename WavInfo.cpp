#include "WavInfo.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>

WavInfo::WavInfo(std::string fileName, int sampleRate, int bitDepth, int channels,
                 double durationSeconds)
    : mFilename(std::move(fileName)),
      mSampleRate(sampleRate),
      mBitDepth(bitDepth),
      mChannels(channels),
      mDurationSeconds(durationSeconds) {
}

void WavInfo::outputData() const {
    std::cout << "=== " << mFilename << " ===" << std::endl;
    std::cout << "Sample Rate: " << mSampleRate << " Hz" << std::endl;
    std::cout << "Bit Depth: " << mBitDepth << " bits" << std::endl;
    std::cout << "Channels: " << mChannels << std::endl;
    std::cout << "Duration: " << mDurationSeconds << " seconds" << std::endl;
    std::cout << std::endl;
}

WavInfo WavInfo::parse(const std::filesystem::path& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath.string());
    }

    // --- Read RIFF header (12 bytes) ---
    char riff[4];
    file.read(riff, 4);
    if (!file.good() || std::string(riff, 4) != "RIFF") {
        throw std::runtime_error("Not a valid RIFF file: " + filepath.string());
    }

    // Skip file size (4 bytes, little-endian uint32)
    uint32_t fileSize;
    file.read(reinterpret_cast<char*>(&fileSize), 4);

    // Read "WAVE" format identifier
    char wave[4];
    file.read(wave, 4);
    if (!file.good() || std::string(wave, 4) != "WAVE") {
        throw std::runtime_error("Not a valid WAV file: " + filepath.string());
    }

    int sampleRate = 0;
    int bitDepth = 0;
    int channels = 0;
    int byteRate = 0;
    uint32_t dataSize = 0;

    // --- Parse sub-chunks ---
    while (file.good()) {
        char chunkId[4];
        file.read(chunkId, 4);
        if (!file.good()) break;

        uint32_t chunkSize;
        file.read(reinterpret_cast<char*>(&chunkSize), 4);
        if (!file.good()) break;

        if (std::string(chunkId, 4) == "fmt ") {
            // Read format data (16 bytes for PCM)
            uint16_t audioFormat;
            file.read(reinterpret_cast<char*>(&audioFormat), 2);

            file.read(reinterpret_cast<char*>(&channels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            file.read(reinterpret_cast<char*>(&byteRate), 4);

            uint16_t blockAlign;
            file.read(reinterpret_cast<char*>(&blockAlign), 2);

            file.read(reinterpret_cast<char*>(&bitDepth), 2);

            // Skip any extension bytes beyond the standard 16
            if (file.good() && chunkSize > 16) {
                file.seekg(chunkSize - 16, std::ios::cur);
            }

        } else if (std::string(chunkId, 4) == "data") {
            dataSize = chunkSize;
            break;  // We have all the info we need
        } else {
            // Skip unknown chunks (handle odd-sized chunks with padding byte)
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (sampleRate == 0 || bitDepth == 0 || channels == 0) {
        throw std::runtime_error("Invalid WAV format (missing fmt data): " + filepath.string());
    }

    // Calculate duration from data size and audio parameters
    double duration = static_cast<double>(dataSize)
                    / (sampleRate * channels * (bitDepth / 8));

    std::string filename = filepath.filename().string();

    return WavInfo(filename, sampleRate, bitDepth, channels, duration);
}
