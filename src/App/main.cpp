#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <vector>

#include "../FastNMS/FastNMS.hpp"
#include "../FasterNMS/CoverTree.hpp"
#include "../GreedyNMS/GreedyNMS.hpp"
int main(int argc, char** argv) {
    Box<int, double, double> a({{0, 1}, {1, 0}}, 0.7, 0);
    Box<int, double, double> b({{0, 2}, {2, 0}}, 0.9, 1);

    std::cout << a.IoU(b) << std::endl;
    std::cout << a.distance(b) << std::endl;

    CoverTree<Box<int, double, double>> cTree;
    cTree.insert(a);
    cTree.insert(b);
    auto p = cTree.kNearestNeighbors(a, 1);

    // for (auto t : p) {
    //     std::cerr << t.rect.midPoint.x << " " << t.rect.midPoint.y <<
    //     std::endl;
    // }

    auto keep = fastNMS<int, double, double>({a, b}, 0.1);

    for (auto p : keep) {
        std::cerr << p << std::endl;
    }
    return 0;
}