#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "./BOENMS/BOENMS.hpp"
#include "./ClusterNMS/ClusterNMS.hpp"
#include "./FastNMS/FastNMS.hpp"
#include "./FastNMS/FastNMS_Par.hpp"
#include "./OrignalNMS/OrignalNMS.hpp"
#include "./QSINMS/QSINMS.hpp"
#include "./QSINMS/eQSINMS.hpp"
#include "./SoftNMS/SoftNMS.hpp"
#include "./PSRR-MaxpoolNMS/PSRR-MaxpoolNMS.hpp"
#include "./Utils/COCOMetrics.hpp"
#include "./Utils/Data.hpp"
#include "./Utils/utils.hpp"

// std::vector<Box<double, double, double>> boxes;
std::vector<uint32_t> keep;

constexpr double iouThreshold = 0.7;
constexpr uint32_t maxDets = 100;
int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    /*
        argc = 4
        argv[1] -- input pred path
        argv[2] -- input label path
        argv[3] -- output path
        argv[4] -- method
    */

    if (argc != 5) {
        std::cerr << "  argv[1] -- input dict \nargv[2] -- output dict\n "
                     "argv[3] -- method(GreedyNMS, FastNMS, FasterNMS, DNMS)"
                  << std::endl;

        exit(-1);
    }

    std::filesystem::path inPredPath(argv[1]);
    std::filesystem::path inLabelPath(argv[2]);
    std::filesystem::path outPath(argv[3]);
    std::string method(argv[4]);

    if (not std::filesystem::exists(inPredPath)) {
        std::cerr << "No such input pred dictionary!" << std::endl;

        exit(-1);
    }

    if (not std::filesystem::exists(inLabelPath)) {
        std::cerr << "No such input label dictionary!" << std::endl;

        exit(-1);
    }

    if (not std::filesystem::exists(outPath)) {
        std::filesystem::create_directory(outPath);
    }

    Timer timer;
    timer.reset();

    std::uint64_t sumTime = 0;
    std::filesystem::directory_iterator inList(inPredPath);

    int cnt = 0;
    int total_images = 0;
    int total_boxes = 0;

    std::vector<COCOMetrics<double>> coco_metrics(10);

    for (const auto& file : inList) {
        std::fstream inPredFile, inLabelFile, outFile;

        std::string inPredFileName =
            std::string(argv[1]) + std::string(file.path().filename());
        std::string inLabelFileName =
            std::string(argv[2]) + std::string(file.path().filename());
        std::string outFileName =
            std::string(argv[3]) + std::string(file.path().filename());

        // std::cerr << inPredFileName << " " << inLabelFileName << " " <<
        // outFileName << std::endl;

        inPredFile.open(inPredFileName,
                        std::ios::in | std::ios::out | std::ios::binary);
        inLabelFile.open(inLabelFileName,
                         std::ios::in | std::ios::out | std::ios::binary);
        outFile.open(outFileName, std::ios::in | std::ios::out |
                                      std::ios::binary | std::ios::trunc);

        // std::vector<Box<double, double, double>> boxes;

        // boxes.clear();
        // input<double, double, double>(boxes, inPredFile);

        Data<double, double, double> data;
        data.input(inPredFile, inLabelFile);

        auto boxes = data.pred_boxes();
        auto _boxes = data.pred_boxes(false);
        auto categories = data.pred_categories();

        total_boxes += boxes.size();
        total_images += 1;
        timer.reset();

        if (method == "OrignalNMS") {
            keep = orignalNMS(boxes, iouThreshold);
        } else if (method == "FastNMS") {
            keep = fastNMS(boxes, iouThreshold);
        } else if (method == "FastNMS_Par") {
            keep = fastNMS_Par(boxes, iouThreshold);
        } else if (method == "SoftNMS") {
            keep = softNMS(boxes, iouThreshold, 0.5, 0.08, 1);
        } else if (method == "BOENMS") {
            keep = boeNMS(boxes, iouThreshold);
        } else if (method == "QSINMS") {
            keep = qsiNMS(boxes, iouThreshold);
        } else if (method == "eQSINMS") {
            keep = eqsiNMS(boxes, iouThreshold);
        } else if (method == "ClusterNMS") {
            keep = clusterNMS(boxes, iouThreshold);
        } else if (method == "PSRR-MaxpoolNMS") {
            std::vector<int> anchors = {64 * 64, 128 * 128, 256 * 256, 512 * 512};
            std::vector<double> ratios = {0.5, 1, 2};
            keep = PSRRNMS(_boxes, categories, anchors, ratios);
        } else {
            std::cerr << "No such method!" << std::endl;
            exit(-1);
        }

        sumTime += timer.elapsed();

        auto gt_category_id = data.label_category_id();
        for (uint32_t i = 0; i < 10; i++) {
            for (auto cid : gt_category_id) {
                // std::cerr << cid << "GT" << std::endl;
                if (cid != -1) {
                    coco_metrics[i].add_gt(cid);
                }
            }
        }
        // std::cerr << "num: " << gt_category_id.size() << std::endl;

        std::sort(keep.begin(), keep.end(),
                  [&](auto x, auto y) { return boxes[x] < boxes[y]; });

        // int cnt = 0;
        for (uint32_t i = 0; i < 10; i++) {
            data.reset_vis_labels();
            for (uint32_t j = 0; j < std::min<uint32_t>(keep.size(), maxDets);
                 j++) {
                auto [category_id, score, tf] =
                    data.get_tf(keep[j], 0.5 + 0.05 * i);
                // std::cerr << category_id << ", " << score << ": " << tf <<
                // std::endl;
                if (tf == 1) {
                    // std::cerr << i << ": " << category_id << ", " << score <<
                    // std::endl;
                    cnt += 1;
                }
                if (tf >= 0) {
                    coco_metrics[i].insert_tf(category_id, score, tf);
                }
            }
        }

        // std::cerr << "#tf: " << cnt << std::endl;

        output(keep, outFile);

        inPredFile.close();
        inLabelFile.close();
        outFile.close();
    }
  
    std::cout << method << " average latency is " << std::fixed
              << std::setprecision(3) << (double)(sumTime) / total_images
              << " microseconds" << std::endl;

    double ap50 = 0, ap75 = 0, ap5095 = 0;
    for (uint32_t i = 0; i < 10; i++) {
        coco_metrics[i].finalize();
        coco_metrics[i].evaluate();
        ap5095 += coco_metrics[i].get_ap();
    }
    // coco_metrics[0].finalize();
    // coco_metrics[0].evaluate();
    ap50 = coco_metrics[0].get_ap();
    ap75 = coco_metrics[5].get_ap();
    std::cout << std::fixed;
    std::cout << std::left << std::setw(12) << "mAP 50:95 ";
    std::cout << std::setprecision(3) << " = " << ap5095 / 10 << std::endl;
    std::cout << std::left << std::setw(12) << "mAP 50 ";
    std::cout << std::setprecision(3) << " = " << ap50 << std::endl;
    std::cout << std::left << std::setw(12) << "mAP 75 ";
    std::cout << std::setprecision(3) << " = " << ap75 << std::endl;
    // std::cout << std::setprecision(3) << "average number of boxes: " << total_boxes << "/" << total_images << " = " << 1.0 * total_boxes / total_images << std::endl;

    // std::cerr << cnt << std::endl;
    return 0;
}
