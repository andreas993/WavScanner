## Part 1: Understanding the WAV File Format

Before writing any code, I needed to understand how WAV files are structured. WAV files follow the **RIFF** (Resource Interchange File Format) standard. Let me explain the structure:

### The RIFF/WAV Binary Layout

```
┌─────────────────────────────────────┐
│         RIFF HEADER (12 bytes)       │
├─────────────────────────────────────┤
│         SUB-CHUNKS                   │
│   ┌───────────┐                      │
│   │ "fmt "    │  ← Audio format info │
│   ├───────────┤                      │
│   │ "data"    │  ← Audio samples     │
│   ├───────────┤                      │
│   │ "info"    │  ← Metadata (optional)│
│   └───────────┘                      │
└─────────────────────────────────────┘
```

### Chunk Structure

Every chunk in a WAV file follows this pattern:

```
┌──────────┬──────────┬──────────────────┐
│  ChunkID  │ ChunkSize│    ChunkData     │
│  (4 bytes)│ (4 bytes)│   (N bytes)      │
└──────────┴──────────┴──────────────────┘
```

---

## Part 2: The RIFF Header (First 12 Bytes)

Every WAV file starts with this:

```
Offset  Size  Content        Example
──────  ────  ───────        ───────
   0      4   "RIFF"         Magic number identifying RIFF format
   4      4   FileSize       File size minus 8 bytes (uint32, little-endian)
   8      4   "WAVE"         Must be "WAVE" for WAV files
```

**In code (lines 32-48 of WavInfo.cpp):**

```cpp
// Read the "RIFF" magic bytes
char riff[4];
file.read(riff, 4);
if (!file.good() || std::string(riff, 4) != "RIFF") {
    throw std::runtime_error("Not a valid RIFF file");
}

// Skip the file size (we don't need it)
uint32_t fileSize;
file.read(reinterpret_cast<char*>(&fileSize), 4);

// Read the "WAVE" identifier
char wave[4];
file.read(wave, 4);
if (!file.good() || std::string(wave, 4) != "WAVE") {
    throw std::runtime_error("Not a valid WAV file");
}
```

**Why this matters:** If a file doesn't start with `"RIFF"` followed by `"WAVE"`, it's not a valid WAV file, and we should reject it early.

---

## Part 3: The "fmt " Chunk (Format Information)

This is the most important chunk — it tells us the audio properties.

### fmt Chunk Layout

```
Offset  Size  Field          Type          Description
──────  ────  ───────        ────          ───────────
   0      2   AudioFormat    uint16        1 = PCM (uncompressed)
   2      2   Channels       uint16        1=mono, 2=stereo
   4      4   SampleRate     uint32        e.g., 44100 or 48000
   8      4   ByteRate       uint32        SampleRate × Channels × Bits/8
  12      2   BlockAlign     uint16        Channels × Bits/8
  14      2   BitsPerSample  uint16        e.g., 16 or 24
```

**In code (lines 66-83 of WavInfo.cpp):**

```cpp
if (std::string(chunkId, 4) == "fmt ") {
    // Read audio format (1 = PCM)
    uint16_t audioFormat;
    file.read(reinterpret_cast<char*>(&audioFormat), 2);

    // Read channels (2 bytes)
    file.read(reinterpret_cast<char*>(&channels), 2);
    // Read sample rate (4 bytes)
    file.read(reinterpret_cast<char*>(&sampleRate), 4);
    // Read byte rate (4 bytes)
    file.read(reinterpret_cast<char*>(&byteRate), 4);

    // Read block align (2 bytes)
    uint16_t blockAlign;
    file.read(reinterpret_cast<char*>(&blockAlign), 2);

    // Read bits per sample (2 bytes)
    file.read(reinterpret_cast<char*>(&bitDepth), 2);

    // Skip extension bytes if chunk is larger than 16 bytes
    if (file.good() && chunkSize > 16) {
        file.seekg(chunkSize - 16, std::ios::cur);
    }
}
```

**Key insight:** We read each field using `file.read()` and cast it to the correct C++ type using `reinterpret_cast`. This reads the raw bytes directly into the variable.

---

## Part 4: The "data" Chunk (Audio Samples)

This chunk contains the actual audio samples. We only need its **size**, not the samples themselves.

### data Chunk Layout

```
Offset  Size  Field        Type      Description
──────  ────  ───────      ────      ───────────
   0      4   "data"        char[4]  Chunk identifier
   4      4   DataSize      uint32   Number of sample bytes
   8     N   AudioData     char[]   The actual audio samples
```

**In code (lines 85-87 of WavInfo.cpp):**

```cpp
} else if (std::string(chunkId, 4) == "data") {
    dataSize = chunkSize;  // Store the size, don't read the samples
    break;                 // We have everything we need!
}
```

**Why `break`?** Once we find the data chunk, we have all the information needed:
- Sample rate (from fmt)
- Bit depth (from fmt)
- Channels (from fmt)
- Data size (from data chunk)

---

## Part 5: The Chunk Parsing Loop

The core logic that walks through the entire file:

**In code (lines 56-92 of WavInfo.cpp):**

```cpp
while (file.good()) {
    // Read chunk ID (4 bytes)
    char chunkId[4];
    file.read(chunkId, 4);
    if (!file.good()) break;

    // Read chunk size (4 bytes)
    uint32_t chunkSize;
    file.read(reinterpret_cast<char*>(&chunkSize), 4);
    if (!file.good()) break;

    if (std::string(chunkId, 4) == "fmt ") {
        // Parse format information...
    } else if (std::string(chunkId, 4) == "data") {
        // Found data chunk, break out
    } else {
        // Skip unknown chunks
        file.seekg(chunkSize, std::ios::cur);
    }
}
```

