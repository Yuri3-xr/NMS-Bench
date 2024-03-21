#pragma once

#include <cinttypes>
#include <fstream>
#include <cstdint>
#include <vector>

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
    Data(uint32_t _img_id = 0) 
        : img_id(_img_id), preds(std::vector<BBox<T, M, S>>()), labels(std::vector<BBox<T, M, S>>()) {}
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
        while (inLabelFile >> _img_id >> ch 
                >> _categoriy_id >> ch 
                >> ltx >> ch >> lty >> ch >> w >> ch >> h) {
            rbx = ltx + w, rby = lty + h;
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
};
