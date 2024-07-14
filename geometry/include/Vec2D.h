#pragma once

#include <algorithm>

namespace geometry {
  
  template<typename T>
  class Vec2D {
    public:
    T x;
    T y;

    Vec2D<T>();
    Vec2D<T>(const T &x, const T &y);
    Vec2D<T>(const Vec2D<T> &other);

    Vec2D<T> operator+(const Vec2D<T> &rhs);
    Vec2D<T> operator-(const Vec2D<T> &rhs);

    Vec2D<T> &operator+=(const Vec2D<T> &rhs);
    Vec2D<T> &operator-=(const Vec2D<T> &rhs);
  };

  template<typename T>
  Vec2D<T>::Vec2D<T>(){
    x = 0.f;
    y = 0.f;
  }

  template<typename T>
  Vec2D<T>::Vec2D<T>(const T &a, const T &b){
    x = a;
    y = b;
  }

  template<typename T>
  Vec2D<T>::Vec2D<T>(const Vec2D<T> &other){
    x = other.x;
    y = other.y;
  }

  template<typename T>
  Vec2D<T> Vec2D<T>::operator+(const Vec2D<T> &rhs){
    Vec2D<T> result;
    result.x = x + rhs.x;
    result.y = y + rhs.y;
    return result;
  }

  template<typename T>
  Vec2D<T> Vec2D<T>::operator-(const Vec2D<T> &rhs){
    Vec2D<T> result;
    result.x = x - rhs.x;
    result.y = y - rhs.y;
    return result;
  }

  template<typename T>
  Vec2D<T> &Vec2D<T>::operator+=(const Vec2D<T> &rhs){
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  template<typename T>
  Vec2D<T> &Vec2D<T>::operator-=(const Vec2D<T> &rhs){
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  template<typename T>
  Vec2D<T> min(const Vec2D<T> &lhs, const Vec2D<T> &rhs){
    Vec2D<T> result;
    result.x = std::min(lhs.x, rhs.x);
    result.y = std::min(lhs.y, rhs.y);
    return result;
  }

  template<typename T>
  Vec2D<T> max(const Vec2D<T> &lhs, const Vec2D<T> &rhs){
    Vec2D<T> result;
    result.x = std::max(lhs.x, rhs.x);
    result.y = std::max(lhs.y, rhs.y);
    return result;
  }

  using ftype = float;
}