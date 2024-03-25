#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../Utils/NMS.hpp"

template<class S>
S fLinear(S iou) {
    return (S)(1) - iou;
}

template<class S>
S fGaussian(S iou, S sigma) {
    return exp(-iou * iou / sigma);
}

template <class T, class M, class S>
auto softNMS(const std::vector<Box<T, M, S>>& boxes, const S& iouThreshold, double sigma, double min_score = 0.1, int method = 1)
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

    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    std::vector<uint8_t> suppressed(
        size, 0);  // Boxes which have already been suppressed

    keep.reserve(size);

    for (uint32_t i = 0; i < size; i++) {
        if (suppressed[i] == 1) continue;

        uint32_t max_pos = i;
        for (uint32_t j = i + 1; j < size; j++) {
            if (suppressed[j] == 1) continue;
            if (dets[j] < dets[max_pos]) {
                max_pos = j;
            }
        }

        std::swap(dets[i], dets[max_pos]);
        keep.emplace_back(dets[i].id);

        for (uint32_t j = i + 1; j < size; j++) {
            if (suppressed[j] == 1) continue;

            auto iou = dets[i].IoU(dets[j]);
            double weight = 1;
            if (method == 0) {
                // greedy nms
                if (iou >= iouThreshold) weight = 0;
            } else if (method == 1) {
                // linear F
                if (iou >= iouThreshold) weight = fLinear(iou);
            } else if (method == 2) {
                if (iou >= iouThreshold) weight = fGaussian(iou, sigma);
            } else {
                // error
                // TODO: ...
            }
            dets[j].score *= weight;
            if (dets[j].score < min_score) {
                suppressed[j] = 1;
            }
        }
    }

    return keep;
}