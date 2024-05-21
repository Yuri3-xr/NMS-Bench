#pragma once

#include <algorithm>
#include <any>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <execution>
#include <numeric>
#include <vector>

#include "../Utils/NMS.hpp"

template <class T, class M, class S>
auto clusterNMS(const std::vector<Box<T, M, S>>& boxes,
                const S& iouThreshold) -> std::vector<std::uint32_t> {
    auto size = std::size(boxes);

    if ((int)size == 0) {
        // empty case
        return std::vector<std::uint32_t>{};
    }

    auto dets = boxes;

    std::sort(std::execution::par_unseq, std::begin(dets), std::end(dets));

    std::vector<uint8_t> suppressed(size), b(size);
    for (int k = 0; k < 3; k++) {
        std::fill(suppressed.begin(), suppressed.end(), 0);
        std::for_each(
            std::execution::par_unseq, std::begin(dets), std::end(dets),
            [&dets, &size, &iouThreshold, &suppressed, &b](const auto& det) {
                auto i = &det - &dets[0];
                S t = 0;
                for (uint32_t j = 0; j < i; j++) {
                    if (!b[j]) t = std::max(t, dets[i].IoU(dets[j]));
                }
                if (t > iouThreshold) {
                    suppressed[i] = 1;
                }
            });
        b = suppressed;
    }

    std::vector<uint32_t> keep;  // Boxes to be kept
    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        if (!suppressed[i]) {
            keep.emplace_back(dets[i].id);
        }
    }

    return keep;
}
