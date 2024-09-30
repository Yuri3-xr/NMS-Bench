#pragma once

#include <cinttypes>
#include <cstdint>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>

#include "./NMS.hpp"
#include "./utils.hpp"

template <class T, class M, class S>
class BBox {
   public:
    uint32_t category_id;
    Box<T, M, S> box;
    BBox(uint32_t _category_id, Box<T, M, S> _box)
        : category_id(_category_id), box(_box) {}
    ~BBox() {}

    bool operator<(const BBox &b) const {
        if (box < b.box) return true;
        if (b.box < box) return false;
        return category_id < b.category_id;
    }
};

template <class T, class M, class S>
class Data {
   public:
    // std::string img_id;
    std::vector<BBox<T, M, S>> preds;
    std::vector<BBox<T, M, S>> labels;
    std::set<uint32_t> vis_labels;
    Data()
        : preds(std::vector<BBox<T, M, S>>()),
          labels(std::vector<BBox<T, M, S>>()),
          vis_labels(std::set<uint32_t>()) {}
    ~Data() {}

    // void set_img_id(std::string _img_id) {
    //     img_id = _img_id;
    // }

    void add_pred(BBox<T, M, S> pred) { preds.emplace_back(pred); }

    void add_label(BBox<T, M, S> label) { labels.emplace_back(label); }

    void input(std::fstream &inPredFile, std::fstream &inLabelFile) {
        std::string _img_id;
        uint32_t _categoriy_id;
        T ltx, lty, rbx, rby, w, h;
        S score;
        int iscrowd;

        uint32_t p = 0;
        std::string buf;
        std::stringstream ss;

        inPredFile >> buf;  // ignore header
        while (inPredFile >> buf) {
            string_split(ss, buf, ',');
            ss >> _img_id >> _categoriy_id >> ltx >> lty >> w >> h >> score;
            rbx = ltx + w, rby = lty + h;
            auto box = Box<T, M, S>(
                Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)), score, p);
            preds.emplace_back(_categoriy_id, box);
            p++;
            ss.clear();
        }

        inLabelFile >> buf;  // ignore header
        p = 0;
        while (inLabelFile >> buf) {
            string_split(ss, buf, ',');
            ss >> _img_id >> _categoriy_id >> ltx >> lty >> w >> h >> iscrowd;
            rbx = ltx + w, rby = lty + h;
            auto box =
                Box<T, M, S>(Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)),
                             (T)(1 - iscrowd), p);
            labels.emplace_back(_categoriy_id, box);
            p++;
            ss.clear();
        }
        std::sort(labels.begin(), labels.end());
    }

    std::vector<Box<T, M, S>> pred_boxes(bool batched_nms = true,
                                         uint32_t max_wh = 7680) {
        std::vector<Box<T, M, S>> boxes;
        for (auto &pred : preds) {
            auto box = pred.box;
            auto category_id = pred.category_id;
            uint32_t offset = (batched_nms == true) * max_wh * category_id;
            box.rect.lt.x += offset, box.rect.lt.y += offset;
            box.rect.rb.x += offset, box.rect.rb.y += offset;
            box.rect.midPoint.x += offset, box.rect.midPoint.y += offset;
            boxes.emplace_back(box);
        }
        return boxes;
    }

    std::vector<uint32_t> pred_categories() {
        std::vector<uint32_t> categories;
        for (auto &pred : preds) {
            auto category_id = pred.category_id;
            categories.emplace_back(category_id);
        }
        return categories;
    }

    std::vector<int32_t> label_category_id() {
        std::vector<int32_t> res;
        for (auto &label : labels) {
            auto category_id = label.category_id;
            if (label.box.score < 0.5) {
                res.emplace_back(-1);
            } else {
                res.emplace_back(category_id);
            }
        }
        return res;
    }

    std::tuple<uint32_t, S, int> get_tf(uint32_t pred_id, T iou_threshold) {
        auto pred_bbox = preds[pred_id];
        uint32_t pred_category_id = pred_bbox.category_id;
        auto pred_box = pred_bbox.box;
        int m = -1;
        T iou = iou_threshold;
        for (uint32_t i = 0; i < labels.size(); i++) {
            auto &label = labels[i];
            uint32_t category_id = label.category_id;
            if (category_id != pred_category_id) continue;
            auto box = label.box;
            if (vis_labels.count(i) && box.score > 0.5) continue;
            T now_iou = pred_box.IoU(box);
            if (box.score < 0.5) {
                // if crowd, iou = i / da instead of i / u.
                T da = pred_bbox.box.rect.area();
                T ga = box.rect.area();
                now_iou *= (da + ga - now_iou) / da;
            }
            if (m != -1 && labels[m].box.score > 0.5 && box.score < 0.5)
                break;  // if matches and not crowd while get a crowd label
            if (now_iou < iou) continue;
            iou = now_iou;  // find the best iou
            m = i;
        }
        if (m == -1) {
            return {pred_category_id, pred_box.score, 0};
        } else {
            vis_labels.insert(m);
            if (labels[m].box.score < 0.5) {
                // iscrowd, ignore
                return {pred_category_id, pred_box.score, -1};
            } else {
                return {pred_category_id, pred_box.score, 1};
            }
        }
    }

    void reset_vis_labels() { vis_labels.clear(); }
};
