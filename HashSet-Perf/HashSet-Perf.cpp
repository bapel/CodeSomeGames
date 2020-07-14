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
#include <vstl\UnorderedSet.hpp>

#include <unordered_set>
#include <EASTL\unordered_set.h>

#include <tsl\robin_set.h>
#include <tsl\sparse_set.h>
#include <tsl\hopscotch_set.h>

#include "Hash\Hash.hpp"

#undef NDEBUG
#include <assert.h>

//void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
//{
//    return new uint8_t[size];
//}
//
//void* __cdecl operator new[](size_t size, size_t align, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
//{
//    return new uint8_t[size];
//}

// Global variables.
const size_t bigger_than_cachesize = 10 * 1024 * 1024;
long p[bigger_than_cachesize];

// When you want to "flush" cache. 
#define FlushCache__()\
{\
    for(int i = 0; i < bigger_than_cachesize; i++)\
        p[i] = rand();\
}

using HiresClock = std::chrono::high_resolution_clock;
using HiresTime = HiresClock::time_point;
using HiresDuration = HiresClock::duration;
using NanoSeconds = std::chrono::nanoseconds;

using Payload = uint64_t;
//using Hasher = pstl::Identity<Payload>;
//using Hasher = std::hash<Payload>;
//using Hasher = pstl::udb2Hash;
//using Hasher = pstl::SoHash<Payload>;
using Hasher = pstl::BrehmHash<Payload>;
const auto Count_n = pstl::Min(50'000'000U, std::numeric_limits<Payload>::max());
const auto Growth = 5;
const auto NumLookups = 5'000'000U;

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

    std::cout << std::endl;

    for (auto i = 0U; i < distribution.Count(); i++)
    {
        if (distribution[i] == 0)
            continue;

        auto p = (100.0f * distribution[i]) / set.Count();

        std::cout << i << ": " << p << "%, ";
        std::cout << distribution[i] << " in " << set.Count() << std::endl;
    }

    std::cout << std::endl;
}

template <class HashSetType>
void ProfileFind(uint64_t count, const std::vector<Payload>& payload)
{
    using namespace NamespaceName__;

    HashSetType set(count);
    ArrayList<uint64_t> hashes(count);

    HashSetType::MaxProbeLength = 0;

    auto i = 0UL;
    while (!set.ShouldRehash())
        set.Add(payload[i++]);
    count = i;

    auto nc = HashSetType::MaxProbeLength;

    auto repeat = Max(1UL, NumLookups / count);
    auto start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = 0; i < count; i++)
        {
            auto res = set.Contains(payload[i]);
            assert(res == true);
        }
    }

    auto success_e = NanoSeconds(HiresClock::now() - start).count();
    auto success_n = (repeat * count);
    auto perSuccess = success_e / success_n;

    start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = count; i < 2UL * count; i++)
        {
            auto res = set.Contains(payload[i]);
            assert(res == false);
        }
    }

    auto fail_e = NanoSeconds(HiresClock::now() - start).count();
    auto fail_n = (repeat * count);
    auto perFail = fail_e / fail_n;

    auto load = (float)set.Count() / set.Capacity();

    std::cout 
        << perSuccess << " ns, "
        << perFail << " ns, "
        << "Load: " << load << ", "
        << count << ", "
        << "Probe: " << nc << ", "
        << success_e << " / " << success_n << ", "
        << fail_e << " / " << fail_n << ", "
        << std::endl;

    //PrintProbeStats(set);
}

