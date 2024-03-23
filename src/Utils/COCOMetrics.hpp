#pragma once 

#include <map>
#include <vector>

template<class T>
class COCOMetrics {
  public:
    T ap;
    std::map<uint32_t, uint32_t> id;
    std::vector<uint32_t> gts;
    std::vector<std::vector<std::pair<T, T>>> rps;
    std::vector<std::vector<std::pair<T, char>>> tfs;


    COCOMetrics() :
        ap(0), 
        id(std::map<uint32_t, uint32_t>()),
        gts(std::vector<uint32_t>()),
        rps(std::vector<std::vector<std::pair<T, T>>>()), 
        tfs(std::vector<std::vector<std::pair<T, char>>>()) {}
    ~COCOMetrics() {}

    uint32_t get_id(uint32_t category_id) {
        if (!id.count(category_id)) {
            id[category_id] = id.size();
            gts.emplace_back(0);
            rps.emplace_back(std::vector<std::pair<T, T>>());
            tfs.emplace_back(std::vector<std::pair<T, char>>());
        }
        return id[category_id];
    }

    void add_gt(uint32_t category_id, uint32_t x = 1) {
        uint32_t i = get_id(category_id);
        gts[i] += x;
    }

    void insert_tf(uint32_t category_id, T score, bool tf) {
        uint32_t i = get_id(category_id);
        tfs[i].emplace_back(-score, tf);
    }

    void insert_rp(uint32_t i, T r, T p) {
        rps[i].emplace_back(r, p);
    }

    void finalize() {
        uint32_t nc = id.size();
        for (uint32_t i = 0; i < nc; i++) {
            std::sort(tfs[i].begin(), tfs[i].end());
            uint32_t tp = 0, fp = 0, gt = gts[i];
            if (gt <= 0) continue;
            for (auto [score, tf] : tfs[i]) {
                if (tf) {
                    tp += 1;
                    T r = (T)(tp) / gt;
                    T p = (T)(tp) / (tp + fp);
                    insert_rp(i, r, p);
                } else {
                    fp += 1;
                }
            }
        }
    }

    void evaluate() {
        uint32_t nc = rps.size();
        T ap_sum = 0;
        uint32_t n = 0;
        int cnt = 0;
        for (uint32_t c = 0; c < nc; c++) {
            if (gts[c] <= 0) continue;
            cnt += gts[c];
            n += 1;
            T sum = 0;
            uint32_t ns = 100;
            T max_p = 0;
            int j = (int)rps[c].size() - 1;
            for (int i = ns; i >= 0; i--) {
                T r = (T)(i) / ns;
                while (j >= 0 && rps[c][j].first >= r) {
                    max_p = std::max(max_p, rps[c][j].second);
                    j--;
                }
                sum += max_p;
            }
            ap_sum += sum / (ns + 1);
        }
        // std::cerr << n << std::endl;
        // std::cerr << "cnt: " << cnt << std::endl;
        ap = ap_sum / n;
    }

    T get_ap() { return ap; }
};