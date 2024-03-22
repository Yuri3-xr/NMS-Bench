#pragma once 

#include <map>
#include <vector>

template<class T>
class COCOMetrics {
  public:
    T ap50;
    std::map<uint32_t, uint32_t> id;
    std::vector<uint32_t> gts;
    std::vector<std::map<T, T>> rps;
    std::vector<std::vector<std::pair<T, char>>> tfs;


    COCOMetrics() :
        ap50(0), 
        id(std::map<uint32_t, uint32_t>()),
        gts(std::vector<uint32_t>()),
        rps(std::vector<std::map<T, T>>()), 
        tfs(std::vector<std::vector<std::pair<T, char>>>()) {}
    ~COCOMetrics() {}

    uint32_t get_id(uint32_t category_id) {
        if (!id.count(category_id)) {
            id[category_id] = id.size();
            gts.emplace_back(0);
            rps.emplace_back(std::map<T, T>());
            tfs.emplace_back(std::vector<std::pair<T, char>>());
        }
        return id[category_id];
    }

    void add_gt(uint32_t category_id, uint32_t x = 1) {
        uint32_t i = get_id(category_id);
        gts[i] += x;
        // std::cerr << category_id << ", " << i << ": " << gts[i] << std::endl;
    }

    void insert_tf(uint32_t category_id, T score, bool tf) {
        uint32_t i = get_id(category_id);
        tfs[i].emplace_back(-score, tf);
    }

    void insert_rp(uint32_t i, T r, T p) {
        // uint32_t i = get_id(category_id);
        if (!rps[i].size()) {
            rps[i][0] = 1;
            rps[i][1] = 0;
        }
        if (rps[i].count(r)) {
            rps[i][r] = std::max(rps[i][r], p);
        } else {
            rps[i][r] = p;
        }
    }

    void finalize() {
        uint32_t nc = id.size();
        for (uint32_t i = 0; i < nc; i++) {
            sort(tfs[i].begin(), tfs[i].end());
            uint32_t tp = 0, fp = 0, gt = gts[i];
            if (gt <= 0) continue;
            for (auto [score, tf] : tfs[i]) {
                if (tf) {
                    tp += 1;
                } else {
                    fp += 1;
                }
                T p = 1.0 * tp / (tp + fp);
                T r = 1.0 * tp / gt;
                // std::cerr << p << ", " << r << std::endl;
                insert_rp(i, r, p);
            }
        }
    }

    void evaluate() {
        uint32_t nc = rps.size();
        T ap50_sum = 0;
        uint32_t n = 0;
        for (uint32_t c = 0; c < nc; c++) {
            if (gts[c] <= 0) continue;
            n += 1;
            T sum = 0;
            uint32_t ns = 100;
            for (uint32_t i = 0; i <= ns; i++) {
                T r = i * 1.0 / ns;
                auto [rr, rp] = *rps[c].lower_bound(r);
                auto [lr, lp] = *(--rps[c].upper_bound(r));
                T p = (r * 2 > (rr + lr)) ? rp : lp;
                // std::cerr << r << " " << p << std::endl;
                sum += p;
            }
            // std::cerr << sum / (ns + 1) << std::endl;
            // for (auto [r, p] : rps[c]) {
            //     std::cerr << r << ", " << p << std::endl;
            // }
            // break;
            ap50_sum += sum / (ns + 1);
        }
        ap50 = ap50_sum / n;
    }
};