template <class HashSetType>
void ProfileFind_1(uint64_t count, const std::vector<Payload>& payload)
{
    using namespace NamespaceName__;

    HashSetType set(count);
    // set.max_load_factor(0.9375f);

    auto i = 0;
    // while (set.load_factor() < (13.0f / 14.0f))
    // while (set.load_factor() < 0.75f)
    while (set.load_factor() < 0.5f)
    {
        auto r = set.insert(payload[i]);
        assert(*r.first == payload[i]);
        if (!r.second)
            break;
        i++;
    }
    count = i;

    auto repeat = Max(1UL, NumLookups / count);
    auto start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = 0; i < count; i++)
        {
            auto iter = set.find(payload[i]);
            assert(set.end() != iter);
            assert(payload[i] == *iter);
        }
    }

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();
    auto perSuccess = elapsed / (repeat * count);

    start = HiresClock::now();

    for (auto r = 0U; r < repeat; r++)
    {
        for (auto i = count; i < 2UL * count; i++)
        {
            auto iter = set.find(payload[i]);
            assert(set.end() == iter);
        }
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

vx_inline__ uint64_t mersenne_mod_classic(uint64_t n, uint64_t s)
{ return n % ((1ULL << s) - 1ULL); }

vx_inline__ uint64_t mersenne_mod_fast(uint64_t n, uint64_t s)
{
    uint64_t m;                            // n % d goes here.
    const uint64_t d = (1ULL << s) - 1ULL; // so d is either 1, 3, 7, 15, 31, ...).

    for (m = n; n > d; n = m)
        for (m = 0ULL; n; n >>= s)
            m += n & d;

    // Now m is a value from 0 to d, but since with modulus division
    // we want m to be 0 when it is d.
    m = m == d ? 0ULL : m;
    return m;
}

vx_inline__ uint64_t mersenne_mod_faster(uint64_t n, uint64_t s)
{
    static const unsigned int M[] = 
    {
        0x00000000, 0x55555555, 0x33333333, 0xc71c71c7,  
        0x0f0f0f0f, 0xc1f07c1f, 0x3f03f03f, 0xf01fc07f, 
        0x00ff00ff, 0x07fc01ff, 0x3ff003ff, 0xffc007ff,
        0xff000fff, 0xfc001fff, 0xf0003fff, 0xc0007fff,
        0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff, 
        0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
        0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
        0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff
    };

    static const unsigned int Q[][6] = 
    {
        { 0,  0,  0,  0,  0,  0}, {16,  8,  4,  2,  1,  1}, {16,  8,  4,  2,  2,  2},
        {15,  6,  3,  3,  3,  3}, {16,  8,  4,  4,  4,  4}, {15,  5,  5,  5,  5,  5},
        {12,  6,  6,  6 , 6,  6}, {14,  7,  7,  7,  7,  7}, {16,  8,  8,  8,  8,  8},
        { 9,  9,  9,  9,  9,  9}, {10, 10, 10, 10, 10, 10}, {11, 11, 11, 11, 11, 11},
        {12, 12, 12, 12, 12, 12}, {13, 13, 13, 13, 13, 13}, {14, 14, 14, 14, 14, 14},
        {15, 15, 15, 15, 15, 15}, {16, 16, 16, 16, 16, 16}, {17, 17, 17, 17, 17, 17},
        {18, 18, 18, 18, 18, 18}, {19, 19, 19, 19, 19, 19}, {20, 20, 20, 20, 20, 20},
        {21, 21, 21, 21, 21, 21}, {22, 22, 22, 22, 22, 22}, {23, 23, 23, 23, 23, 23},
        {24, 24, 24, 24, 24, 24}, {25, 25, 25, 25, 25, 25}, {26, 26, 26, 26, 26, 26},
        {27, 27, 27, 27, 27, 27}, {28, 28, 28, 28, 28, 28}, {29, 29, 29, 29, 29, 29},
        {30, 30, 30, 30, 30, 30}, {31, 31, 31, 31, 31, 31}
    };

    static const unsigned int R[][6] = 
    {
        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0x0000ffff, 0x000000ff, 0x0000000f, 0x00000003, 0x00000001, 0x00000001},
        {0x0000ffff, 0x000000ff, 0x0000000f, 0x00000003, 0x00000003, 0x00000003},
        {0x00007fff, 0x0000003f, 0x00000007, 0x00000007, 0x00000007, 0x00000007},
        {0x0000ffff, 0x000000ff, 0x0000000f, 0x0000000f, 0x0000000f, 0x0000000f},
        {0x00007fff, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f, 0x0000001f},
        {0x00000fff, 0x0000003f, 0x0000003f, 0x0000003f, 0x0000003f, 0x0000003f},
        {0x00003fff, 0x0000007f, 0x0000007f, 0x0000007f, 0x0000007f, 0x0000007f},
        {0x0000ffff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff},
        {0x000001ff, 0x000001ff, 0x000001ff, 0x000001ff, 0x000001ff, 0x000001ff}, 
        {0x000003ff, 0x000003ff, 0x000003ff, 0x000003ff, 0x000003ff, 0x000003ff}, 
        {0x000007ff, 0x000007ff, 0x000007ff, 0x000007ff, 0x000007ff, 0x000007ff}, 
        {0x00000fff, 0x00000fff, 0x00000fff, 0x00000fff, 0x00000fff, 0x00000fff}, 
        {0x00001fff, 0x00001fff, 0x00001fff, 0x00001fff, 0x00001fff, 0x00001fff}, 
        {0x00003fff, 0x00003fff, 0x00003fff, 0x00003fff, 0x00003fff, 0x00003fff}, 
        {0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff, 0x00007fff}, 
        {0x0000ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff}, 
        {0x0001ffff, 0x0001ffff, 0x0001ffff, 0x0001ffff, 0x0001ffff, 0x0001ffff}, 
        {0x0003ffff, 0x0003ffff, 0x0003ffff, 0x0003ffff, 0x0003ffff, 0x0003ffff}, 
        {0x0007ffff, 0x0007ffff, 0x0007ffff, 0x0007ffff, 0x0007ffff, 0x0007ffff},
        {0x000fffff, 0x000fffff, 0x000fffff, 0x000fffff, 0x000fffff, 0x000fffff}, 
        {0x001fffff, 0x001fffff, 0x001fffff, 0x001fffff, 0x001fffff, 0x001fffff}, 
        {0x003fffff, 0x003fffff, 0x003fffff, 0x003fffff, 0x003fffff, 0x003fffff}, 
        {0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff}, 
        {0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff},
        {0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff, 0x01ffffff}, 
        {0x03ffffff, 0x03ffffff, 0x03ffffff, 0x03ffffff, 0x03ffffff, 0x03ffffff}, 
        {0x07ffffff, 0x07ffffff, 0x07ffffff, 0x07ffffff, 0x07ffffff, 0x07ffffff},
        {0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff},
        {0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff}, 
        {0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff}, 
        {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff}
    };

    uint64_t m = 0;                        // n % d goes here.
    const uint64_t d = (1ULL << s) - 1ULL; // so d is either 1, 3, 7, 15, 31, ...).

    m = (n & M[s]) + ((n >> s) & M[s]);

    for (const unsigned int * q = &Q[s][0], * r = &R[s][0]; m > d; q++, r++)
        m = (m >> *q) + (m & *r);

    m = m == d ? 0 : m; // OR, less portably: m = m & -((signed)(m - d) >> s);

    return m;
}

