#include <EASTL\vector.h>
#include <EASTL\random.h>
#include <EASTL\hash_set.h>
#include <EASTL\stack.h>
#include <EASTL\chrono.h>

#include <random>
#include <algorithm>
#include <iostream>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t align, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

using ProfilerTimePoint = eastl::chrono::time_point<eastl::chrono::system_clock>;
using ProfilerDuration = eastl::chrono::duration<eastl::chrono::system_clock>;

eastl::stack<ProfilerTimePoint> profiler;

inline ProfilerTimePoint SystemClock() { return eastl::chrono::high_resolution_clock::now(); }

inline void ProfileStart()
{
    profiler.push(SystemClock());
}

inline ProfilerDuration ProfileEnd()
{
    auto duration = SystemClock() - profiler.top();
    profiler.pop();
}

template <class T>
size_t EraseIndices(const eastl::vector<T>& vec, const eastl::vector<size_t>& indices, eastl::vector<T>& result)
{
    result = vec;
    auto sorted = indices;

    std::sort(sorted.begin(), sorted.end());

    size_t numRemoved = 0;

    for (auto i = 0; i < sorted.size(); i++)
    {
        auto idxToRemove = sorted[numRemoved] - numRemoved++;
        result.erase(result.begin() + idxToRemove);
    }

    return numRemoved;
}

template <class T>
size_t EraseIndices_1(const eastl::vector<T>& vec, const eastl::vector<size_t>& indices, eastl::vector<T>& result)
{
    result = vec;
    auto sorted = indices;

    std::sort(sorted.begin(), sorted.end());

    size_t numRemoved = 0;

    for (auto i = 0; i < sorted.size(); i++)
    {
        auto idxToRemove = sorted[numRemoved] - numRemoved++;
        result.erase(result.begin() + idxToRemove);
    }

    return numRemoved;
}

int main()
{
    using number_t = uint32_t;
    constexpr auto n_Numbers = 65536 * 2;//eastl::numeric_limits<number_t>::max() / 8192;
    constexpr auto n_Remove = n_Numbers / 2;

    eastl::vector<number_t> numbers;
    eastl::vector<size_t> remove;

    numbers.reserve(n_Numbers);
    numbers.resize(n_Numbers);

    auto i = 0ULL;
    std::mt19937 gen(0);
    std::uniform_int_distribution dis(0, (int)numbers.size() - 1);

    eastl::generate_n(numbers.data(), numbers.size(), [&i](){ return i++; });

    i = 0;
    eastl::hash_set<size_t> _remove;
    for (; i < n_Remove; i++)
        _remove.insert(dis(gen));

    remove.reserve(n_Remove);
    for (auto iter = _remove.begin(); iter != _remove.end(); iter++)
        remove.push_back(*iter);

    eastl::vector<number_t> r1;
    EraseIndices(numbers, remove, r1);

    std::cout << r1[0] << std::endl;

    return 0;
}