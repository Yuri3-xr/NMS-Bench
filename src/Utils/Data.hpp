#pragma once

#include <cinttypes>
#include <fstream>
#include <cstdint>
#include <vector>
#include <set>

#include "./NMS.hpp"

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
        if (box > b.box) return false;
        return category_id < b.category_id;
    }
};

template <class T, class M, class S>
class Data {
  public:
    uint32_t img_id;
    std::vector<BBox<T, M, S>> preds;
    std::vector<BBox<T, M, S>> labels;
    std::set<uint32_t> vis_labels;
    Data(uint32_t _img_id = 0) 
        : 
        img_id(_img_id), 
        preds(std::vector<BBox<T, M, S>>()), 
        labels(std::vector<BBox<T, M, S>>()), 
        vis_labels(std::set<uint32_t>()) 
        {}
    ~Data() {}

    void set_img_id(uint32_t _img_id) {
        img_id = _img_id;
    }

    void add_pred(BBox<T, M, S> pred) {
        preds.emplace_back(pred);
    }

    void add_label(BBox<T, M, S> label) {
        labels.emplace_back(label);
    }

    void input(std::fstream& inPredFile, std::fstream& inLabelFile) {
        uint32_t _img_id, _categoriy_id;
        T ltx, lty, rbx, rby, w, h;
        S score;
        uint32_t p = 0;
        std::string header;
        char ch;
        inPredFile >> header;
        while (inPredFile >> _img_id >> ch 
                >> _categoriy_id >> ch 
                >> ltx >> ch >> lty >> ch >> w >> ch >> h >> ch >> score) {
            rbx = ltx + w, rby = lty + h;
            auto box = Box<T, M, S>(Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)),
                           score, p);
            preds.emplace_back(_categoriy_id, box);
            set_img_id(_img_id);
            p++;
        }
        inLabelFile >> header;
        p = 0;
        while (inLabelFile >> _img_id >> ch 
                >> _categoriy_id >> ch 
                >> ltx >> ch >> lty >> ch >> w >> ch >> h) {
            rbx = ltx + w, rby = lty + h;
            // std::cerr << ltx << ", " << lty << std::endl;
            auto box = Box<T, M, S>(Rect<T, M>(Point<T>(ltx, lty), Point<T>(rbx, rby)),
                           0, p);
            labels.emplace_back(_categoriy_id, box);
            set_img_id(_img_id);
            p++;
        }
    }

    std::vector<Box<T, M, S>> pred_boxes(bool batched_nms = true, uint32_t max_wh = 7680) {
        std::vector<Box<T, M, S>> boxes;
        for (auto pred : preds) {
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

    std::vector<uint32_t> label_category_id() {
        std::vector<uint32_t> res;
        for (auto label : labels) {
            auto category_id = label.category_id;
            res.emplace_back(category_id);
        }
        return res;
    }

    std::tuple<uint32_t, S, bool> get_tf(uint32_t pred_id, T iou_threshold) {
        auto pred_bbox = preds[pred_id];
        uint32_t pred_category_id = pred_bbox.category_id;
        auto pred_box = pred_bbox.box;
        for (uint32_t i = 0; i < labels.size(); i++) {
            auto label = labels[i];
            uint32_t category_id = label.category_id;
            auto box = label.box;
            if (vis_labels.count(i)) continue;
            if (category_id == pred_category_id && pred_box.IoU(box) >= iou_threshold) {
                vis_labels.insert(i);
                return {pred_category_id, pred_box.score, true};
            }
        }
        return {pred_category_id, pred_box.score, false};
    }   
};
