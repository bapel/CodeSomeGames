#pragma once

// Taken from: http://www.rioki.org/2017/01/09/windows_stacktrace.html

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winnt.h>
#include <Psapi.h>
#include <stdint.h>

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
//#pragma pack( push, before_imagehlp, 8 )
#include <imagehlp.h>
//#pragma pack( pop, before_imagehlp )

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

struct StackWalkContext
{
    static const auto k_FunctionNameLen = 63;
    HANDLE Process;
    HANDLE Thread;
    STACKFRAME64 StackFrame;
    CONTEXT ContextRecord;
    DWORD64 Displacement;
    IMAGEHLP_SYMBOL64 Symbol;
    char FunctionName[k_FunctionNameLen];
    DWORD Offset;
    IMAGEHLP_LINE64 Line;
};

// This has to be a macro to capture context at the call site.
#define StackWalk_Start(context__, process__)\
{\
    context__ = { (HANDLE)process__, GetCurrentThread(), {}, {} };\
    context__.ContextRecord.ContextFlags = CONTEXT_FULL;\
    RtlCaptureContext(&context__.ContextRecord);\
    context__.StackFrame.AddrPC.Offset    = context__.ContextRecord.Rip;\
    context__.StackFrame.AddrPC.Mode      = AddrModeFlat;\
    context__.StackFrame.AddrStack.Offset = context__.ContextRecord.Rsp;\
    context__.StackFrame.AddrStack.Mode   = AddrModeFlat;\
    context__.StackFrame.AddrFrame.Offset = context__.ContextRecord.Rbp;\
    context__.StackFrame.AddrFrame.Mode   = AddrModeFlat;\
    context__.Symbol.SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL64);\
    context__.Symbol.MaxNameLength = StackWalkContext::k_FunctionNameLen;\
}

#if __cplusplus
extern "C" {
#endif

uint64_t StackWalk_Init()
{
    auto process = GetCurrentProcess();
    SymInitialize(process, 0, true);
    SymSetOptions(SYMOPT_LOAD_LINES);
    return (uint64_t)process;
}

bool StackWalk_Next(StackWalkContext* inOutContext)
{ 
    auto result = StackWalk64(
        IMAGE_FILE_MACHINE_AMD64,
        inOutContext->Process,
        inOutContext->Thread,
        &inOutContext->StackFrame,
        &inOutContext->ContextRecord,
        nullptr, // ReadMemoryRoutine
        SymFunctionTableAccess,
        SymGetModuleBase,
        nullptr); // TranslateAddress

    if (result)
    {
        SymGetSymFromAddr64(
            inOutContext->Process, 
            inOutContext->StackFrame.AddrPC.Offset, 
            &inOutContext->Displacement, 
            (PIMAGEHLP_SYMBOL64)&inOutContext->Symbol);

        UnDecorateSymbolName(
            inOutContext->Symbol.Name, 
            inOutContext->FunctionName, 
            63, UNDNAME_COMPLETE);

        SymGetLineFromAddr64(
            inOutContext->Process, 
            inOutContext->StackFrame.AddrPC.Offset, 
            &inOutContext->Offset, 
            &inOutContext->Line);
    }

    return result;
}

void StackWalk_Shutdown(uint64_t process)
{
    SymCleanup((HANDLE)process);
}

#if __cplusplus
}
#endif