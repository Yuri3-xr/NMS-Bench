#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "./BobNMS/BobNMS.hpp"
#include "./DNMS/DNMS.hpp"
#include "./FastNMS/FastNMS.hpp"
#include "./FasterNMS/CoverTree.hpp"
#include "./FasterNMS/FasterNMS.hpp"
#include "./GreedyNMS/GreedyNMS.hpp"
#include "./SoftNMS/SoftNMS.hpp"
#include "./Utils/time.hpp"
#include "./Utils/IO.hpp"
#include "./Utils/Data.hpp"
#include "./Utils/COCOMetrics.hpp"

// std::vector<Box<double, double, double>> boxes;
std::vector<std::uint32_t> keep;

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

    COCOMetrics<double> metrics;

    for (const auto& file : inList) {
        std::fstream inPredFile, inLabelFile, outFile;

        std::string inPredFileName =
            std::string(argv[1]) + std::string(file.path().filename());
        std::string inLabelFileName = 
            std::string(argv[2]) + std::string(file.path().filename());
        std::string outFileName =
            std::string(argv[3]) + std::string(file.path().filename());

        // std::cerr << inPredFileName << " " << inLabelFileName << " " << outFileName << std::endl;

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

        // for (auto box : boxes) {
        //     std::cout << data.img_id << " ";
        //     std::cout << box.rect.lt << " " << box.rect.rb << " " << box.score << std::endl;
        // }

        // break;

        timer.reset();

        if (method == "GreedyNMS") {
            keep = greedyNMS(boxes, iouThreshold);
        } else if (method == "FastNMS") {
            keep = fastNMS(boxes, iouThreshold);
        } else if (method == "FasterNMS") {
            keep = fasterNMS(boxes, iouThreshold);
        } else if (method == "DNMS") {
            keep = dNMS(boxes, iouThreshold);
        } else if (method == "SoftNMS") {
            keep = softNMS(boxes, iouThreshold, 0.5, 0.08, 1);
        } else if (method == "BobNMS") {
            keep = bobNMS(boxes, iouThreshold);
        } else {
            std::cerr << "No such method!" << std::endl;
            exit(-1);
        }

        sumTime += timer.elapsed();

        auto gt_category_id = data.label_category_id();
        for (auto cid : gt_category_id) {
            // std::cerr << cid << "GT" << std::endl;
            metrics.add_gt(cid);
        }

        for (uint32_t i = 0; i < std::min<uint32_t>(keep.size(), maxDets); i++) {
            auto [category_id, score, tf] = data.get_tf(keep[i], 0.5);
            // std::cerr << category_id << ", " << score << ": " << tf << std::endl;
            metrics.insert_tf(category_id, score, tf);
        }

        output(keep, outFile);

        inPredFile.close();
        inLabelFile.close();
        outFile.close();
    }

    std::cerr << method << " process time is " << sumTime << "ms" << std::endl;

    metrics.finalize();
    metrics.evaluate();
    std::cout << method << " mAP 50 is " << metrics.ap50 << std::endl;
    return 0;
}