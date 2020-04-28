#include <stdio.h>
#include "StackWalk.h"

void PrintStack()
{
    auto process = StackWalk_Init();

    StackWalkContext context = { (HANDLE)process };
    StackWalk_Start(context, process);

    while (StackWalk_Next(&context))
    {
        printf("0x%llx __ %s __ file %s, line %d\n", 
            context.Symbol.Address,
            context.FunctionName, 
            context.Line.FileName, 
            context.Line.LineNumber);
    }

    StackWalk_Shutdown(process);
}

class Foo
{
public:
    void Fn5() { PrintStack(); }
    void Fn4() { Fn5(); }
    void Fn3() { Fn4(); }
    void Fn2() { Fn3(); }
    void Fn1() { Fn2(); }
};

int main()
{
    Foo foo;
    foo.Fn1();
}