#ifndef Test__

#include <inttypes.h>
#include <type_traits>
#include <chrono>
#include <iostream>

#define HashMetrics__
#include "Collections.hpp"
#include "ArrayList.hpp"
#include "HashSet\MetaHashSet.hpp"
#include "HashSet\SimdHashSet.hpp"
#include "HashSet\HoodHashSet.hpp"
// #include "ChunkHashSet.hpp"
// #include "PrimeHashSet.hpp"

#include <unordered_set>
#include <EASTL\unordered_set.h>

#include "Hash\Hash.hpp"

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
using Hasher = std::hash<Payload>;
//using Hasher = NamespaceName__::SoHash<Payload>;
//using Hasher = NamespaceName__::xxHash<Payload>;
//using Hasher = NamespaceName__::BrehmHash<Payload>;
const auto Count_n = 10'000'000U;
const auto Growth = 10;
const auto NumLookups = 40'000'000U;

template <class HashSetType>
void ProfileFind(uint32_t count)
{
    using namespace NamespaceName__;

    HashSetType set(count);
    ArrayList<uint64_t> hashes(count);
    const auto dv = 15704;

    HashSetType::MaxProbeLength = 0;

    for (auto i = 0U; i < set.Capacity(); i++)
    {
        if (set.ShouldRehash())
        {
            count = i;
            break;
        }

        auto hash = Hasher()(i);
        hashes.Add(hash);

        if (i == dv)
            assert(set.Add(i, hash));
        else
            assert(set.Add(i, hash));
    }

    auto nc = HashSetType::MaxProbeLength;

    assert(NumLookups > count);

    auto finds = NumLookups;
    auto repeat = Max(1, NumLookups / (2 * count));
    auto start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = 0; i < count; i++)
        {
            if (i == dv)
                assert(set.Contains(i, hashes[i]));
            else
                assert(set.Contains(i, hashes[i]));
        }
            
        for (auto i = count; i < 2 * count; i++)
            assert(!set.Contains(i, Hasher()(i)));
    }

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perQuery = elapsed / (repeat * (2 * count));
    auto load = (float)set.Count() / set.Capacity();

    std::cout 
        << perQuery << " ns, "
        << "Load: " << load << ", "
        << "Probe: " << nc << ", "
        << count << ", "
        << finds << ", "
        << elapsed << " ns"
        << std::endl;
}

template <class HashSetType>
void ProfileFind_1(uint32_t count)
{
    using namespace NamespaceName__;

    HashSetType set(count);

    for (auto i = 0U; i < count; i++)
        set.insert(i);

    assert(NumLookups > count);
    assert(NumLookups % count == 0);

    auto finds = NumLookups;
    auto repeat = NumLookups / (2 * count);
    auto start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = 0; i < count; i++)
            assert(set.find(i) != set.end());
        for (auto i = count; i < 2 * count; i++)
            assert(set.find(i) == set.end());
    }

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();

    std::cout 
        << elapsed / NumLookups << " ns, "
        << count << ", "
        << finds << ", "
        << elapsed << " ns"
        << std::endl;
}

void HashDistribution();
void TestingSandbox();
void Profiling();

int main()
{
    const auto r1 = 0b1000'0000U & 0b1111'1110U;
    const auto r2 = 0b1000'0000U & 0b0101'1100U;

    //HashDistribution();
    //TestingSandbox();
    Profiling();
    //HashToFile();
    return 0;
}

void TestingSandbox()
{
    using namespace NamespaceName__;

    //FlatHashSet<Payload, Hasher> set(8);
    //SimdHashSet<Payload, Hasher> set;
    HoodHashSet<Payload, Hasher> set;

    //for (auto i = 0U; i < 10; i++)
    //    set.Add(i);
    //
    //for (auto i = 10U; i < 20; i++)
    //    set.Add(i);

    //assert(set.Add(3) == false);
    //assert(set.Remove(3) == true);
}

void Profiling()
{
    using namespace NamespaceName__;

    auto n = Growth;
    
    //n = Growth;
    //std::cout << "std::unordered_set\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<std::unordered_set<Payload, Hasher>>(n);
    //std::cout << std::endl;

    //n = Growth;
    //std::cout << "eastl::unordered_set\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<eastl::unordered_set<Payload, Hasher>>(n);
    //std::cout << std::endl;

    //n = Growth;
    //std::cout << "eastl::hash_set\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<eastl::hash_set<Payload, Hasher>>(n);
    //std::cout << std::endl;

    //n = Growth;
    //std::cout << "MetaHashSet\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind<MetaHashSet<Payload, Hasher>>(n);
    //std::cout << std::endl;

    n = Growth;
    std::cout << "SimdHashSet\n---" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind<SimdHashSet<Payload, Hasher>>(n);
    std::cout << std::endl;

    n = Growth;
    std::cout << "HoodHashSet\n---" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind<HoodHashSet<Payload, Hasher>>(n);
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

#include <fstream>
#include <iomanip>

void HashToFile()
{
    std::ofstream file("hash.txt");
    uint32_t bn[64] = {};
    
    for (auto i = 0U; i < 5000; i++)
    {
        auto h = Hasher()(i);

        file << std::setw( 4) << i << ", ";
        file << std::setw(20) << h << ", ";

        for (auto j = 0; j < 64; j++)
        {
            //if (j > 0 && j % 4 == 0) file << ' ';
            file << (h >> (64 - j) & 1) ? '1' : '0';
            file << ',';

            if (h >> (64 - j) & 1)
                bn[j]++;
        }

        file << std::endl;
    }

    file.close();
}

#endif