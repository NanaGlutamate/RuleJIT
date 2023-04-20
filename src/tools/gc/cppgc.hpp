/**
 * @file cppgc.hpp
 * @author djw
 * @brief
 * @date 2023-04-19
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-19</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

namespace rulejit::gc {

template <typename Ty> struct NullableGCPointer {
    NullableGCPointer(Ty &p) : p(&p) {}
    NullableGCPointer() : p(nullptr) {}

    Ty &operator*() { return *p; }
    const Ty &operator*() const { return *p; }
    template<typename Func1, typename Func2> auto ifNull(Func1&& f1, Func2&& f2) {
        if (p == nullptr) {
            return std::invoke(f1);
        } else {
            return std::invoke(f2, *p);
        }
    }


  private:
    Ty *p;
};

template <typename Ty> struct GCPointer {
    GCPointer(Ty &p) : p(p) {}

    Ty &operator*() { return p; }
    const Ty &operator*() const { return p; }
    operator NullableGCPointer<Ty>() { return NullableGCPointer<Ty>(p); }
    friend GCPointer assertNotNull(NullableGCPointer<Ty> np) { return GCPointer(*np); }

  private:
    Ty &p;
};

template <typename Ty> struct GCAble {
    bool inStack;
    static void* operator new(size_t count) {}
    GCPointer<Ty> operator&() {}
    ~GCAble() = default;
};

} // namespace rulejit::gc