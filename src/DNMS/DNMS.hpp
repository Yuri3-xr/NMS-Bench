#pragma once

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <queue>
#include <vector>

#include "../App/NMS.hpp"

template <class S>
auto getIoUhreshold(const S& sigma, const S& iouThreshold) -> S {
    /*
        get current iouThreshold by val and degnums
    */

    S ret = 0.7;

    ret = std::min<S>(0.95, ret);

    return ret;
}

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
    
    S sumScore = 0;
    for (uint32_t i = 0; i < size; i++) {
        sumScore += dets[i].score;
        for (uint32_t j = i + 1; j < size; j++) {
            iouMatrix[i][j] = iouMatrix[j][i] = dets[i].IoU(dets[j]);
        }

        uint32_t cnt = 0;
        S sum = 0;
        for (uint32_t j = 0; j < size; j++) {
            if (j == i) continue;
            if (iouMatrix[i][j] > 1E-10) {
                cnt += 1;
                sum += iouMatrix[i][j];
            }
        }

        if (cnt == 0) {
            iouLevel[i] = iouThreshold;
            continue;
        }

        // at least one sample
        S mu = sum / cnt;  // mean
        S sigma = 0;       // variance

        for (uint32_t j = 0; j < size; j++) {
            if (j == i) continue;
            if (iouMatrix[i][j] > 1E-10) {
                sigma += (iouMatrix[i][j] - mu) * (iouMatrix[i][j] - mu);
            }
        }

        sigma /= static_cast<S>(cnt);

        iouLevel[i] = getIoUhreshold(sigma, iouThreshold);
    }

    sumScore /= static_cast<S>(size);
    std::cerr << sumScore << std::endl;
    // Soft NMS with dynamic IoU threshold
    keep.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        if (dets[i].score > sumScore * 0.04 || i == 0) {
            keep.emplace_back(dets[i].id);

            for (uint32_t j = i + 1; j < size; j++) {
                auto iou = dets[i].IoU(dets[j]);
                if (iou > iouLevel[j]) {
                    dets[j].score *= (1 - iouLevel[j]);
                }
            }
            if (i < size - 1)
                std::sort(std::begin(dets) + i + 1, std::end(dets));
        }
    }

    return keep;
}