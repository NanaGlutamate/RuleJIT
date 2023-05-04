#include <string>

#include "tools/myfunction.hpp"

std::string adds(std::string x, int y, int z) { return x + std::to_string(y); }

int add(int x, int y) { return x + y; }

auto help(std::string x) {
    using namespace tools::myfunc;
    return curry_value(adds)(x);
}

int main() {
    using namespace tools::myfunc;

    // auto f = curry([](int x, int y){return x+y;});
    std::string s = "12345";

    auto f1 = curry_ref(add);
    auto add1 = f1(1);
    auto result1 = add1(4);

    auto f2 = curry_ref(adds);
    auto add2 = f2(std::string("123"));
    auto add3 = add2(4);
    auto result2 = add3(4);

    auto f3 = help("114514");
    auto result3 = f3(4)(1);

    // auto addr = f(result);
    // auto r2 = addr(4);
}