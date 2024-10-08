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
auto fastNMS_Par(const std::vector<Box<T, M, S>>& boxes,
                 const S& iouThreshold) -> std::vector<uint32_t> {
    auto size = std::size(boxes);

    if ((int)size == 0) {
        // empty case
        return std::vector<uint32_t>{};
    }

    auto dets = boxes;

    std::sort(std::execution::par_unseq, std::begin(dets), std::end(dets));

    std::vector<uint8_t> suppressed(size);
    std::for_each(std::execution::par_unseq, std::begin(dets), std::end(dets),
                  [&dets, &size, &iouThreshold, &suppressed](const auto& det) {
                      auto i = &det - &dets[0];
                      S t = 0;
                      for (uint32_t j = 0; j < i; j++) {
                          t = std::max(t, dets[i].IoU(dets[j]));
                      }
                      if (t > iouThreshold) {
                          suppressed[i] = 1;
                      }
                  });

    std::vector<uint32_t> keep;  // Boxes to be kept
    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        if (!suppressed[i]) {
            keep.emplace_back(dets[i].id);
        }
    }

    return keep;
}
