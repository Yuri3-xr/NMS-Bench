#pragma once

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../App/NMS.hpp"
#include "CoverTree.hpp"

constexpr unsigned int K = 50;

template <class T, class M, class S>
auto fasterNMS(const std::vector<Box<T, M, S>>& boxes, const S& iouThreshold)
    -> std::vector<std::uint32_t> {
    auto size = std::size(boxes);

    const unsigned int K = boxes.size() / 10.0;

    if ((int)size == 0) {
        // empty case
        return std::vector<std::uint32_t>{};
    }

    /*
        sort boxes by scores
    */

    auto dets = boxes;
    // std::sort(std::begin(dets), std::end(dets));

    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    std::vector<uint8_t> suppressed(
        size, 0);  // Boxes which have already been suppressed

    CoverTree<Box<T, M, S>> coverTree;
    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        auto check = coverTree.kNearestNeighbors(dets[i], K);

        for (const auto& p : check) {
            if (dets[i].id == p.id) continue;
            auto iou = dets[i].IoU(p);
            if (iou > iouThreshold && dets[i].score < p.score) {
                suppressed[dets[i].id] = 1;
            }
        }

        coverTree.insert(dets[i]);
        if (not suppressed[dets[i].id]) {
            keep.emplace_back(dets[i].id);
        }
    }

    return keep;
}