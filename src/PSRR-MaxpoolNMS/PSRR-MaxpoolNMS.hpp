#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <vector>
#include <cassert>
#include <map>

#include "../Utils/NMS.hpp"

double dist2(double x, double y) { return x * x + y * y; }

template <class T, class M, class S>
auto PSRRNMS(const std::vector<Box<T, M, S>>& boxes, const std::vector<uint32_t>& categories,
                const std::vector<int>& anchors, 
                const std::vector<double>& ratios,
                const int& _W = 640, const int& _H = 640, 
                const double& alpha = 0.75,
                const int& beta = 16) -> std::vector<uint32_t> {
    auto size = std::size(boxes);

    if ((int)size == 0) {
        // empty case
        return std::vector<uint32_t>{};
    }

    auto dets = boxes;
    std::vector<uint32_t> keep;  // Boxes which need to be keeped
    std::vector<uint8_t> suppressed(
        size, 0);  // Boxes which have already been suppressed

    keep.reserve(size);

    int W = std::round(static_cast<double>(_W) / beta) + 1, 
        H = std::round(static_cast<double>(_H) / beta) + 1;

    std::vector<std::vector<int>> category_boxes;
    std::map<int, int> mp; uint32_t cnt = 0;
    for (uint32_t i = 0; i < size; i++) {
        int c = categories[i];
        if (mp.count(c)) {
            category_boxes[mp[c]].emplace_back(i);
        } else {
            mp[c] = cnt++;
            category_boxes.push_back({static_cast<int>(i)});
        }
    }

    // init, generate ns * nr score maps
    uint32_t ns = anchors.size(), nr = ratios.size();
    std::vector<std::vector<std::vector<int>>> score_maps(ns, 
        std::vector<std::vector<int>>(nr, 
        std::vector<int>(W * H, -1)));

    std::vector<std::vector<std::vector<int>>> max_pools(ns, 
        std::vector<std::vector<int>>(nr, 
        std::vector<int>(W * H, -1)));
    std::vector<std::vector<int>> s_max_pools(ns, 
        std::vector<int>(W * H, -1));
    std::vector<std::vector<int>> r_max_pools(nr, 
        std::vector<int>(W * H, -1));
    std::vector<int> all_max_pools(W * H, -1);

    std::vector<std::vector<std::vector<int>>> max_pools_shift(ns, 
        std::vector<std::vector<int>>(nr, 
        std::vector<int>((W + 1) * (H + 1), -1)));
    std::vector<std::vector<int>> s_max_pools_shift(ns, 
        std::vector<int>((W + 1) * (H + 1), -1));
    std::vector<std::vector<int>> r_max_pools_shift(nr, 
        std::vector<int>((W + 1) * (H + 1), -1));
    std::vector<int> all_max_pools_shift((W + 1) * (H + 1), -1);
    std::vector<std::tuple<uint32_t, uint32_t, int>> spatial_maps(size);

    // init, generate w, h, kx, ky table for looking up
    std::vector<std::vector<int>> w(ns, std::vector<int>(nr, 0)), h(ns, std::vector<int>(nr, 0));
    std::vector<std::vector<int>> kx(ns, std::vector<int>(nr, 0)), ky(ns, std::vector<int>(nr, 0));
    std::vector<int> s_kx(ns, std::max(W, H)), s_ky(ns, std::max(W, H));
    std::vector<int> r_kx(nr, std::max(W, H)), r_ky(nr, std::max(W, H));
    int all_kx = std::max(W, H), all_ky = std::max(W, H);

    for (uint32_t s = 0; s < ns; s++) {
        for (uint32_t r = 0; r < nr; r++) {
            double _w = std::sqrt(static_cast<double>(anchors[s]) / ratios[r]);
            h[s][r] = std::round(_w * ratios[r]);
            w[s][r] = std::round(_w);
            
            kx[s][r] = std::max<int>(std::round(alpha * w[s][r] / beta), 1);
            ky[s][r] = std::max<int>(std::round(alpha * h[s][r] / beta), 1);
            
            s_kx[s] = std::min(s_kx[s], kx[s][r]);
            s_ky[s] = std::min(s_ky[s], ky[s][r]);

            r_kx[r] = std::min(r_kx[r], kx[s][r]);
            r_ky[r] = std::min(r_ky[r], ky[s][r]);

            all_kx = std::min(all_kx, kx[s][r]);
            all_ky = std::min(all_ky, ky[s][r]);
        }
    }

    for (auto IDX : category_boxes) {

        // spatial & channel recovery
        for (auto i : IDX) {
            
            // spatial recovery
            int X = std::floor(dets[i].getMidPoint().x / beta);
            int Y = std::floor(dets[i].getMidPoint().y / beta);
            // if (X >= W || Y >= H) {
            //     std::cerr << X << ", " << Y << std::endl;
            //     std::cerr << dets[i].rect.lt.x << ", " << dets[i].rect.lt.y << std::endl;
            //     std::cerr << dets[i].rect.rb.x << ", " << dets[i].rect.rb.y << std::endl;
            //     std::cerr << dets[i].getMidPoint().x / beta << std::endl;
            //     std::cerr << dets[i].getMidPoint().y / beta << std::endl;
            // }
            assert(X >= 0 && X < W);
            assert(Y >= 0 && Y < H);
            int idx = X + Y * W;

            // channeal recovery
            double area = dets[i].rect.area();
            double ratio = (dets[i].rect.rb - dets[i].rect.lt).y / (dets[i].rect.rb - dets[i].rect.lt).x;

            // std::cerr << i << ": " << area << ", " << ratio << " " << idx << std::endl;
            uint32_t s_map = 0, r_map = 0;
            for (uint32_t s = 0; s < ns; s++) {
                for (uint32_t r = 0; r < nr; r++) {
                    // find the nearest channel
                    if (dist2(area - anchors[s], ratio - ratios[r]) < 
                        dist2(area - anchors[s_map], ratio - ratios[r_map])) {
                        s_map = s, r_map = r;
                    }
                }
            }
            // std::cerr << s_map << ", " << r_map << std::endl;
            // std::cerr << score_maps[s_map][r_map][idx] << std::endl;
            int j = score_maps[s_map][r_map][idx];
            if (j == -1 || dets[j].score < dets[i].score) {
                // std::cerr << "?" << std::endl;
                score_maps[s_map][r_map][idx] = i;
            }

            spatial_maps[i] = std::tuple<uint32_t, uint32_t, int>(s_map, r_map, idx);
            // std::cerr << "ok" << std::endl;
        }

        for (auto i : IDX) {
            if (suppressed[i] == 1) continue;
            auto [s, r, idx] = spatial_maps[i];
            int X = std::floor(dets[i].getMidPoint().x / beta);
            int Y = std::floor(dets[i].getMidPoint().y / beta);
            
            // single-channel maxpool
            {
                int x = X / kx[s][r], y = Y / ky[s][r];

                int j = max_pools[s][r][x + y * W];
                if (j == -1) {
                    max_pools[s][r][x + y * W] = i;
                } else {
                    if (dets[j].score > dets[i].score) {
                        suppressed[i] = 1;
                    } else {
                        suppressed[j] = 1;
                        max_pools[s][r][x + y * W] = i;
                    }
                }

                // shifted maxpool
                // int shift_x = std::round(static_cast<double>(kx[s][r]) / 2);
                // int shift_y = std::round(static_cast<double>(ky[s][r]) / 2);
                // x = (X >= shift_x) ? (x - shift_x) / kx[s][r] + 1 : 0;
                // y = (Y >= shift_y) ? (y - shift_y) / ky[s][r] + 1 : 0;
                // if (x > 0 && y > 0) {
                //     j = max_pools_shift[s][r][x + y * W];
                //     if (j == -1) {
                //         max_pools_shift[s][r][x + y * W] = i;
                //     } else {
                //         if (dets[j].score > dets[i].score) {
                //             suppressed[i] = 1;
                //         } else {
                //             suppressed[j] = 1;
                //             max_pools_shift[s][r][x + y * W] = i;
                //         }
                //     }
                // }
            }

            // cross-ratio maxpool
            {
                int x = X / s_kx[s], y = Y / s_ky[s];
                int j = s_max_pools[s][x + y * W];
                if (j == -1) {
                    s_max_pools[s][x + y * W] = i;
                } else {
                    if (dets[j].score > dets[i].score) {
                        suppressed[i] = 1;
                    } else {
                        suppressed[j] = 1;
                        s_max_pools[s][x + y * W] = i;
                    }
                }

                // shifted maxpool
                // int shift_x = std::round(static_cast<double>(s_kx[s]) / 2);
                // int shift_y = std::round(static_cast<double>(s_ky[s]) / 2);
                // x = (X >= shift_x) ? (x - shift_x) / s_kx[s] + 1 : 0;
                // y = (Y >= shift_y) ? (y - shift_y) / s_ky[s] + 1 : 0;
                // if (x > 0 && y > 0) {
                //     j = s_max_pools_shift[s][x + y * W];
                //     if (j == -1) {
                //         s_max_pools_shift[s][x + y * W] = i;
                //     } else {
                //         if (dets[j].score > dets[i].score) {
                //             suppressed[i] = 1;
                //         } else {
                //             suppressed[j] = 1;
                //             s_max_pools_shift[s][x + y * W] = i;
                //         }
                //     }
                // }
            }

            // cross-scale maxpool
            {
                int x = X / r_kx[r], y = Y / r_ky[r];
                int j = r_max_pools[r][x + y * W];
                if (j == -1) {
                    r_max_pools[r][x + y * W] = i;
                } else {
                    if (dets[j].score > dets[i].score) {
                        suppressed[i] = 1;
                    } else {
                        suppressed[j] = 1;
                        r_max_pools[r][x + y * W] = i;
                    }
                }

                // shifted maxpool
                // int shift_x = std::round(static_cast<double>(r_kx[r]) / 2);
                // int shift_y = std::round(static_cast<double>(r_ky[r]) / 2);
                // x = (X >= shift_x) ? (x - shift_x) / r_kx[r] + 1 : 0;
                // y = (Y >= shift_y) ? (y - shift_y) / r_ky[r] + 1 : 0;
                // if (x > 0 && y > 0) {
                //     j = r_max_pools_shift[r][x + y * W];
                //     if (j == -1) {
                //         r_max_pools_shift[r][x + y * W] = i;
                //     } else {
                //         if (dets[j].score > dets[i].score) {
                //             suppressed[i] = 1;
                //         } else {
                //             suppressed[j] = 1;
                //             r_max_pools_shift[r][x + y * W] = i;
                //         }
                //     }
                // }
            }

            // // cross-all maxpool
            {
                int x = X / all_kx, y = Y / all_ky;
                int j = all_max_pools[x + y * W];
                if (j == -1) {
                    all_max_pools[x + y * W] = i;
                } else {
                    if (dets[j].score > dets[i].score) {
                        suppressed[i] = 1;
                    } else {
                        suppressed[j] = 1;
                        all_max_pools[x + y * W] = i;
                    }
                }

                // shifted maxpool
                // int shift_x = std::round(static_cast<double>(all_kx) / 2);
                // int shift_y = std::round(static_cast<double>(all_ky) / 2);
                // x = (X >= shift_x) ? (x - shift_x) / all_kx + 1 : 0;
                // y = (Y >= shift_y) ? (y - shift_y) / all_ky + 1 : 0;
                // if (x > 0 && y > 0) {
                //     j = all_max_pools_shift[x + y * W];
                //     if (j == -1) {
                //         all_max_pools_shift[x + y * W] = i;
                //     } else {
                //         if (dets[j].score > dets[i].score) {
                //             suppressed[i] = 1;
                //         } else {
                //             suppressed[j] = 1;
                //             all_max_pools_shift[x + y * W] = i;
                //         }
                //     }
                // }
            }
        }

        for (auto i : IDX) {
            // reuse, O(n_i) roll back
            auto [s, r, idx] = spatial_maps[i];
            score_maps[s][r][idx] = -1;
            int X = std::floor(dets[i].getMidPoint().x / beta);
            int Y = std::floor(dets[i].getMidPoint().y / beta);

            // single-channel maxpool
            {
                int x = X / kx[s][r], y = Y / ky[s][r];
                max_pools[s][r][x + y * W] = -1;

                int shift_x = std::round(static_cast<double>(kx[s][r]) / 2);
                int shift_y = std::round(static_cast<double>(ky[s][r]) / 2);
                x = (X >= shift_x) ? (x - shift_x) / kx[s][r] + 1 : 0;
                y = (Y >= shift_y) ? (y - shift_y) / ky[s][r] + 1 : 0;
                
                max_pools_shift[s][r][x + y * W] = -1;
            }

            // cross-ratio maxpool
            {
                int x = X / s_kx[s], y = Y / s_ky[s];
                s_max_pools[s][x + y * W] = -1;

                int shift_x = std::round(static_cast<double>(s_kx[s]) / 2);
                int shift_y = std::round(static_cast<double>(s_ky[s]) / 2);
                x = (X >= shift_x) ? (x - shift_x) / s_kx[s] + 1 : 0;
                y = (Y >= shift_y) ? (y - shift_y) / s_ky[s] + 1 : 0;

                s_max_pools_shift[s][x + y * W] = -1;
            }

            // cross-scale maxpool
            {
                int x = X / r_kx[r], y = Y / r_ky[r];
                r_max_pools[r][x + y * W] = -1;

                int shift_x = std::round(static_cast<double>(r_kx[r]) / 2);
                int shift_y = std::round(static_cast<double>(r_ky[r]) / 2);
                x = (X >= shift_x) ? (x - shift_x) / r_kx[r] + 1 : 0;
                y = (Y >= shift_y) ? (y - shift_y) / r_ky[r] + 1 : 0;

                r_max_pools_shift[r][x + y * W] = -1;
            }

            // cross-all maxpool
            {
                int x = X / all_kx, y = Y / all_ky;
                all_max_pools[x + y * W] = -1;

                int shift_x = std::round(static_cast<double>(all_kx) / 2);
                int shift_y = std::round(static_cast<double>(all_ky) / 2);
                x = (X >= shift_x) ? (x - shift_x) / all_kx + 1 : 0;
                y = (Y >= shift_y) ? (y - shift_y) / all_ky + 1 : 0;

                all_max_pools_shift[x + y * W] = -1;
            }            
        }

    }

    for (uint32_t i = 0; i < size; i++) {
        if (suppressed[i]) continue;
        keep.emplace_back(dets[i].id);
    }

    return keep;
}
