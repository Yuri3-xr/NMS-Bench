#pragma once

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../App/NMS.hpp"
#include "CoverTree.hpp"

template <class T, class M, class S>
auto fasterNMS(const std::vector<Box<T, M, S>>& boxes, const S& iouThreshold)
    -> std::vector<std::uint32_t> {
    auto size = std::size(boxes);

    const unsigned int K = 10;  // dynamic rate

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

    auto cmp = [&](const Box<T, M, S>& _cmpa, const Box<T, M, S>& _cmpb) {
        return _cmpa.rect.midPoint.x + _cmpa.rect.midPoint.y <
               _cmpb.rect.midPoint.x + _cmpb.rect.midPoint.y;
    };
    keep.reserve(size);
    std::set<Box<T, M, S>, decltype(cmp)> st(cmp);
    keep.emplace_back(dets[0].id);
    st.insert(dets[0]);

    for (uint32_t i = 1; i < size; i++) {
        auto it = st.lower_bound(dets[i]);

        auto rightSolver = [&](auto it) -> bool {
            // from it to it + k - 1
            int k = K;
            if (it == std::end(st)) return false;
            while (k) {
                auto iou = dets[i].IoU(*it);
                if (iou > iouThreshold && dets[i].score < (it->score)) {
                    suppressed[dets[i].id] = 1;
                    return true;
                }
                if (std::next(it) != std::end(st)) {
                    it = std::next(it);
                    k--;
                } else {
                    break;
                }
            }

            return false;
        };

        auto leftSolver = [&](auto it) -> bool {
            // from it - k to it - 1
            int k = K;

            if (it == std::begin(st)) return false;
            it = std::prev(it);
            while (k) {
                auto iou = dets[i].IoU(*it);
                if (iou > iouThreshold && dets[i].score < (it->score)) {
                    suppressed[dets[i].id] = 1;
                    return true;
                }
                if (it != std::begin(st)) {
                    it = std::prev(it);
                    k--;
                } else {
                    break;
                }
            }

            return false;
        };

        if (!leftSolver(it)) rightSolver(it);

        if (not suppressed[dets[i].id]) {
            keep.emplace_back(dets[i].id);
        }
        st.insert(dets[i]);
    }

    return keep;
}
