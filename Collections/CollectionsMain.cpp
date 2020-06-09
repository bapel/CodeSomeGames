#ifndef Test__

//#define UseSimd__
#include "MetaHashSet.hpp"
#include "SimdHashSet.hpp"
#include "PrimeHashSet.hpp"
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

template <class T>
struct SoHash;

template <>
struct SoHash<uint64_t>
{
    uint64_t operator()(uint64_t x) const
    {
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        x = x ^ (x >> 31);
        return x;
    }
};

using Payload = uint64_t;
using Hasher = SoHash<Payload>;
const auto Count_n = 1'000'000U; //(16 * 1024 * 1024) / sizeof(Payload);
const auto Growth = 10;
const auto NumLookups = 10'000'000;

template <class HashSetType>
void ProfileFind(uint32_t count)
{
    HashSetType set(1.5f * count);

    HashSetType::MaxProbeLength = 0;
    for (auto i = 0U; i < count; i++)
        set.Add(i);

    auto nc = HashSetType::MaxProbeLength;
    auto success = 0U;
    auto finds = NumLookups;
    auto start = HiresClock::now();

    for (auto i = 0U; i < finds; i++)
        success += set.Contains(i);

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFind = elapsed / finds;
    std::cout 
        << perFind << " ns, "
        << "MPL: " << nc << " "
        << success << ", "
        << finds << ", "
        << elapsed << " ns"
        << std::endl;
}

template <class HashSetType>
void ProfileFind_1(uint32_t count)
{
    HashSetType set(1.5f * count);

    for (auto i = 0U; i < count; i++)
        set.insert(i);

    auto success = 0U;
    auto finds = NumLookups;
    auto start = HiresClock::now();

    for (auto i = 0U; i < finds; i++)
        success += (set.end() != set.find(i));

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFind = elapsed / finds;
    std::cout 
        << perFind << " ns, "
        << success << ", "
        << finds << ", "
        << elapsed << " ns"
        << std::endl;
}

void HashDistribution();
void TestingSandbox();
void Profiling();

int main()
{
    //HashDistribution();
    //TestingSandbox();
    Profiling();
    return 0;
}

void TestingSandbox()
{
    using namespace NamespaceName__;

    //FlatHashSet<uint32_t> set(8);
    SimdHashSet<uint32_t> set(8);

    for (auto i = 0U; i < 10; i++)
        set.Add(i);
    
    for (auto i = 10U; i < 20; i++)
        set.Add(i);

    assert(set.Add(3) == false);
    assert(set.Remove(3) == true);
}

void Profiling()
{
    using namespace NamespaceName__;

    auto n = Growth;
    
    n = Growth;
    std::cout << "std::unordered_set" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind_1<std::unordered_set<Payload>>(n);
    std::cout << std::endl;

    n = Growth;
    std::cout << "eastl::unordered_set" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind_1<eastl::unordered_set<Payload, Hasher>>(n);
    std::cout << std::endl;

    n = Growth;
    std::cout << "MetaHashSet" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind<MetaHashSet<Payload, Hasher>>(n);
    std::cout << std::endl;

    n = Growth;
    std::cout << "SimdHashSet" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind<SimdHashSet<Payload, Hasher>>(n);
    std::cout << std::endl;
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

        //if (bucket == 7)
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