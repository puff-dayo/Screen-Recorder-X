#include "get_screen.h"
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

std::string exec_command(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::pair<int, int> get_screen_size() {
    std::string output = exec_command("xrandr | grep '*' | awk '{print $1}'");
    
    size_t x_pos = output.find('x');
    if (x_pos == std::string::npos) {
        throw std::runtime_error("Could not parse screen size");
    }
    
    int width = std::stoi(output.substr(0, x_pos));
    int height = std::stoi(output.substr(x_pos + 1));
    
    return {width, height};
}