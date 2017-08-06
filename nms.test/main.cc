
#include <stdio.h>

#if defined(__linux__) || defined(__APPLE__)
#   include<dlfcn.h>
#endif

#if defined(_WIN32)
extern "C"
{
    const auto     RTLD_LAZY = 1;
    static void*   INVALID_HANDLE_VALUE = reinterpret_cast<void*>(0ll - 1ll);
    void*          LoadLibraryA(const char* path);
    int            FreeLibrary(void* handle);
    void*          GetProcAddress(void* handle, const char* name);
}

static void* dlopen(const char* path, int mode) {
    auto ret = LoadLibraryA(path);
    (void)mode;
    if (ret == INVALID_HANDLE_VALUE) ret = nullptr;
    return ret;
}

static void* dlsym(void* handle, const char* symbol) {
    auto ret = GetProcAddress(handle, symbol);
    return ret;
}

static int dlclose(void* handle) {
    auto ret = ::FreeLibrary(handle);
    return ret;
}
#endif

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("%s\n", argv[0]);
        printf("    %s [path] ...\n", argv[0]);
        return -1;
    }
    const auto path = argv[1];
    auto library = dlopen(path, RTLD_LAZY);

    if (library == nullptr) {
        printf("load library: %s failed", argv[1]);
        return -1;
    }

    auto test = dlsym(library, "test");

    if (!test) {
        printf("cannot locate: %s\n", "test");
        return -1;
    }

    typedef int test_t(int $argc, const char* $argv[]);
    auto ftest  = reinterpret_cast<test_t*>(test);
    auto ret    = (*ftest)(argc - 2, argv + 2);
    return ret;
}
