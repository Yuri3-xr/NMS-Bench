#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "../FastNMS/FastNMS.hpp"
#include "../FasterNMS/CoverTree.hpp"
#include "../FasterNMS/FasterNMS.hpp"
#include "../GreedyNMS/GreedyNMS.hpp"
#include "../Utils/time.hpp"
#include "IO.hpp"

std::vector<Box<double, double, double>> boxes;
std::vector<std::uint32_t> keep;

constexpr double iouThreshold = 0.5;
int main(int argc, char** argv) {
    /*
        argc = 4
        argv[1] -- input path
        argv[2] -- output path
        argv[3] -- method
    */

    if (argc != 4) {
        std::cerr << "  argv[1] -- input dict \nargv[2] -- output dict\n "
                     "argv[3] -- method(GreedyNMS, FastNMS, FasterNMS)"
                  << std::endl;

        exit(-1);
    }

    std::filesystem::path inPath(argv[1]);
    std::filesystem::path outPath(argv[2]);
    std::string method(argv[3]);

    if (not std::filesystem::exists(inPath)) {
        std::cerr << "No such input dict!" << std::endl;

        exit(-1);
    }

    if (not std::filesystem::exists(outPath)) {
        std::filesystem::create_directory(outPath);
    }

    Timer timer;
    timer.reset();
    std::filesystem::directory_iterator inList(inPath);

    for (const auto& file : inList) {
        std::fstream inFile, outFile;

        std::string inFileName =
            std::string(argv[1]) + std::string(file.path().filename());
        std::string outFileName =
            std::string(argv[2]) + std::string(file.path().filename());

        std::cerr << inFileName << " " << outFileName << std::endl;

        inFile.open(inFileName,
                    std::ios::in | std::ios::out | std::ios::binary);
        outFile.open(outFileName, std::ios::in | std::ios::out |
                                      std::ios::binary | std::ios::trunc);

        boxes.clear();
        input<double, double, double>(boxes, inFile);

        if (method == "GreedyNMS") {
            keep = greedyNMS(boxes, iouThreshold);
        } else {
            if (method == "FastNMS") {
                keep = fastNMS(boxes, iouThreshold);
            } else if (method == "FasterNMS") {
                keep = fasterNMS(boxes, iouThreshold);
            } else {
                std::cerr << "No such method!" << std::endl;
                exit(-1);
            }
        }

        output(keep, outFile);

        inFile.close();
        outFile.close();
    }

    std::cerr << std::format("Time is {}\n", timer.elapsed());
    return 0;
}