#ifndef Test__

//#define UseSimd__
#include "FlatHashSet.hpp"
#include <inttypes.h>
#include <type_traits>

#include <chrono>
#include <iostream>
#include <unordered_set>
#include <EASTL\unordered_set.h>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t align, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

using HiresClock = std::chrono::high_resolution_clock;
using HiresTime = HiresClock::time_point;
using HiresDuration = HiresClock::duration;
using NanoSeconds = std::chrono::nanoseconds;

using Payload = uint64_t;
const auto Count_n = (8 * 1024 * 1024) / sizeof(Payload);
const auto Growth = 10;
const auto NumLookups = 10'000'000;

void ProfileFind(uint32_t count)
{
    using namespace NamespaceName__;

    FlatHashSet<Payload> set(2 * count);

    FlatHashSet<Payload>::NumCollisions = 0;
    for (auto i = 0U; i < count; i++)
        set.Add(i);

    auto nc = FlatHashSet<Payload>::NumCollisions;
    auto finds = NumLookups;
    auto start = HiresClock::now();
    
    for (auto i = 0U; i < finds; i++)
        assert(set.Contains(i % count));

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFind = elapsed / finds;
    std::cout 
        << perFind << " ns, " 
        << count << ',' 
        << finds << ',' 
        << elapsed << " ns" 
        << std::endl;
}

void ProfileFind_1(uint32_t count)
{
    std::unordered_set<Payload> set(count);

    for (auto i = 0U; i < count; i++)
        set.insert(i);

    auto finds = NumLookups;
    auto start = HiresClock::now();

    for (auto i = 0U; i < finds; i++)
        assert(set.end() != set.find(i % count));

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFind = elapsed / finds;
    std::cout 
        << perFind << " ns, " 
        << count << ',' 
        << finds << ',' 
        << elapsed << " ns" 
        << std::endl;
}

void ProfileFind_2(uint32_t count)
{
    eastl::unordered_set<Payload> set(count);

    for (auto i = 0U; i < count; i++)
        set.insert(i);

    auto finds = NumLookups;
    auto start = HiresClock::now();

    for (auto i = 0U; i < finds; i++)
        assert(set.end() != set.find(i % count));

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFind = elapsed / finds;
    std::cout 
        << perFind << " ns, " 
        << count << ',' 
        << finds << ',' 
        << elapsed << " ns" 
        << std::endl;
}

void HashDistribution();

int main()
{
    using namespace NamespaceName__;

    //FlatHashSet<uint32_t> set(8);

    //for (auto i = 0U; i < 10; i++)
    //    set.Add(i);
    //
    //for (auto i = 10U; i < 20; i++)
    //    set.Add(i);

    //assert(set.Add(3) == false);
    //assert(set.Remove(3) == true);

    auto n = Growth;

    //n = Growth;
    //std::cout << "std::unordered_set" << std::endl;
    //for (; n < Count_n; n*=Growth)
    //    ProfileFind_1(n);
    //std::cout << std::endl;

    n = Growth;
    std::cout << "eastl::unordered_set" << std::endl;
    for (; n < Count_n; n*=Growth)
        ProfileFind_2(n);
    std::cout << std::endl;

    n = Growth;
    std::cout << "FlatHashSet" << std::endl;
    for (; n < Count_n; n*=Growth)
        ProfileFind(n);
    std::cout << std::endl;
    
    return 0;
}

#include "ArrayList.hpp"
using namespace NamespaceName__;

template <class T>
void PrintBytes(const T& value)
{
    static const char* p[16] = 
    {
        "0000","0001","0010","0011",
        "0100","0101","0110","0111",
        "1000","1001","1010","1011",
        "1100","1101","1110","1111",
    };

    const auto count = sizeof(T);
    const uint8_t* bytes = (uint8_t*)&value;

    for (auto i = 0U; i < count; i++)
    {
        auto byte = bytes[i];
        printf("%s %s ", p[byte >> 4], p[byte & 0x0F]);
    }
}

void HashDistribution()
{
    using Uint32Hash = std::hash<uint32_t>;
    Uint32Hash hashFunction;

    const auto minNumBuckets = 8;
    const auto maxNumBuckets = 32;
    ArrayList<int> buckets(minNumBuckets);

    buckets.Resize(minNumBuckets);
    memset(buckets.Data(), 0, buckets.AllocatedSize());

    for (auto i = 0U; i < 100; i++)
    {
        auto hash = hashFunction(i);
        auto bucket = hash % buckets.Count();
        buckets[bucket]++;

        auto occupied = 0U;
        for (auto j = 0U; j < buckets.Count(); j++)
            occupied += buckets[j] > 0;
        auto lf = (float)occupied / buckets.Count();

        if (bucket == 7)
        {
            printf("%4u | %24llu | " , i, hash);
            PrintBytes(hash);
            printf(" | %4llu | LF: %f |", bucket, lf);
            for (auto j = 0U; j < buckets.Count(); j++)
                printf(" %d |", buckets[j]);
            printf("\n");
        }

        //if (occupied + 1 >= buckets.Count())
        //{
        //    auto numBuckets = 2 * buckets.Count();
        //    if (numBuckets > maxNumBuckets)
        //        goto finish;

        //    i = 0U;
        //    buckets.Clear();
        //    buckets.Resize(numBuckets);
        //    memset(buckets.Data(), 0, buckets.AllocatedSize());
        //}
    }

finish:
    return;
}

#endif