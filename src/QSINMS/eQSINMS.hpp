#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <stack>
#include <vector>

#include "../Utils/NMS.hpp"

template <class T, class M, class S>
auto eqsiNMS(const std::vector<Box<T, M, S>> &boxes,
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
    std::sort(std::begin(dets), std::end(dets),
              [&](const auto &x, const auto &y) {
                  return x.getMidPoint() < y.getMidPoint();
              });

    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    std::vector<int> dp(size, 1);

    keep.reserve(size);

    std::stack<int> id_stk;
    for (int i = 0; i < (int)size; i++) {
        while (!id_stk.empty() && dets[id_stk.top()].score < dets[i].score) {
            auto &stk_box = dets[id_stk.top()];
            if (dets[i].IoU(stk_box) > iouThreshold) {
                dp[stk_box.id] = 0;
            }
            id_stk.pop();
        }
        id_stk.push(i);
    }
    while (!id_stk.empty()) {
        id_stk.pop();
    }
    for (int i = (int)size - 1; i >= 0; i--) {
        while (!id_stk.empty() && dets[id_stk.top()].score < dets[i].score) {
            auto &stk_box = dets[id_stk.top()];
            if (dets[i].IoU(stk_box) > iouThreshold) {
                dp[stk_box.id] = 0;
            }
            id_stk.pop();
        }
        id_stk.push(i);
    }

    for (int i = 0; i < (int)size; i++) {
        if (dp[i]) {
            keep.emplace_back(i);
        }
    }

    return keep;
}