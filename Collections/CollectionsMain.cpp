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

using Payload = uint32_t;
//using Hasher = std::hash<Payload>;
using Hasher = NamespaceName__::SoHash<Payload>;
//using Hasher = NamespaceName__::BrehmHash<Payload>;
const auto Count_n = 10'000'000U;
const auto Growth = 10;
const auto NumLookups = 10'000'000U;

template <class HashSetType>
void PrintProbeStats(const HashSetType&) { }

void PrintProbeStats(const pstl::HoodHashSet<Payload, Hasher>& set) 
{
    pstl::ArrayList<uint8_t> probes;
    pstl::ArrayList<uint32_t> distribution;
    
    probes.Resize(set.Count());
    distribution.Resize(256, 0);

    set.GetProbes(probes.Data(), probes.Count());

    for (auto i = 0U; i < probes.Count(); i++)
    {
        auto length = probes[i];
        distribution[length]++;
    }

    for (auto i = 0U; i < distribution.Count(); i++)
    {
        if (distribution[i] == 0)
            continue;

        auto p = (100.0f * distribution[i]) / set.Count();
        
        std::cout << i << ": " << p << "%, ";
        std::cout << distribution[i] << " in " << set.Count() << std::endl;
    }
}

template <class HashSetType>
void ProfileFind(uint64_t count)
{
    using namespace NamespaceName__;

    HashSetType set(count);
    ArrayList<uint64_t> hashes(count);
    
    HashSetType::MaxProbeLength = 0;

    for (auto i = 0U; i < count; i++)
    {
        auto hash = Hasher()(i);
        assert(set.Add(i, hash));
        hashes.Add(hash);
    }

    auto nc = HashSetType::MaxProbeLength;

    auto repeat = Max(1UL, NumLookups / count);
    auto start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = 0; i < count; i++)
            set.Contains(i);
    }

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perSuccess = elapsed / (repeat * count);

    start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = count; i < 2UL * count; i++)
            set.Contains(i);
    }

    elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFail = elapsed / (repeat * count);
    
    auto load = (float)set.Count() / set.Capacity();

    std::cout 
        << perSuccess << " ns, "
        << perFail << " ns, "
        << "Load: " << load << ", "
        << "Probe: " << nc << ", "
        << count
        << std::endl;

    //std::cout << std::endl;
    //PrintProbeStats(set);
    //std::cout << std::endl;
}

template <class HashSetType>
void ProfileFind_1(uint64_t count)
{
    using namespace NamespaceName__;

    HashSetType set(2 * count);
    set.max_load_factor(1.0f);

    for (auto i = 0U; i < count; i++)
        set.insert(i);

    auto repeat = Max(1UL, NumLookups / count);
    auto start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = 0; i < count; i++)
        {
            auto iter = set.find(i);
            assert(set.end() != iter);
        }
    }

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perSuccess = elapsed / (repeat * count);

    start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = count; i < 2UL * count; i++)
            assert(set.end() == set.find(i));
    }

    elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perFail = elapsed / (repeat * count);

    std::cout 
        << perSuccess << " ns, "
        << perFail << " ns, "
        << "Load: " << set.load_factor() << ", "
        << count
        << std::endl;
}

void HashDistribution();
void TestingSandbox();
void Profiling();

int main()
{
    auto m = 0b0001'1001;

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
    
    n = Growth;
    std::cout << "std::unordered_set\n---" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind_1<std::unordered_set<Payload, Hasher>>(n);
    std::cout << std::endl;

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

    n = Growth;
    std::cout << "MetaHashSet\n---" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind<MetaHashSet<Payload, Hasher>>(n);
    std::cout << std::endl;

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