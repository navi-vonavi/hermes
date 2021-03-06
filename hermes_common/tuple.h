// Copyright (c) 2009 hp-FEM group at the University of Nevada, Reno (UNR).
// Distributed under the terms of the BSD license (see the LICENSE
// file for the exact terms).
// Email: hermes1d@googlegroups.com, home page: http://hpfem.org/

#ifndef __HERMES_COMMON_TUPLE_H
#define __HERMES_COMMON_TUPLE_H

/// A vector of values.
/** This class is used to pass a variable number of parameters in a type-safe fashion.
 *  \par Suggested Use
 *  Let us assume a function foo(const Tuple<Solution*>&) and instances sln1-sln3 of a class Solution. Then,
 *  - 1 parameter: foo(&sln1);
 *  - 2 up to 10 parameters: foo(Tuple<double>(&sln1, &sln2, &sln3));
 *  - more than 10 parameters: Fill the instance similar to STL vector (std::vector). */
template<typename T>
class Tuple: public std::vector<T> {
public:
  /// A default constructor. Creates an empty vector.
  explicit Tuple() {};
  /// 1 parameter constructor.
  Tuple(const T& a) { this->push_back(a); };
  /// 2 parameters constructor.
  Tuple(const T& a, const T& b) { std::vector<T>::reserve(2); this->push_back(a); this->push_back(b); };
  /// 3 parameters constructor.
  Tuple(const T& a, const T& b, const T& c) { std::vector<T>::reserve(3); this->push_back(a); this->push_back(b); this->push_back(c); };
  /// 4 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d) { std::vector<T>::reserve(4); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); };
  /// 5 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d, const T& e) { std::vector<T>::reserve(5); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); this->push_back(e); };
  /// 6 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d, const T& e, const T& f) { std::vector<T>::reserve(6); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); this->push_back(e); this->push_back(f); };
  /// 7 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d, const T& e, const T& f, const T& g) { std::vector<T>::reserve(7); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); this->push_back(e); this->push_back(f); this->push_back(g); };
  /// 8 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d, const T& e, const T& f, const T& g, const T& h) { std::vector<T>::reserve(8); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); this->push_back(e); this->push_back(f); this->push_back(g); this->push_back(h); };
  /// 9 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d, const T& e, const T& f, const T& g, const T& h, const T& i) { std::vector<T>::reserve(9); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); this->push_back(e); this->push_back(f); this->push_back(g); this->push_back(h); this->push_back(i); };
  /// 10 parameters constructor.
  Tuple(const T& a, const T& b, const T& c, const T& d, const T& e, const T& f, const T& g, const T& h, const T& i, const T& j) { std::vector<T>::reserve(10); this->push_back(a); this->push_back(b); this->push_back(c); this->push_back(d); this->push_back(e); this->push_back(f); this->push_back(g); this->push_back(h); this->push_back(i); this->push_back(j); };
};

#endif
