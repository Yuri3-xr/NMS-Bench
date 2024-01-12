#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <vector>

#include "../FastNMS/FastNMS.hpp"
#include "../GreedyNMS/GreedyNMS.hpp"

int main(int argc, char** argv) {
    Box<int, double> a({{0, 2}, {2, 0}}, 0.7, 0);
    Box<int, double> b({{0, 1}, {1, 0}}, 0.7, 1);

    std::cout << a.IoU(b) << std::endl;
    return 0;
}