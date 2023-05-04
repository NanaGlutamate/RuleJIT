#include <any>
#include <array>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_map>

#include "gtest/gtest.h"

#include "tools/anyprocess.hpp"
#include "tools/stringprocess.hpp"

TEST(TestStr, test_str_join) {
    using namespace std;
    using namespace tools::mystr;
    using namespace std::literals;

    EXPECT_EQ("1,2,3"s,
              (std::vector<int>{1, 2, 3} | views::transform([](int x) { return std::to_string(x); }) | join(",")));
    EXPECT_EQ(""s, (std::vector<int>{} | views::transform([](int x) { return std::to_string(x); }) | join(",")));
    EXPECT_EQ(""s, (std::vector<std::string>{} | join(",")));
    EXPECT_EQ("114514"s, (std::vector<std::string>{"114514"} | join(",")));
}

TEST(TestStr, test_remove_space) {
    using namespace std;
    using namespace tools::mystr;
    using namespace std::literals;

    EXPECT_EQ("114514"s, removeSpace("  114514  "sv));
    EXPECT_EQ("114514"s, removeSpace("  114514  "s));
    EXPECT_EQ("114514"s, removeSpace("114514"sv));
    EXPECT_EQ("114514"s, removeSpace("114514"s));
    EXPECT_EQ("114514"s, removeSpace("  114514"sv));
    EXPECT_EQ("114514"s, removeSpace("  114514"s));
    EXPECT_EQ("114514"s, removeSpace("114514  "sv));
    EXPECT_EQ("114514"s, removeSpace("114514  "s));
    EXPECT_EQ(""s, removeSpace(""s));
    EXPECT_EQ(""s, removeSpace(""sv));
    EXPECT_EQ(""s, removeSpace(" "s));
    EXPECT_EQ(""s, removeSpace(" "sv));

    EXPECT_EQ("114 514"s, removeSpace("114 514"s));
    EXPECT_EQ("114 514"s, removeSpace("  114 514"s));
    EXPECT_EQ("114 514"s, removeSpace("114 514  "s));
}

// TEST(TestStr, test_str_split) {
//     using namespace std;
//     using namespace rulejit::mystr;

//     // std::vector<int> tmp;
//     // for(auto&& x : std::string("1,2,3") | split_view(",")){
//     //     tmp.push_back(std::stoi(std::string(x)));
//     // }
//     // EXPECT_EQ((std::vector<int>{1, 2, 3}), tmp);
// }

TEST(TestAny, test_any_visit) {
    using namespace tools::myany;

    ASSERT_TRUE(visit<err>(
        [](const auto &x) {
            auto name = typeid(x).name();
            if constexpr (std::is_same_v<double, std::remove_cvref_t<decltype(x)>>) {
                return x == 1.0;
            } else {
                return false;
            }
        },
        std::any(1.0)));

    ASSERT_DOUBLE_EQ(visit<err>(
                         [](const auto &x) {
                             if constexpr (std::is_constructible_v<double, decltype(x)>) {
                                 return double(x);
                             } else {
                                 return 0.;
                             }
                         },
                         std::any(1.2)),
                     1.2);

    ASSERT_DOUBLE_EQ(visit<err>(
                         [](const auto &x) {
                             if constexpr (std::is_constructible_v<double, decltype(x)>) {
                                 return double(x);
                             } else {
                                 return 0.;
                             }
                         },
                         std::any(std::string("123"))),
                     0.);

    ASSERT_THROW(visit<err>(
                     [](const auto &x) {
                         if constexpr (std::is_constructible_v<double, decltype(x)>) {
                             return double(x);
                         } else {
                             return 0.;
                         }
                     },
                     std::any("?")),
                 std::logic_error);
}

