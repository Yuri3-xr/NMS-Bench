#pragma once 

#include <map>
#include <vector>

template<class T>
class COCOMetrics {
  public:
    T ap;
    std::map<uint32_t, uint32_t> id;
    std::vector<uint32_t> gts;
    std::vector<std::map<T, T>> rps;
    std::vector<std::vector<std::pair<T, char>>> tfs;


    COCOMetrics() :
        ap(0), 
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
            std::sort(tfs[i].begin(), tfs[i].end());
            uint32_t tp = 0, fp = 0, gt = gts[i];
            if (gt <= 0) continue;
            for (auto [score, tf] : tfs[i]) {
                if (tf) {
                    tp += 1;
                } else {
                    fp += 1;
                }
                T p = (T)(tp) / (tp + fp);
                T r = (T)(tp) / gt ;
                insert_rp(i, r, p);
                // std::cerr << p << ", " << r << std::endl;
            }
        }
    }

    void evaluate() {
        uint32_t nc = rps.size();
        T ap_sum = 0;
        uint32_t n = 0;
        int cnt = 0;
        for (uint32_t c = 0; c < nc; c++) {
            if (!rps[c].size() || !gts[c]) continue;
            // std::cerr << c << ": " << gts[c] << std::endl;
            cnt += gts[c];
            n += 1;
            T sum = 0;
            // for (auto [nr, p] : rps[c]) {
            //     sum += p;
            // }
            uint32_t ns = 100;
            // std::vector<std::pair<T, T>> _rps;
            // for (auto [r, p] : rps[c]) {
            //     _rps.emplace_back(r, p);
            // }
            // std::sort(_rps.begin(), _rps.end());
            // T max_p = 0;
            // int j = (int)_rps.size() - 1;
            // for (int i = ns; i >= 0; i--) {
            //     T r = i * 1.0 / ns;
            //     while (j >= 0 && _rps[j].first >= r) {
            //         max_p = std::max(max_p, _rps[j].second);
            //         // std::cerr << _rps[j].first << std::endl;
            //         j--;
            //     }
            //     sum += max_p;
            // }
            T max_p = 0;
            // if (c == 0) {
            //     std::cout << "class: " << c << std::endl;
            //     for (auto [r, p] : rps[c]) {
            //         std::cout << r << ", " << p << std::endl;
            //     }
            // }
            // std::cout << std::endl;
            // std::cout << rps[c].size() << std::endl;
            // if (!rps[c].size()) {
            //     std::cerr << "cuo: " << c << std::endl;
            // }
            for (int i = ns; i >= 0; i--) {
                T r = (T)(i) / ns;
                if (rps[c].lower_bound(r) != rps[c].end()) {
                    auto [_r, p] = *rps[c].lower_bound(r);
                    max_p = std::max(max_p, p);
                }
                // auto [lr, lp] = *(--rps[c].upper_bound(r));
                // T p = (r * 2 > (rr + lr)) ? rp : lp;j
                // T p = std::max(rp, lp);
                // std::cerr << r << " " << p << std::endl;
                sum += max_p;
            }
            // std::cerr << sum / (ns + 1) << std::endl;
            // for (auto [r, p] : rps[c]) {
            //     std::cerr << r << ", " << p << std::endl;
            // }
            // break;
            ap_sum += sum;
        }
        // std::cerr << n << std::endl;
        // std::cerr << "cnt: " << cnt << std::endl;
        ap = ap_sum / (1.0 * n * 101);
    }

    T get_ap() { return ap; }
};