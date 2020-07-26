#pragma once

#include <catch.hpp>
//#include <vstl\UnorderedSet.hpp>
#include <vstl\RobinHoodSet.hpp>
#include <vstl\Array.hpp>

template <class T, class H, class B>
void print_stats(const vstl::RobinHoodSet<T, H, B>& set)
{
    vstl::Array<uint8_t, vstl::RobinHoodSet<T, H, B>::max_probe()> probe_dis;
    set.probe_distribution(probe_dis.data(), probe_dis.size());
    std::cout << "@TODO: probes ..." << std::endl;
}

TEMPLATE_TEST_CASE("Set construction", "[set]", 
    vstl::detail::PowerOfTwoPolicy)
{
    auto n = 16ULL;
    vstl::RobinHoodSet<uint64_t, std::hash<uint64_t>, TestType> set(n);

    REQUIRE(set.bucket_count() >= n);

    SECTION("Can successfully insert unique values?")
    {
        for (auto i = 0ULL; i < n; i++)
        {
            // @Todo: could hash produce non-unique values?
            // Since input and output type are the same-size, probably not?

            auto x = std::hash<uint64_t>()(i);
            auto r = set.insert(x);

            REQUIRE(r.second);
        }

        REQUIRE(set.size() == n);
    }

    SECTION("All inserted values can be retrieved?")
    {
        for (auto i = 0ULL; i < n; i++)
        {
            auto x = std::hash<uint64_t>()(i);
            auto iter = set.find(x);

            REQUIRE(iter != set.end());
            REQUIRE(x == *iter);
        }
    }

    SECTION("For loop with iterators matches count?")
    {
        auto count = 0ULL;
        for (auto iter = set.cbegin(); iter < set.cend(); ++iter)
            count++;

        REQUIRE(count == n);
    }

    SECTION("Range for loop matches count?")
    {
        auto count = 0ULL;
        for (auto& x : set)
            count++;

        REQUIRE(count == n);
    }
}

TEMPLATE_TEST_CASE("Set growth", "[set]", 
    vstl::detail::PowerOfTwoPolicy)
{
    auto n = 50ULL;
    vstl::RobinHoodSet<uint64_t, std::hash<uint64_t>, TestType> set;

    for (auto i = 0ULL; i < n; i++)
    {
        auto r = set.insert(i);
        assert(r.second);
        REQUIRE(r.second);
    }

    REQUIRE(set.size() == n);

    for (auto i = 0ULL; i < n; i++)
    {
        auto iter = set.find(i);
        auto value = *iter;
        REQUIRE(iter != set.end());
        REQUIRE(i == value);
    }

    // iterators work?
    auto i = 0;
    for (auto iter = set.cbegin(); iter < set.cend(); ++iter)
    {
        const auto x = *iter;
        REQUIRE(x >= 0);
        assert(x < n);
        REQUIRE(x < n);
        i++;
    }

    // ranged for works/compiles?
    for (auto& x : set)
    {
        REQUIRE(x >= 0);
        assert(x < n);
        REQUIRE(x < n);
    }
}

TEMPLATE_TEST_CASE("Set fill many", "[set]",
    vstl::detail::PowerOfTwoPolicy)
{
    auto n = 4096;
    vstl::RobinHoodSet<uint64_t, std::hash<uint64_t>, TestType> set(n);

    for (auto i = 0ULL; i < n; i++)
    {
        auto r = set.insert(i);
        assert(r.second);
        REQUIRE(r.second);
        auto x = *r.first;
        assert(x == i);
        REQUIRE(x == i);
    }

    REQUIRE(set.size() == n);

    for (auto i = 0ULL; i < n; i++)
    {
        auto iter = set.find(i);
        auto value = *iter;
        assert(iter != set.end());
        REQUIRE(iter != set.end());
        REQUIRE(i == value);
    }

    print_stats(set);
}