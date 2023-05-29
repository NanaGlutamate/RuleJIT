#pragma once

#include <cmath>

#include "typedef.hpp"

extern "C" {
using namespace ruleset;

}

namespace ruleset{

template <typename V>
size_t length(V&& v){ return v.size(); }

template <typename V, typename Op>
void push(V& v, Op&& op){ v.push_back(std::forward<Op>(op)); }

template <typename V>
void resize(V& v, size_t size){ v.resize(size); }

bool strEqual(const string& lhs, const string& rhs){ return lhs == rhs; }

inline typedReal<f64> _func_buildIn645264trimffunc40f54524432f54524432f54524432f5452414562f5452(const f64& x, const f64& a, const f64& b, const f64& c);

inline typedReal<f64> _func_buildIn6448646161func40string4432string414562f5452(const string& a, const string& b);

inline typedReal<f64> _func_buildIn644964absfunc40f5452414562f5452(const f64& a);

inline typedReal<f64> _func_buildIn645064minfunc40f54524432f5452414562f5452(const f64& a, const f64& b);

inline typedReal<f64> _func_buildIn645364trapmffunc40f54524432f54524432f54524432f54524432f5452414562f5452(const f64& x, const f64& a, const f64& b, const f64& c, const f64& d);

inline typedReal<f64> _func_buildIn645164maxfunc40f54524432f5452414562f5452(const f64& a, const f64& b);


inline typedReal<f64> _func_buildIn645264trimffunc40f54524432f54524432f54524432f5452414562f5452(const f64& x, const f64& a, const f64& b, const f64& c) {
    return((x < a) ? (typedReal<f64>)((0)) : (typedReal<f64>)(((x < b) ? (typedReal<f64>)(x - a / b - a) : (typedReal<f64>)(((x < c) ? (typedReal<f64>)(c - x / c - b) : (typedReal<f64>)((0)))))));
}

inline typedReal<f64> _func_buildIn6448646161func40string4432string414562f5452(const string& a, const string& b) {
    return(strEqual(a, b));
}

inline typedReal<f64> _func_buildIn644964absfunc40f5452414562f5452(const f64& a) {
    return(fabs(a));
}

inline typedReal<f64> _func_buildIn645064minfunc40f54524432f5452414562f5452(const f64& a, const f64& b) {
    return((a > b) ? (typedReal<f64>)(b) : (typedReal<f64>)(a));
}

inline typedReal<f64> _func_buildIn645364trapmffunc40f54524432f54524432f54524432f54524432f5452414562f5452(const f64& x, const f64& a, const f64& b, const f64& c, const f64& d) {
    return((x < a) ? (typedReal<f64>)((0)) : (typedReal<f64>)(((x < b) ? (typedReal<f64>)(x - a / b - a) : (typedReal<f64>)(((x < c) ? (typedReal<f64>)((1)) : (typedReal<f64>)(((x < d) ? (typedReal<f64>)(d - x / d - c) : (typedReal<f64>)((0)))))))));
}

inline typedReal<f64> _func_buildIn645164maxfunc40f54524432f5452414562f5452(const f64& a, const f64& b) {
    return((a < b) ? (typedReal<f64>)(b) : (typedReal<f64>)(a));
}


}

