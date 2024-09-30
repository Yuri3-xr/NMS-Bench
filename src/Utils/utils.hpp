#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "./NMS.hpp"

void string_split(std::stringstream& ss, const std::string& str, char ch) {
    std::istringstream iss(str);
    std::string buf;
    for (uint32_t i = 0; std::getline(iss, buf, ch); i++) {
        if (i) ss << " ";
        ss << buf;
    }
}

template <class T, class M, class S>
auto input(std::vector<Box<T, M, S>>& boxes, std::fstream& inFile) -> void {
    T ltx, lty, rbx, rby;
    S score;

    int p = 0;

    while (inFile >> ltx >> lty >> rbx >> rby >> score) {
        boxes.emplace_back(Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)),
                           score, p);
        p++;
    }

    return;
}

auto output(const std::vector<uint32_t>& keep, std::fstream& outFile) -> void {
    for (const auto& id : keep) {
        outFile << id << '\n';
    }

    return;
}

struct Timer {
    std::chrono::high_resolution_clock::time_point st;

    Timer() { reset(); }

    void reset() { st = std::chrono::high_resolution_clock::now(); }

    std::chrono::milliseconds::rep elapsed() {
        auto ed = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(ed - st)
            .count();
    }
};
