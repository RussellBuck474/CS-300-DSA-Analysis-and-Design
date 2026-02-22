// Advising Assistance Program
// Single-file C++ program for final project
// Reads a course data file, stores course objects in a vector, and provides a menu

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>

struct Course {
    std::string number;        // e.g., CSCI1301
    std::string title;         // course title
    std::vector<std::string> prereqs; // list of prerequisite course numbers
};

// Trim helpers
static inline std::string ltrim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    return s.substr(start);
}

// Natural (human) string comparison that treats digit runs as numbers.
// Returns true if a < b in natural order.
static bool naturalLess(const std::string &a, const std::string &b) {
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        unsigned char ca = static_cast<unsigned char>(a[i]);
        unsigned char cb = static_cast<unsigned char>(b[j]);
        if (std::isdigit(ca) && std::isdigit(cb)) {
            // find full digit run for both
            size_t ia = i, ib = j;
            while (ia < a.size() && std::isdigit(static_cast<unsigned char>(a[ia]))) ++ia;
            while (ib < b.size() && std::isdigit(static_cast<unsigned char>(b[ib]))) ++ib;

            // skip leading zeros for numeric comparison
            size_t aStart = i;
            while (aStart < ia && a[aStart] == '0') ++aStart;
            size_t bStart = j;
            while (bStart < ib && b[bStart] == '0') ++bStart;

            size_t aDigits = ia - aStart;
            size_t bDigits = ib - bStart;

            if (aDigits != bDigits) return aDigits < bDigits;

            // same number of significant digits: compare digit by digit
            for (size_t k = 0; k < aDigits; ++k) {
                char da = a[aStart + k];
                char db = b[bStart + k];
                if (da != db) return da < db;
            }

            // numbers are equal in value; if one has more leading zeros, treat shorter run as smaller
            size_t aRun = ia - i;
            size_t bRun = ib - j;
            if (aRun != bRun) return aRun < bRun;

            // advance past digit runs
            i = ia;
            j = ib;
            continue;
        }

        // non-digit comparison (case-insensitive)
        char A = static_cast<char>(std::toupper(ca));
        char B = static_cast<char>(std::toupper(cb));
        if (A != B) return A < B;
        ++i; ++j;
    }
    // shorter string wins
    return a.size() < b.size();
}
static inline std::string rtrim(const std::string &s) {
    if (s.empty()) return "";
    size_t end = s.size() - 1;
    while (end != static_cast<size_t>(-1) && std::isspace(static_cast<unsigned char>(s[end]))) {
        if (end == 0) return "";
        --end;
    }
    return s.substr(0, end + 1);
}
static inline std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

// Split a string by a delimiter char
static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        std::string t = trim(item);
        if (!t.empty()) out.push_back(t);
    }
    return out;
}

// Parse a single input line into a Course object.
// Expected format per line: <course number>,<course title>,<prereq1;prereq2;...>
// Prerequisites field is optional.
// Note: parsing for building/validation is handled by validateAndBuildVector

// Build Course objects from validated raw lines.
static std::vector<Course> validateAndBuildVector(const std::vector<std::string> &rawLines, const std::vector<std::string> &validCourseNumbers) {
    std::vector<Course> courseList;
    for (const auto &line : rawLines) {
        // split by comma into tokens
        std::vector<std::string> tokens = split(line, ',');
        if (tokens.size() < 2) continue; // defensive, should not happen

        std::string courseNumber = tokens[0];
        std::string courseTitle = tokens[1];

        // Expand any combined prereq token (e.g., "CSCI1100;MATH1001") into separate tokens
        std::vector<std::string> prereqTokens;
        for (size_t i = 2; i < tokens.size(); ++i) {
            const std::string &tok = tokens[i];
            if (tok.find(';') != std::string::npos || tok.find('|') != std::string::npos) {
                // normalize separators and split
                std::string norm;
                for (char c : tok) {
                    if (c == '|' ) norm += ';';
                    else norm += c;
                }
                std::vector<std::string> parts = split(norm, ';');
                for (auto &p : parts) prereqTokens.push_back(p);
            } else {
                if (!tok.empty()) prereqTokens.push_back(tok);
            }
        }

        std::vector<std::string> prereqList;
        for (const auto &pr : prereqTokens) {
            // check existence in validCourseNumbers
            bool found = false;
            for (const auto &v : validCourseNumbers) {
                if (v == pr) { found = true; break; }
            }
            if (!found) {
                std::cout << "Error: Prerequisite " << pr << " does not exist\n";
            }
            prereqList.push_back(pr);
        }

        Course c;
        c.number = courseNumber;
        c.title = courseTitle;
        c.prereqs = std::move(prereqList);
        courseList.push_back(std::move(c));
    }
    return courseList;
}

