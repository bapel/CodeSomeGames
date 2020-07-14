#pragma once

//#include <vstl\UnorderedSet.hpp>
#include <vstl\RobinHoodSet.hpp>
#include <vstl\Array.hpp>

template <class T>
void print_stats(const vstl::RobinHoodSet<T>& set)
{
    vstl::Array<uint8_t, vstl::RobinHoodSet<T>::max_probe()> probe_dis;
    set.probe_distribution(probe_dis.data(), probe_dis.size());
    std::cout << "@TODO: probes ..." << std::endl;
}

TEST_CASE("Unordered set construction", "[set]")
{
    auto n = 50ULL;
    vstl::RobinHoodSet<uint64_t> set(n);

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

TEST_CASE("Unordered set growth", "[set]")
{
    auto n = 50ULL;
    vstl::RobinHoodSet<uint64_t> set;

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

TEST_CASE("Unordered fill many", "[set]")
{
    auto n = 5000ULL;
    vstl::RobinHoodSet<uint64_t> set(n);

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