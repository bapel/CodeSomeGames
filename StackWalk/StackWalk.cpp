#include <stdio.h>
#include "CallStack.h"

#include <thread>
#include <mutex>

void PrintStack(uint64_t process);

#define FooMember(idx__, idxM1__)\
    void Fn##idx__(uint64_t process) { Fn##idxM1__(process); }

class Foo
{
public:
    FooMember(16, 15);
    FooMember(15, 14);
    FooMember(14, 13);
    FooMember(13, 12);
    FooMember(12, 11);
    FooMember(11, 10);
    FooMember(10,  9);
    FooMember( 9,  8);
    FooMember( 8,  7);
    FooMember( 7,  6);
    FooMember( 6,  5);
    FooMember( 5,  4);
    FooMember( 4,  3);
    FooMember( 3,  2);
    FooMember( 2,  1);
    void Fn1(uint64_t process) { PrintStack(process); }
};

void CallFoo(uint64_t process)
{
    Foo foo;
    foo.Fn16(process);
}

std::mutex g_LogMutex;

void TestThreads(int numThreads, uint64_t process)
{
    std::thread* threads = new std::thread[numThreads];

    for (auto i = 0; i < numThreads; i++)
        threads[i] = std::thread(CallFoo, process);

    for (auto i = 0; i < numThreads; i++)
        threads[i].join();
}

#include <iostream>

int main()
{
    auto process = CallStack_Init();

    for (auto i = 0; i < 1000; i++)
    {
        printf("\n~~~~ Attempt: %d ~~~~\n", i);

        CallFoo(process);
        TestThreads(4, process);
    }

    CallStack_Shutdown(process);
}

void PrintStack(uint64_t process)
{
    g_LogMutex.lock();

    CallStack callStack = {};
    CallStack_Capture(callStack);

    // Can't printf get_id()
    std::cout<< "\n---- Thread: " << std::this_thread::get_id() << " ----\n\n";

    for (auto i = 0; i < CallStack::k_MaxDepth; i++)
    {
        if (callStack.Lines[i].Address == UINT64_MAX)
            break;

        printf("0x%llx __ %s __ file %s, line %d\n", 
            callStack.Lines[i].Address,
            callStack.Lines[i].FuncName, 
            callStack.Lines[i].FileName, 
            callStack.Lines[i].LineNumber);
    }

    g_LogMutex.unlock();
}