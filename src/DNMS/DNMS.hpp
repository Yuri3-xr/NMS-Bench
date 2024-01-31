#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../App/NMS.hpp"

template <class T, class M, class S>
auto dNMS(const std::vector<Box<T, M, S>>& boxes, const S& iouThreshold)
    -> std::vector<std::uint32_t> {
    auto size = std::size(boxes);

    if ((int)size == 0) {
        // empty case
        return std::vector<std::uint32_t>{};
    }

    /*
        sort boxes by scores
    */

    auto dets = boxes;
    std::sort(std::begin(dets), std::end(dets));

    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    std::vector<uint8_t> suppressed(
        size, 0);  // Boxes which have already been suppressed

    std::vector<std::vector<S>> iouMatrix(size, std::vector<S>(size, 0));
    std::vector<S> iouLevel(size, 0);

    for (uint32_t i = 0; i < size; i++) {
        for (uint32_t j = i + 1; j < size; j++) {
            iouMatrix[i][j] = iouMatrix[j][i] = dets[i].IoU(dets[j]);
        }

        uint32_t deg = 0;
        S val = 0;
        for (uint32_t j = 0; j < size; j++) {
            if (j == i) continue;
            if (iouMatrix[i][j] > 1E-10) {
                deg += 1;
                val += iouMatrix[i][j];
            }
        }

        iouLevel[i] = getIoUhreshold(val, deg, iouThreshold);
    }

    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        if (suppressed[i] == 1) continue;
        keep.emplace_back(dets[i].id);

        for (uint32_t j = i + 1; j < size; j++) {
            if (suppressed[j] == 1) continue;

            auto iou = iouMatrix[i][j];
            if (iou > iouLevel[j]) {
                suppressed[j] = 1;
            }
        }
    }

    return keep;
}