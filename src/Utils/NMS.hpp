#pragma once

#include <cinttypes>
#include <cstdint>
#include <iostream>
#include <tuple>

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
    constexpr bool operator<(const Point &r) const {
        return std::tie(x, y) < std::tie(r.x, r.y);
    }
    constexpr bool operator>(const Point &r) const {
        return std::tie(x, y) > std::tie(r.x, r.y);
    }
    friend std::ostream &operator<<(std::ostream &os, const Point &p) {
        return os << p.x << " " << p.y;
    }
    friend std::istream &operator>>(std::istream &is, Point &p) {
        is >> p.x >> p.y;
        return is;
    }
};

template <class T, class M>
class Rect {
   public:
    Rect() : lt(0, 0), rb(0, 0), midPoint(0, 0){};
    Point<T> lt, rb;
    /*
        lt: left-top point
        rb: right-bottom point
        Pay attention to the orignal point
                   rb
        |----------|
        |          |
        |          |
        |          |
        |----------|
        lt
    */
    Point<M> midPoint;

    constexpr Rect(Point<T> _lt, Point<T> _rb)
        : lt(_lt),
          rb(_rb),
          midPoint(M(lt.x + rb.x) / M(2), M(lt.y + rb.y) / M(2)){};

    Rect<T, M> operator=(const Rect<T, M> &other) {
        this->lt = other.lt;
        this->rb = other.rb;
        this->midPoint = other.midPoint;

        return (*this);
    }
    constexpr auto area() const noexcept -> T {
        auto width = std::max(static_cast<T>(0), rb.x - lt.x);
        auto height = std::max(static_cast<T>(0), rb.y - lt.y);

        return width * height;
    }
};

/*
    T: lt/ rb point type
    M: midPoint type
    S: score type
*/

template <class T, class M, class S>
class Box {
   public:
    Box() { score = 0; }
    Rect<T, M> rect;
    S score;
    uint32_t id = -1;
    constexpr Box(Rect<T, M> _rect, S _score, uint32_t _id)
        : rect(_rect), score(_score), id(_id){};

    friend bool operator<(const Box<T, M, S> &_cmpa,
                          const Box<T, M, S> &_cmpb) {
        return _cmpa.score > _cmpb.score;
    }
    bool operator==(const Box &p) const { return id == p.id; }
    Box<T, M, S> operator=(const Box<T, M, S> &other) {
        this->rect = other.rect;
        this->score = other.score;
        this->id = other.id;

        return (*this);
    }
    auto IoU(const Box<T, M, S> &other) -> S {
        auto inter1 = Point(std::max(rect.lt.x, other.rect.lt.x),
                            std::max(rect.lt.y, other.rect.lt.y));
        auto inter2 = Point(std::min(rect.rb.x, other.rect.rb.x),
                            std::min(rect.rb.y, other.rect.rb.y));

        auto inter = Rect<T, M>(inter1, inter2).area();
        auto area1 = this->rect.area();
        auto area2 = other.rect.area();

        return static_cast<S>(inter) / (area1 + area2 - inter);
    }

    T distance(const Box<T, M, S> &other) const {
        auto p = (this->rect).midPoint - other.rect.midPoint;
        return static_cast<M>(sqrt(p.x * p.x + p.y * p.y));
    }

    const Point<M> getMidPoint() const { return rect.midPoint; }
};
