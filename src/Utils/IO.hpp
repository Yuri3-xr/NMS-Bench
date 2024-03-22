#pragma once

#include <fstream>
#include <iomanip>
#include <vector>

#include "NMS.hpp"

template <class T, class M, class S>
auto input(std::vector<Box<T, M, S>>& boxes, std::fstream& inFile) -> void {
    uint32_t _img_id, _categoriy_id;
    T ltx, lty, rbx, rby, w, h;
    S score;
    uint32_t p = 0;
    std::string header;
    char ch;
    inFile >> header;
    while (inFile >> _img_id >> ch 
            >> _categoriy_id >> ch 
            >> ltx >> ch >> lty >> ch >> w >> ch >> h >> ch >> score) {
        ltx += _categoriy_id * 800, lty += _categoriy_id * 800;
        rbx = ltx + w, rby = lty + h;
        boxes.emplace_back(Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)),
                        score, p);
        // preds.emplace_back(_categoriy_id, box);
        // set_img_id(_img_id);
        p++;
    }

    return ;

    // T ltx, lty, rbx, rby;
    // S score;

    // int p = 0;

    // while (inFile >> ltx >> lty >> rbx >> rby >> score) {
    //     boxes.emplace_back(Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)),
    //                        score, p);
    //     p++;
    // }

    // return;
}

auto output(const std::vector<uint32_t>& keep, std::fstream& outFile) -> void {
    for (const auto& id : keep) {
        outFile << id << '\n';
    }

    return;
}