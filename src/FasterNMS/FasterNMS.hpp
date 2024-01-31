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

    const unsigned int K = 3;

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

    /*
        Method1: dynamic insert
        map: coco yolov8_n 37.2
    */

    // CoverTree<Box<T, M, S>> coverTree;
    // keep.reserve(size);

    // for (uint32_t i = 0; i < size; i++) {
    //     auto check = coverTree.kNearestNeighbors(dets[i], K);

    //     for (const auto& p : check) {
    //         if (dets[i].id == p.id) continue;
    //         auto iou = dets[i].IoU(p);
    //         if (iou > iouThreshold && dets[i].score < p.score) {
    //             suppressed[dets[i].id] = 1;
    //         }
    //     }

    //     coverTree.insert(dets[i]);
    //     if (not suppressed[dets[i].id]) {
    //         keep.emplace_back(dets[i].id);
    //     }
    // }

    /*
        Method2: build at first, maybe get awful result
        map: coco yolov8_n 20.4
    */

    // CoverTree<Box<T, M, S>> coverTree(dets);
    // keep.reserve(size);

    // for (uint32_t i = 0; i < size; i++) {
    //     auto check = coverTree.kNearestNeighbors(dets[i], K);

    //     for (const auto& p : check) {
    //         if (dets[i].id == p.id) continue;
    //         auto iou = dets[i].IoU(p);
    //         if (iou > iouThreshold && dets[i].score < p.score) {
    //               suppressed[dets[i].id] = 1;
    //         }
    //     }

    //     if(not suppressed[dets[i].id]) {
    //         keep.emplace_back(dets[i].id);
    //     }
    // }

    /*
        Method 3: Just use set to store x + y
        lower bound prev / next / this to check
    */

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
        auto p = *(it);
        auto iou = dets[i].IoU(p);
        if (iou > iouThreshold && dets[i].score < p.score) {
            suppressed[dets[i].id] = 1;
            st.insert(dets[i]);
            continue;
        }

        if (std::next(it) != std::end(st)) {
            auto p = *(std::next(it));
            auto iou = dets[i].IoU(p);
            if (iou > iouThreshold && dets[i].score < p.score) {
                suppressed[dets[i].id] = 1;
                st.insert(dets[i]);
                continue;
            }
        }

        if (it != std::begin(st)) {
            auto p = *(std::prev(it));
            auto iou = dets[i].IoU(p);
            if (iou > iouThreshold && dets[i].score < p.score) {
                suppressed[dets[i].id] = 1;
                st.insert(dets[i]);
                continue;
            }
        }

        if (not suppressed[dets[i].id]) {
            keep.emplace_back(dets[i].id);
            st.insert(dets[i]);
        }

    }

    return keep;
}
