#pragma once

#include "..\Collections.hpp"
#include <EASTL\functional.h>

Namespace__
{
    template <class T>
    using Identity = eastl::hash<T>;

    struct udb2Hash
    {
        __inline uint32_t operator()(uint32_t key) const
        {
            key += ~(key << 15);
            key ^=  (key >> 10);
            key +=  (key << 3);
            key ^=  (key >> 6);
            key += ~(key << 11);
            key ^=  (key >> 16);
            return key;
        }
    };

    // What integer hash function are good that accepts an integer hash key?
    // https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key/

    template <class T>
    struct BrehmHash;

    template <>
    struct BrehmHash<uint64_t>
    {
        #define xorshift(n__, i__) (n__ ^ (n__ >> i__))

        __inline uint64_t operator()(const uint64_t& n) const
        {
            const uint64_t p = 0x5555555555555555ull; // pattern of alternating 0 and 1
            const uint64_t c = 17316035218449499591ull; // random uneven integer constant; 
            return c * xorshift(p * xorshift(n, 32), 32);
        }
    };

    template <class T>
    struct SoHash;

    template <>
    struct SoHash<uint64_t>
    {
        __inline uint64_t operator()(uint64_t x) const
        {
            x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
            x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
            x = x ^ (x >> 31);
            return x;
        }
    };

    template <>
    struct SoHash<uint32_t>
    {
        __inline uint32_t operator()(uint32_t x) const
        {
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            return x;
        }
    };
}