TEST(TestAny, test_any_equal) {
    using namespace std;
    using namespace tools::myany;
    using CSValueMap = std::unordered_map<std::string, std::any>;

    ASSERT_TRUE(anyEqual(std::any(1.0), std::any(1.0)));
    ASSERT_TRUE(anyEqual(std::any(1), std::any(1)));
    ASSERT_TRUE(anyEqual(std::any(true), std::any(true)));

    ASSERT_FALSE(anyEqual(std::any(1.0), std::any(1)));
    ASSERT_FALSE(anyEqual(std::any(1.0), std::any(1.1)));
    ASSERT_FALSE(anyEqual(std::any(0), std::any(false)));

    ASSERT_TRUE(anyEqual(std::any(std::vector<std::any>{1., 2., 3.}), std::any(std::vector<std::any>{1., 2., 3.})));
    ASSERT_TRUE(anyEqual(std::any(std::vector<std::any>{13, 13}), std::any(std::vector<std::any>{13, 13})));
    ASSERT_TRUE(anyEqual(std::any(std::vector<std::any>{}), std::any(std::vector<std::any>{})));

    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{1., 2., 3.}), std::any(std::vector<std::any>{1., 2., 3.1})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{1, 2, 3}), std::any(std::vector<std::any>{1., 2., 3.})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{1, 2, 3}), std::any(std::vector<std::any>{1, 3})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{1, 2, 3}), std::any(std::vector<std::any>{})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{}), std::any(std::vector<std::any>{1, 3})));
    ASSERT_FALSE(anyEqual(std::any(1.), std::any(std::vector<std::any>{1, 3})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{1, 3}), std::any(1.)));

    ASSERT_TRUE(
        anyEqual(std::any(CSValueMap{{"123", 1.}, {"456", 234}}), std::any(CSValueMap{{"123", 1.}, {"456", 234}})));
    ASSERT_TRUE(anyEqual(std::any(CSValueMap{}), std::any(CSValueMap{})));

    ASSERT_FALSE(anyEqual(std::any(CSValueMap{{"123", 1.}}), std::any(CSValueMap{{"123", 1.}, {"456", 234}})));
    ASSERT_FALSE(
        anyEqual(std::any(CSValueMap{{"123", 1.}, {"113", 234}}), std::any(CSValueMap{{"123", 1.}, {"456", 234}})));
    ASSERT_FALSE(
        anyEqual(std::any(CSValueMap{{"123", 234}, {"456", 1.}}), std::any(CSValueMap{{"123", 1.}, {"456", 234}})));
    ASSERT_FALSE(anyEqual(std::any(CSValueMap{}), std::any(CSValueMap{{"123", 1.}, {"456", 234}})));
    ASSERT_FALSE(anyEqual(std::any(13), std::any(CSValueMap{{"123", 1.}, {"456", 234}})));
    ASSERT_FALSE(anyEqual(std::any(CSValueMap{}), std::any(14.)));
    ASSERT_FALSE(anyEqual(std::any(CSValueMap{}), std::any(std::string("112453"))));
    ASSERT_FALSE(anyEqual(std::any(CSValueMap{}), std::any(std::vector<std::any>{})));

    ASSERT_TRUE(anyEqual(std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{}}),
                         std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{}})));
    ASSERT_TRUE(anyEqual(std::any(CSValueMap{{"123", 1.}, {"456", std::vector<std::any>{234}}}),
                         std::any(CSValueMap{{"123", 1.}, {"456", std::vector<std::any>{234}}})));

    ASSERT_FALSE(
        anyEqual(std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{}}),
                 std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{{"123", 1.}}})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{}}),
                          std::any(std::vector<std::any>{CSValueMap{}, CSValueMap{{"123", 1.}, {"456", 234}}})));
    ASSERT_FALSE(anyEqual(std::any(std::vector<std::any>{1.}),
                         std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{}})));
    ASSERT_FALSE(anyEqual(std::any(12),
                         std::any(std::vector<std::any>{CSValueMap{{"123", 1.}, {"456", 234}}, CSValueMap{}})));
}

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}