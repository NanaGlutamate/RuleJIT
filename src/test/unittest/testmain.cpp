#include "gtest/gtest.h"

TEST(TestStr, test_str_join){
    auto f =[](){return [](auto x){return x;};};
    auto a = f()(1);
    auto b = f()(1.);
}