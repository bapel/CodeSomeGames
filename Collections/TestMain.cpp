void TestFreeList();
void TestAllocators();
void TestArrayList();

int main()
{
    TestFreeList();
    TestAllocators();
    TestArrayList();
}

#include "pfk\FreeList.hpp"

void PrintFreeListItems(pfk::FreeList<char> fl)
{
    for (auto i = 0; i < fl.Count(); i++)
    {
        char ch;
        if (!fl.Get(i, &ch)) continue;
        printf("%c ", ch);
    }

    printf("\n");
}

void TestFreeList()
{
    pfk::FreeList<char> fl;

    fl.Create(2);

    auto _0 = fl.Acquire(); fl[_0] = '0';
    auto _1 = fl.Acquire(); fl[_1] = '1';
    auto _2 = fl.Acquire(); fl[_2] = '2';
    auto _3 = fl.Acquire(); fl[_3] = '3';

    PrintFreeListItems(fl);

    fl.Release(_1);
    fl.Release(_2);

    PrintFreeListItems(fl);

    auto _4 = fl.Acquire(); fl[_4] = '4';
    auto _5 = fl.Acquire(); fl[_5] = '5';
    auto _6 = fl.Acquire(); fl[_6] = '6';
    auto _7 = fl.Acquire(); fl[_7] = '7';

    PrintFreeListItems(fl);

    fl.Release(_5);
    fl.Release(_6);

    PrintFreeListItems(fl);

    fl.Destroy();
}

#include "pfk\Memory.hpp"

void TestAllocators()
{
    using namespace pfk;

    pfk::memory::Init();
    auto da = pfk::memory::GetDefaultAllocator();

    void* block = pfk_malloc(da, 32, 8);
    pfk_free(da, block);

    block = pfk_malloc(da, 32, 8);
    block = pfk_malloc(da, 32, 8);

    pfk::memory::Shutdown();
}

#include "pfk\collections\ArrayList.hpp"

void TestArrayList()
{
    //using namespace pfk;
    //using namespace pfk::memory;
    //using namespace pfk::collections;

    //auto al = pstl_array_list_create();

    //pstl_array_list_free();
}