// Load courses from filename into vector (clears vector first)
static bool loadCoursesFromFile(const std::string &filename, std::vector<Course> &courses) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cout << "Error: Could not open course data file\n";
        // Print current working directory and files to help debugging where the program is running
        try {
            auto cwd = std::filesystem::current_path();
            std::cout << "Current working directory: " << cwd.string() << "\n";
            std::cout << "Files in working directory:\n";
            for (const auto &entry : std::filesystem::directory_iterator(cwd)) {
                std::cout << "  " << entry.path().filename().string() << "\n";
            }
        } catch (...) {
            // ignore filesystem errors
        }
        std::cout << "Tip: Place the CSV file in the program's working directory (usually the executable folder, e.g., Debug\\ or Release\\),\n";
        std::cout << "or provide the full path to the file. In Visual Studio, you can set the file's property 'Copy to Output Directory' to 'Copy always'.\n";
        return false;
    }

    std::vector<std::string> validCourseNumbers;
    std::vector<std::string> rawLines;

    std::string line;
    while (std::getline(infile, line)) {
        std::string t = trim(line);
        if (t.empty()) continue;
        if (t[0] == '#') continue;
        // split to check tokens
        std::vector<std::string> tokens = split(t, ',');
        if (tokens.size() < 2) {
            std::cout << "Format Error: Missing course number or title\n";
            continue;
        }
        validCourseNumbers.push_back(tokens[0]);
        rawLines.push_back(t);
    }
    infile.close();

    courses = validateAndBuildVector(rawLines, validCourseNumbers);
    std::cout << "Loaded " << courses.size() << " course(s) from '" << filename << "'.\n";
    return true;
}

// Print a list of courses in alphanumeric order by course number
static void printCoursesAlphanumeric(const std::vector<Course> &courses) {
    if (courses.empty()) {
        std::cout << "No courses loaded. Use option 1 to load a course data file.\n";
        return;
    }
    // make a copy to sort
    std::vector<Course> copy = courses;
    std::sort(copy.begin(), copy.end(), [](const Course &a, const Course &b) {
        return naturalLess(a.number, b.number);
    });

    std::cout << "\nAlphanumeric list of courses:\n";
    for (const auto &c : copy) {
        std::cout << c.number << " - " << c.title;
        if (!c.prereqs.empty()) {
            std::cout << " (Prereqs: ";
            for (size_t i = 0; i < c.prereqs.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << c.prereqs[i];
            }
            std::cout << ")";
        }
        std::cout << "\n";
    }
}

// Find a course by number (case-insensitive). Returns pointer or nullptr.
static const Course* findCourse(const std::vector<Course> &courses, const std::string &number) {
    std::string target = number;
    std::transform(target.begin(), target.end(), target.begin(), [](unsigned char c){ return std::toupper(c); });
    for (const auto &c : courses) {
        std::string n = c.number;
        std::transform(n.begin(), n.end(), n.begin(), [](unsigned char ch){ return std::toupper(ch); });
        if (n == target) return &c;
    }
    return nullptr;
}

int main() {
    std::vector<Course> courses;
    bool running = true;
    while (running) {
        std::cout << "\nAdvising Assistance Menu:\n";
        std::cout << "  1) Load course data file\n";
        std::cout << "  2) Print alphanumeric list of all Computer Science courses\n";
        std::cout << "  3) Print course title and prerequisites for a course\n";
        std::cout << "  9) Exit program\n";
        std::cout << "Select an option: ";
        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::string junk;
            std::getline(std::cin, junk);
            std::cout << "Invalid input. Please enter a number from the menu.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                std::cout << "Enter filename to load (example: courses.txt): ";
                std::string filename;
                std::getline(std::cin, filename);
                filename = trim(filename);
                if (filename.empty()) {
                    std::cout << "Filename cannot be empty.\n";
                } else {
                    loadCoursesFromFile(filename, courses);
                }
                break;
            }
            case 2:
                printCoursesAlphanumeric(courses);
                break;
            case 3: {
                if (courses.empty()) {
                    std::cout << "No courses loaded. Use option 1 first.\n";
                    break;
                }
                std::cout << "Enter course number to look up (e.g., CSCI1301): ";
                std::string num;
                std::getline(std::cin, num);
                num = trim(num);
                if (num.empty()) {
                    std::cout << "Course number cannot be empty.\n";
                    break;
                }
                const Course* found = findCourse(courses, num);
                if (!found) {
                    std::cout << "Course '" << num << "' not found.\n";
                } else {
                    std::cout << found->number << " - " << found->title << "\n";
                    if (found->prereqs.empty()) std::cout << "Prerequisites: None\n";
                    else {
                        std::cout << "Prerequisites: ";
                        for (size_t i = 0; i < found->prereqs.size(); ++i) {
                            if (i) std::cout << ", ";
                            std::cout << found->prereqs[i];
                        }
                        std::cout << "\n";
                    }
                }
                break;
            }
            case 9:
                std::cout << "Exiting program. Goodbye.\n";
                running = false;
                break;
            default:
                std::cout << "Invalid menu option. Please select 1, 2, 3, or 9.\n";
                break;
        }
    }
    return 0;
}

