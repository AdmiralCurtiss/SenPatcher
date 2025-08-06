#include "thread.h"

#include "util/scope.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
// thread name setting code from
// https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace {
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
    DWORD dwType;     // Must be 0x1000.
    LPCSTR szName;    // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
} // namespace

using PSetThreadDescription = HRESULT(WINAPI*)(HANDLE hThread, PCWSTR lpThreadDescription);
#endif

namespace HyoutaUtils {
#ifdef BUILD_FOR_WINDOWS
void SetThreadNameViaException(const char* name) {
    THREADNAME_INFO info{};
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = DWORD(-1);
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable : 6320 6322)
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#pragma warning(pop)
}

void SetThreadNameViaSetThreadDescription(const char* name) {
    HMODULE kernelBase = LoadLibraryW(L"KernelBase.dll");
    if (kernelBase != nullptr) {
        auto libraryScope = HyoutaUtils::MakeScopeGuard([&]() { FreeLibrary(kernelBase); });
        FARPROC setThreadDesc = GetProcAddress(kernelBase, "SetThreadDescription");
        if (setThreadDesc != nullptr) {
            auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(name, strlen(name));
            if (wstr) {
                reinterpret_cast<PSetThreadDescription>(setThreadDesc)(GetCurrentThread(),
                                                                       wstr->c_str());
            }
        }
    }
}
#endif

void SetThreadName(const char* name) {
#ifdef BUILD_FOR_WINDOWS
    // set via exception for the VS debugger
    SetThreadNameViaException(name);

    // also set via SetThreadDescription(), if available
    SetThreadNameViaSetThreadDescription(name);
#endif
}
} // namespace HyoutaUtils
