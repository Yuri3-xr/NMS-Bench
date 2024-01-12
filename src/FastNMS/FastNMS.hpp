#pragma once

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <vector>

#include "../App/NMS.hpp"

template <class T, class S>
auto fastNMS(const std::vector<Box<T, S>>& boxes, const S& iouThreshold)
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

    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        keep.emplace_back(dets[i].id);

        for (uint32_t j = i + 1; j < size; j++) {
            if (suppressed[j] == 1) continue;

            auto iou = dets[i].IoU(dets[j]);
            if (iou > iouThreshold) {
                suppressed[j] = 1;
            }
        }
    }

    return keep;
}