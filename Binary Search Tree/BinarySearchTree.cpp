#include <filesystem>
#include <fstream>
...
void loadBids(string csvPath, BinarySearchTree* bst) {
    cout << "Loading CSV file " << csvPath << endl;

    // Diagnostic: print process working directory and file existence
    try {
        cout << "Working directory: " << std::filesystem::current_path() << endl;
    } catch(...) {
        // some older toolchains may not support std::filesystem::current_path()
    }
    if (!std::ifstream(csvPath)) {
        cerr << "CSV file not found or cannot be opened: " << csvPath << endl;
        cerr << "Use absolute path or copy the CSV into the working directory." << endl;
        // optionally return early or continue to let csv::Parser throw (your choice)
    }