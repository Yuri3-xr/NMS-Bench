#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <vector>

#include "../Utils/NMS.hpp"

template <class T, class M, class S>
auto boeNMS(const std::vector<Box<T, M, S>>& boxes,
            const S& iouThreshold) -> std::vector<uint32_t> {
    auto size = std::size(boxes);

    if ((int)size == 0) {
        // empty case
        return std::vector<uint32_t>{};
    }

    /*
        sort boxes by scores
    */

    auto dets = boxes;
    std::sort(std::begin(dets), std::end(dets));

    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    std::vector<uint8_t> suppressed(
        size, 0);  // Boxes which have already been suppressed
    std::vector<std::pair<Point<M>, uint32_t>> perm(size);

    for (uint32_t i = 0; i < size; i++) {
        perm[i] = std::make_pair(dets[i].getMidPoint(), i);
    }

    auto cmp = [](std::pair<Point<M>, uint32_t> x,
                  std::pair<Point<M>, uint32_t> y) -> bool {
        if (x.first < y.first) return true;
        if (y.first < x.first) return false;
        return x.second < y.second;
    };
    std::sort(perm.begin(), perm.end(), cmp);

    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        if (suppressed[i] == 1) continue;
        keep.emplace_back(dets[i].id);

        auto rect = dets[i].rect;
        uint32_t st = std::lower_bound(perm.begin(), perm.end(),
                                       std::make_pair(rect.lt, 0), cmp) -
                      perm.begin();
        uint32_t ed = std::upper_bound(perm.begin(), perm.end(),
                                       std::make_pair(rect.rb, 0), cmp) -
                      perm.begin();
        st = std::max<uint32_t>(st, 0);
        ed = std::min<uint32_t>(ed, size);
        // std::cerr << st << ", " << ed << std::endl;

        for (uint32_t _j = st; _j < ed; _j++) {
            uint32_t j = perm[_j].second;
            // uint32_t id = j;
            if (suppressed[j] == 1) continue;

            auto iou = dets[i].IoU(dets[j]);
            if (iou > iouThreshold) {
                suppressed[j] = 1;
            }
        }
    }

    return keep;
}