**How it works step by step:**

1. **Read chunk ID** — What type of chunk is this?
2. **Read chunk size** — How many bytes of data follow?
3. **Check the ID:**
   - If `"fmt "` → Read and store the format data
   - If `"data"` → Store the size and **stop** (we're done!)
   - If anything else → **Skip** this chunk entirely

**Why skip unknown chunks?** WAV files can have optional chunks like `"info"`, `"bext"`, etc. We ignore them because we only care about `"fmt "` and `"data"`.

---

## Part 6: Calculating Duration

This is where the math comes in!

### The Formula

```
Duration = DataSize / (SampleRate × Channels × BitsPerSample / 8)
```

### Why This Formula?

Let's break it down:

| Component | Meaning |
|-----------|---------|
| **BitsPerSample / 8** | Converts bits to **bytes** per sample |
| **Channels × (Bits/8)** | Bytes per **one frame** (all channels together) |
| **SampleRate × Channels × (Bits/8)** | Bytes per **second** (byte rate) |
| **DataSize / ByteRate** | **Seconds** of audio |

### Example Calculation

For a file with:
- Data size = 7,200,000 bytes
- Sample rate = 48,000 Hz
- Channels = 2 (stereo)
- Bit depth = 24 bits

```
Bytes per second = 48000 × 2 × (24 / 8)
                 = 48000 × 2 × 3
                 = 288,000 bytes/second

Duration = 7,200,000 / 288,000
         = 25.0 seconds
```

**In code (line 99-100 of WavInfo.cpp):**

```cpp
double duration = static_cast<double>(dataSize)
                / (sampleRate * channels * (bitDepth / 8));
```

---

## Part 7: The Main Function Flow

Now let's trace through what happens when you run the program:

### Step-by-Step Execution

```
1. Program starts in main()
   │
   ├─► Set path = "E:\Development\C++\WavScanner"
   │
   ├─► Check if directory is empty → No, continue
   │
   ├─► Call getWavFiles(path)
   │    │
   │    ├─► Create directory_iterator for the path
   │    ├─► Loop through each entry
   │    ├─► Check if extension == ".wav"
   │    └─► Return vector of 4 WAV file paths
   │
   ├─► Check if any WAV files found → Yes, 4 files
   │
   ├─► Print "Found 4 WAV file(s):"
   │
   └─► For EACH WAV file:
        │
        ├─► Try:
        │   │
        │   ├─► Call WavInfo::parse(filepath)
        │   │   │
        │   │   ├─► Open file in binary mode
        │   │   ├─► Read & validate RIFF header
        │   │   ├─► Loop through chunks
        │   │   ├─► Parse "fmt " → get sampleRate, bitDepth, channels
        │   │   ├─► Parse "data" → get dataSize
        │   │   ├─► Calculate duration
        │   │   └─► Return WavInfo object
        │   │
        │   ├─► Call wavInfo.outputData()
        │   │   └─► Print formatted info to console
        │   │
        │   └─► Catch exceptions → Print error message
        │
        └─► Return 0 (success)
```

---

## Part 8: Error Handling

The code handles errors at every stage:

| Error | How it's caught |
|-------|-----------------|
| File can't be opened | `file.is_open()` check → throws exception |
| Not a RIFF file | `"RIFF"` check → throws exception |
| Not a WAV file | `"WAVE"` check → throws exception |
| Missing fmt data | `sampleRate == 0` check → throws exception |
| Invalid file | Caught by `try/catch` in main() → prints error |

---

## Part 9: Key C++ Concepts Used

| Concept | Where it's used |
|---------|-----------------|
| **`reinterpret_cast`** | Reading raw bytes into integer types |
| **`std::ifstream`** | Binary file reading |
| **`std::filesystem`** | Directory iteration, path handling |
| **`std::runtime_error`** | Throwing descriptive error messages |
| **`const` methods** | `outputData() const` — doesn't modify object |
| **`std::move`** | Efficient string transfer in constructor |
| **Range-based for loops** | Clean iteration over files |

---

## Visual Summary

```
WAV File on Disk                    Your Code
─────────────                       ───────────
"RIFF" ────────────────┐            ✅ Validates RIFF
FileSize ──────────────┤            ⏭️  Skipped
"WAVE" ────────────────┘            ✅ Validates WAVE
                                   
"fmt " ────────────────┐            ✅ Reads fmt chunk
  AudioFormat           │            → audioFormat
  Channels ─────────────┤            → channels (2)
  SampleRate ───────────┤            → sampleRate (48000)
  ByteRate              │            → byteRate
  BlockAlign            │            → blockAlign
  BitsPerSample ────────┘            → bitDepth (24)
                                   
"data" ────────────────┐            ✅ Reads data chunk
  DataSize ─────────────┘            → dataSize (7200000)
  Audio Samples               ⏭️  Not read (too large)
                                   
                                    CALCULATION:
  Duration = dataSize / (48000 × 2 × 3)
           = 7200000 / 288000
           = 25.0 seconds
                                   
OUTPUT:
  === filename.wav ===
  Sample Rate: 48000 Hz
  Bit Depth: 24 bits
  Channels: 2
  Duration: 25.0 seconds
```

---

That's the complete picture! The code reads binary data directly from the WAV file, extracts the format information from the `"fmt "` chunk, gets the data size from the `"data"` chunk, and calculates the duration using simple arithmetic.
