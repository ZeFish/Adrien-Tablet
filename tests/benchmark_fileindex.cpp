#include <iostream>
#include <vector>
#include <string>
#include <utility>

// Mocking Arduino String
class String : public std::string {
public:
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    String() : std::string() {}
};

// Mocking File and FS
class File {
public:
    std::string _name;
    bool _isDir;
    size_t _size;
    bool _isValid;

    File(std::string name, bool isDir, size_t size, bool isValid = true)
        : _name(name), _isDir(isDir), _size(size), _isValid(isValid) {}

    // Conversion to bool
    operator bool() const { return _isValid; }

    bool isDirectory() { return _isDir; }
    const char* name() { return _name.c_str(); }
    size_t size() { return _size; }
};

class MockRoot {
public:
    int _fileCount;
    int _currentIndex;
    mutable int _readCount; // To track performance

    MockRoot(int count) : _fileCount(count), _currentIndex(0), _readCount(0) {}

    File openNextFile() {
        if (_currentIndex >= _fileCount) {
            return File("", false, 0, false);
        }
        _readCount++;
        _currentIndex++;
        // Create dummy files. Every 10th is a directory.
        bool isDir = (_currentIndex % 10 == 0);
        return File("item" + std::to_string(_currentIndex), isDir, 1024);
    }

    void reset() {
        _currentIndex = 0;
        _readCount = 0;
    }
};

#define MAX_BTN_NUM 14

void run_baseline(MockRoot& root) {
    std::vector<String> floders;
    std::vector<std::pair<String, size_t>> files;

    std::cout << "Running Baseline..." << std::endl;

    for (File file = root.openNextFile(); file; file = root.openNextFile()) {
        if (file.isDirectory()) {
            floders.push_back(String(file.name()));
        } else {
            files.push_back(std::make_pair(String(file.name()), file.size()));
        }
    }

    std::cout << "Baseline Read Count: " << root._readCount << std::endl;
}

void run_optimized(MockRoot& root) {
    std::vector<String> floders;
    std::vector<std::pair<String, size_t>> files;

    std::cout << "Running Optimized..." << std::endl;

    for (File file = root.openNextFile(); file; file = root.openNextFile()) {
        if (file.isDirectory()) {
            floders.push_back(String(file.name()));
        } else {
            files.push_back(std::make_pair(String(file.name()), file.size()));
        }

        // Optimization: Stop if we have enough items
        // We can display at most MAX_BTN_NUM + 1 items (0 to MAX_BTN_NUM)
        // logic in frame_fileindex.cpp loops:
        // for n in floders: if _key_files.size() > MAX_BTN_NUM break
        // for n in files: if _key_files.size() > MAX_BTN_NUM break

        // Total items we can show is 15.
        if (floders.size() + files.size() >= (MAX_BTN_NUM + 1)) {
             break;
        }
    }

    std::cout << "Optimized Read Count: " << root._readCount << std::endl;
}

int main() {
    // Scenario 1: 100 items
    MockRoot root(100);

    run_baseline(root);

    root.reset();
    run_optimized(root);

    return 0;
}
