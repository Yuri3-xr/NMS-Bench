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
#include "../util/time.hpp"
#include "IO.hpp"

std::vector<Box<int32_t, double, double>> boxes;
std::vector<std::uint32_t> keep;
constexpr double iouThreshold = 0.5;
int main(int argc, char** argv) {
    /*
        argc = 4
        argv[1] -- input file
        argv[2] -- output file
        argv[3] -- method
    */

    if (argc != 4) {
        std::cerr << "  argv[1] -- input file\nargv[2] -- output file\n "
                     "argv[3] -- method(GreedyNMS, FastNMS, FasterNMS)"
                  << std::endl;

        exit(-1);
    }

    std::fstream inFile, outFile;
    inFile.open(argv[1], std::ios::in | std::ios::out | std::ios::binary);
    outFile.open(argv[2], std::ios::in | std::ios::out | std::ios::binary |
                              std::ios::trunc);
    std::string method(argv[3]);

    input<int32_t, double, double>(boxes, inFile);

    Timer timer;
    timer.reset();
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

    std::cerr << std::format("Time is {}\n", timer.elapsed());
    output(keep, outFile);

    inFile.close();
    outFile.close();
    return 0;
}