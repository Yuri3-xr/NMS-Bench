#pragma once

#include <cinttypes>
#include <cstdint>
#include <iostream>

template <class T>
class Point {
   public:
    T x, y;
    constexpr Point(T _x = 0, T _y = 0) : x(_x), y(_y) {}
    constexpr Point operator+() const noexcept { return *this; }
    constexpr Point operator-() const noexcept { return Point(-x, -y); }
    constexpr Point operator+(const Point &p) const {
        return Point(x + p.x, y + p.y);
    }
    constexpr Point operator-(const Point &p) const {
        return Point(x - p.x, y - p.y);
    }
    constexpr Point &operator+=(const Point &p) {
        return x += p.x, y += p.y, *this;
    }
    constexpr Point &operator-=(const Point &p) {
        return x -= p.x, y -= p.y, *this;
    }
    constexpr T operator*(const Point &p) const { return x * p.x + y * p.y; }
    const Point operator/(T d) const { return Point(x / d, y / d); }
    constexpr Point &operator*=(const T &k) { return x *= k, y *= k, *this; }
    constexpr Point operator*(const T &k) { return Point(x * k, y * k); }
    constexpr bool operator==(const Point &r) const noexcept {
        return r.x == x and r.y == y;
    }
    constexpr bool operator!=(const Point &r) const noexcept {
        return !(*this == r);
    }
    friend std::ostream &operator<<(std::ostream &os, const Point &p) {
        return os << p.x << " " << p.y;
    }
    friend std::istream &operator>>(std::istream &is, Point &p) {
        is >> p.x >> p.y;
        return is;
    }
};

template <class T>
class Rect {
   public:
    Point<T> lt, rb;
    /*
        lt: left-top point
        rb: right-bottom point
    */
    Point<T> midPoint;

    constexpr Rect(Point<T> _lt, Point<T> _rb) : lt(_lt), rb(_rb) {
        midPoint = Point((lt + rb) / 2);
    }

    constexpr auto area() const noexcept -> T {
        auto width = std::max(static_cast<T>(0), rb.x - lt.x);
        auto height = std::max(static_cast<T>(0), lt.y - rb.y);

        return width * height;
    }
};

template <class T, class S>
class Box {
   public:
    Rect<T> rect;
    S score;
    uint32_t id;
    constexpr Box(Rect<T> _rect, S _score, uint32_t _id)
        : rect(_rect), score(_score), id(_id){};

    friend bool operator<(const auto &_cmpa, const auto &_cmpb) {
        return _cmpa.score > _cmpb.score;
    }

    auto IoU(const Box<T, S> &other) -> S {
        auto inter1 = Point(std::max(rect.lt.x, other.rect.lt.x),
                            std::min(rect.lt.y, other.rect.lt.y));
        auto inter2 = Point(std::min(rect.rb.x, other.rect.rb.x),
                            std::max(rect.rb.y, other.rect.rb.y));

        auto inter = Rect(inter1, inter2).area();
        auto area1 = this->rect.area();
        auto area2 = other.rect.area();

        return static_cast<S>(inter) / (area1 + area2 - inter);
    }
};
