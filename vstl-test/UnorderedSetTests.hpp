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

TEST_CASE("Mersenne policy", "[array-list]")
{
    using mp = vstl::detail::MersennePolicy;

    auto i = mp::init_capacity();

    do
    {
        std::cout 
            << i << ", " 
            << mp::translate_capacity(i) << ", " 
            << mp::max_size(i) << ", "
            << (float)mp::max_size(i) / mp::translate_capacity(i) << ", "
            << std::endl;
        
        for (auto j = 0ULL; j < 1'000; j++)
        {
            auto h = std::hash<uint64_t>()(j);
            auto a = mp::mod(h, i);
            auto b = h % mp::translate_capacity(i);
            REQUIRE(a == b);
            assert(a == b);
        }

        i = mp::next_capacity(i);
    }
    while (mp::translate_capacity(i) < 1'000'000);

    std::cout << std::endl;
}

TEMPLATE_TEST_CASE("Set construction", "[set]", 
    vstl::detail::PowerOfTwoPolicy, 
    vstl::detail::MersennePolicy)
{
    auto n = 50ULL;
    vstl::RobinHoodSet<uint64_t, std::hash<uint64_t>, TestType> set(n);

    REQUIRE(set.capacity() > n);

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

TEMPLATE_TEST_CASE("Set growth", "[set]", 
    vstl::detail::PowerOfTwoPolicy, 
    vstl::detail::MersennePolicy)
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
    vstl::detail::PowerOfTwoPolicy, 
    vstl::detail::MersennePolicy)
{
    auto n = 5000ULL;
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