//
// Custom Lib For E-Bot
// E-Bot is ai controlled players for counter-strike 1.6
// 
// For reduce glibc requirements and take full advantage of compiler optimizations
// And to get same results/performance on every OS
//

#pragma once
#include <functional>
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <cstdint>
#define INFINITE static_cast<uint32_t>(-1)
#endif

class cthread
{
private:
#ifdef WIN32
    HANDLE thread = nullptr;
    static DWORD WINAPI local(LPVOID lpParam)
    {
        const std::function<void()>* func = static_cast<std::function<void()>*>(lpParam);
        if (func != nullptr)
            (*func)();
        return 0;
    }
#else
    pthread_t thread = 0; // Zero is commonly used to indicate an invalid thread
    static pthread_t invalid_thread;
    static void* local(void* arg)
    {
        const std::function<void()>* func = static_cast<std::function<void()>*>(arg);
        if (func != nullptr)
            (*func)();
        return 0;
    }
#endif

public:
    cthread(const std::function<void()>& func)
    {
#ifdef WIN32
        thread = CreateThread(nullptr, 0, local, new std::function<void()>(func), 0, 0);
#else
        pthread_create(&thread, nullptr, local, new std::function<void()>(func));
#endif
    }

    bool join(uint32_t timeout = INFINITE)
    {
        if (!thread)
            return false;

#ifdef WIN32
        WaitForSingleObject(thread, timeout);
        CloseHandle(thread);
        thread = nullptr;
#else
        if (pthread_join(thread, nullptr) != 0)
            return false;

        pthread_cancel(thread);
#endif
        return true;
    }

    bool detach(void)
    {
        if (!thread)
            return false;

#ifdef WIN32
        CloseHandle(thread);
        thread = nullptr;
#else
        if (pthread_detach(thread) != 0)
            return false;

        pthread_cancel(thread);
#endif
        return true;
    }

    bool async(void)
    {
        return detach();
    }
};