vx_inline__ uint64_t pow2_mod_fast(uint64_t n, uint64_t s)
{ return n & ((1ULL << s) - 1ULL); }

uint64_t profile_function(const char* spaces, const char* name, uint64_t (*fn)(uint64_t, uint64_t))
{
    uint64_t accum = 0ULL;
    const auto n = 10'000'000ULL;

    auto start = HiresClock::now();

    for (auto i = 0ULL; i < n; i++)
        accum += fn(i, 7);

    auto elapsed = NanoSeconds(HiresClock::now() - start).count();;
    auto dt = elapsed / n;

    std::cout << spaces << name << ": " << dt << ", " << accum << std::endl;

    return accum;
}

#define profile_function__(Spaces__, Func__) profile_function(Spaces__, #Func__, Func__)

int main()
{
    {
        profile_function__("", mersenne_mod_classic);
        profile_function__("   ", mersenne_mod_fast);
        profile_function__(" ", mersenne_mod_faster);
        profile_function__("       ", pow2_mod_fast);
        std::cout << std::endl;
    }

    //HashDistribution();
    //TestingSandbox();
    //Profiling();
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

#include <algorithm>
#include <random>

void Profiling()
{
    using namespace NamespaceName__;

    std::vector<Payload> payload;

    auto i = 0ULL;
    payload.resize(4UL * Count_n);
    std::generate_n(payload.data(), payload.size(), [&i]() { return i++; });

    // std::random_device device;
    //std::mt19937 generator(0);
    //std::shuffle(payload.begin(), payload.end(), generator);

    auto n = Growth;

    n = Growth;
    std::cout << "std::unordered_set\n---" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind_1<std::unordered_set<Payload, Hasher>>(n, payload);
    std::cout << std::endl;

    //n = Growth;
    //std::cout << "tsl::robin_set\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<tsl::robin_set<Payload, Hasher>>(n, payload);
    //std::cout << std::endl;

    //n = Growth;
    //std::cout << "tsl::sparse_set\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<tsl::sparse_set<Payload, Hasher>>(n, payload);
    //std::cout << std::endl;

    //n = Growth;
    //std::cout << "tsl::hopscotch_set\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<tsl::hopscotch_set<Payload, Hasher>>(n, payload);
    //std::cout << std::endl;

    //n = Growth;
    //std::cout << "vstl::UnorderedSet\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind_1<vstl::UnorderedSet<Payload, Hasher>>(n, payload);
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
    //    ProfileFind<MetaHashSet<Payload, Hasher>>(n, payload);
    //std::cout << std::endl;

    n = Growth;
    std::cout << "SimdHashSet\n---" << std::endl;
    for (; n <= Count_n; n*=Growth)
        ProfileFind<SimdHashSet<Payload, Hasher>>(n, payload);
    std::cout << std::endl;

    //n = Growth;
    //std::cout << "HoodHashSet\n---" << std::endl;
    //for (; n <= Count_n; n*=Growth)
    //    ProfileFind<HoodHashSet<Payload, Hasher>>(n, payload);
    //std::cout << std::endl;
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
        buckets[(uint32_t)bucket]++;

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