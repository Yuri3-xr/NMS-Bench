#pragma once

#include <limits.h>

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

#include "../Utils/NMS.hpp"

template <class T, class M, class S>
int partition(std::vector<Box<T, M, S>> &boxes, int low, int high,
              const S &iouThreshold) {
    int p = low;
    for (int j = low; j < high; j++) {
        if (boxes[j].score > boxes[p].score) {
            p = j;
        }
    }
    std::swap(boxes[p], boxes[high - 1]);
    auto &pivot = boxes[high - 1];

    int i = low;
    for (int j = low; j < high - 1; j++) {
        if (boxes[j].getMidPoint() < pivot.getMidPoint()) {
            // if (boxes[j].score > pivot.score) {
            // if (boxes[j].getMidPoint().x + boxes[j].getMidPoint().y <
            // pivot.getMidPoint().x + pivot.getMidPoint().y) {
            std::swap(boxes[i], boxes[j]);
            i++;
        }
    }

    std::swap(boxes[i], boxes[high - 1]);
    return i;
}

template <class T, class M, class S>
void quickSort(std::vector<Box<T, M, S>> &boxes, int low, int high,
               std::vector<int> &dp, std::vector<uint32_t> &keep,
               const S &iouThreshold) {
    if (low >= high) return;

    // std::cerr << low << ", " << high << std::endl;

    int p = partition(boxes, low, high, iouThreshold);
    auto &pivot = boxes[p];

    if (dp[pivot.id]) {
        for (int i = low; i < high; i++) {
            if (i == p) continue;
            auto &b = boxes[i];
            if (pivot.IoU(b) > iouThreshold) {
                dp[b.id] = 0;
            }
        }
        keep.emplace_back(pivot.id);
    }

    quickSort<T, M, S>(boxes, low, p, dp, keep, iouThreshold);
    quickSort<T, M, S>(boxes, p + 1, high, dp, keep, iouThreshold);
}

template <class T, class M, class S>
auto qsiNMS(const std::vector<Box<T, M, S>> &boxes,
            const S &iouThreshold) -> std::vector<uint32_t> {
    auto size = std::size(boxes);

    if ((int)size == 0) {
        // empty case
        return std::vector<uint32_t>{};
    }

    /*
        sort boxes by scores
    */

    auto dets = boxes;
    // std::shuffle(dets.begin(), dets.end(), std::mt19937(time(0)));
    // std::sort(std::begin(dets), std::end(dets));

    // std::cerr << dets.size() << std::endl;

    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    keep.reserve(size);
    std::vector<int> dp(dets.size(), 1);
    quickSort<T, M, S>(dets, 0, dets.size(), dp, keep, iouThreshold);

    // for (auto box : dets) {
    //     std::cerr << box.score << ' ';
    // }
    // std::cerr << '\n';

    return keep;
}