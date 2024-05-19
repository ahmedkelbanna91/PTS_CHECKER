#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <limits>
#include <future>
#include <conio.h>
#include <chrono>
#include <Windows.h>



namespace fs = std::filesystem;

void enable_ansi_escape_codes() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hConsole, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, consoleMode);
}

bool is_txt_file(const fs::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".txt";
}

bool is_pts_file(const fs::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".pts";
}

fs::path change_extension_to_pts(const fs::path& path) {
    fs::path new_path = path;
    new_path.replace_extension(".pts");
    fs::rename(path, new_path);
    return new_path;
}

bool is_valid_line(const std::string& line) {
    std::istringstream iss(line);
    double x, y, z;
    return (iss >> x >> y >> z) && (iss.eof());
}

std::string process_line(const std::string& line) {
    std::istringstream iss(line);
    std::ostringstream oss;
    double x, y, z;

    iss >> x >> y >> z;
    oss << std::fixed << std::setprecision(6) << x << " " << y << " " << z;

    return oss.str();
}

bool process_file(const std::string& filepath) {
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "\x1B[91mFailed to modify:\x1B[0m   " << fs::path(filepath).filename().string() << "  \x1B[91m(unable to open)\x1B[0m" << std::endl;
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    bool first_line = true;
    bool checked_by_banna = false;
    int line_counter = 0;
    int total_line_count = 0;

    while (std::getline(infile, line)) {
        if (first_line) {
            if (line == "CHECKED_BY_BANNA") {
                checked_by_banna = true;
                break;
            }
            first_line = false;
        }
        if (is_valid_line(line)) {
            ++total_line_count;
        }
    }

    if (checked_by_banna) {
        infile.close();
        std::cout << "\x1B[93mNo need to modify:\x1B[0m " << fs::path(filepath).filename().string() << std::endl;
        return false;
    }

    infile.clear();  // Clear the EOF flag
    infile.seekg(0);  // Set the file position to the beginning

    std::string previous_line;
    double prev_x, prev_y, prev_z;
    double cur_x, cur_y, cur_z;
    int max_line_count = 1500;

    while (std::getline(infile, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }

        if (is_valid_line(line) && (total_line_count <= max_line_count || line_counter % 2 == 0)) {
            std::istringstream iss(line);
            iss >> cur_x >> cur_y >> cur_z;

            if (previous_line.empty() || (prev_x != cur_x || prev_y != cur_y || prev_z != cur_z)) {
                lines.push_back(process_line(line));
                prev_x = cur_x;
                prev_y = cur_y;
                prev_z = cur_z;
                previous_line = line;
            }
        }

        if (total_line_count > max_line_count) {
            ++line_counter;
        }
    }

    infile.close();

    std::ofstream outfile(filepath);
    if (!outfile.is_open()) {
        std::cerr << "\x1B[91mFailed to modify:\x1B[0m   " << fs::path(filepath).filename().string() << "  \x1B[91m(unable to write)\x1B[0m" << std::endl;
        return false;
    }

    outfile << "CHECKED_BY_BANNA" << std::endl;
    for (const auto& processed_line : lines) {
        outfile << processed_line << std::endl;
    }
    outfile << lines.size() << std::endl;

    outfile.close();
    return true;
}

int main() {
    enable_ansi_escape_codes();
    std::cout << "\n \x1B[93m============================'Created by Banna'===============================\x1B[0m" << std::endl;
    std::cout << " \x1B[93m============================='PTS CHECKER V1'================================\x1B[0m\n" << std::endl;

    bool txt2pts = false;
    std::string exe_path = fs::absolute(fs::current_path()).string();
    for (const auto& entry : fs::directory_iterator(exe_path)) {
        fs::path filepath = entry.path();
        if (is_txt_file(filepath)) {
            filepath = change_extension_to_pts(filepath);
            txt2pts = true;
        }
        if (is_pts_file(filepath)) {
            if (process_file(filepath.string())) {
                if(txt2pts) {
                    std::cout << "\x1B[92mModified:\x1B[0m   " << fs::path(filepath).filename().string() << "  \x1B[92mTXT to PTS\x1B[0m" << std::endl;
                    txt2pts = false;
                }
                else {
                    std::cout << "\x1B[92mModified:\x1B[0m   " << fs::path(filepath).filename().string() << std::endl;
                }
            }
        }
    }

    // Exit program
    auto end_time = std::chrono::steady_clock::now();
    int Delay = 10;
    int remaining_seconds = Delay;
    std::cout << "\n" << std::endl;

    auto input_checker = std::async(std::launch::async, [&]() {
        while (remaining_seconds > 0) {
            if (_kbhit()) {
                _getch(); // Read the pressed key and discard it
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for a short duration
        }
    });

    while (remaining_seconds > 0 && input_checker.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
        auto xDelay = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - end_time).count();
        remaining_seconds = Delay - xDelay;
        std::cout << "\r\x1B[93mPress ENTER to EXIT or wait " << remaining_seconds << " seconds...\x1B[0m" << std::flush;
    }

    return EXIT_SUCCESS;
}
