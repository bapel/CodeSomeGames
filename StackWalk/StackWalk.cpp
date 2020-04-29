#include <stdio.h>
#include "CallStack.h"

#include <thread>
#include <mutex>

void PrintStack();

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

int main()
{
    auto process = CallStack_Init();

    CallFoo(process);
    TestThreads(4, process);

    CallStack_Shutdown(process);
}

void PrintStack(uint64_t process)
{
    struct LineInfo
    {
        uint64_t Address;
        char* FileName;
        uint32_t LineNumber;
        char FunctionName[63];
    };

    const int k_MaxCapturedLines = 24;
    LineInfo lines[k_MaxCapturedLines];
    size_t linesCount = 0;

    CallStackContext context = { (HANDLE)process };
    CallStack_StartCapture(context, process);

    while (CallStack_Next(&context))
    {
        auto i = linesCount;
        
        lines[i].Address = context.Symbol.Address;
        lines[i].FileName = context.Line.FileName;
        lines[i].LineNumber = context.Line.LineNumber;
        strcpy_s(lines[i].FunctionName, context.FunctionName);

        linesCount++;
        if (linesCount == k_MaxCapturedLines)
            break;
    }

    g_LogMutex.lock();

    printf("\n---- Thread: %d ----\n\n", std::this_thread::get_id());

    for (auto i = 0; i < linesCount; i++)
    {
        printf("0x%llx __ %s __ file %s, line %d\n", 
            lines[i].Address,
            lines[i].FunctionName, 
            lines[i].FileName, 
            lines[i].LineNumber);
    }

    g_LogMutex.unlock();
}