#pragma once
#include <fstream>
#include <vector>

#include "NMS.hpp"

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