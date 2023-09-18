#include <stdio.h>
#include "../LambdaTram.hpp"

using namespace std;

#if defined(__ANDROID__) & false
#include <android/log.h>
#define LOGLINE __android_log_print(ANDROID_LOG_DEBUG, "LambdaTram", "%s:%d \n", __FUNCTION__, __LINE__);
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "LambdaTram", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "LambdaTram", __VA_ARGS__)
#else
#define LOGLINE printf("%s:%d \n", __FUNCTION__, __LINE__);
#define LOGD(...) printf(__VA_ARGS__)
#define LOGE(...) printf(__VA_ARGS__)
#endif

int main()
{

    LOGLINE

    LOGE("Current Main %p\n", &main);

    int test = 123;
    void *func_ptr1 = REGISTER_LAMBDA([&](int x, int y) -> void *
                                      {
        LOGE("Lambda called with %d %d %d \n", x, y, test);
        return nullptr; });

    std::function<void *(int, int)> my_lambda = [](int x, int y) -> void *
    {
        LOGE("Lambda called with %d %d \n", x, y);
        return nullptr;
    };
    auto func_ptr2 = REGISTER_LAMBDA(my_lambda);

    LOGD("func_ptr1 -> %p \n", func_ptr1);
    LOGD("func_ptr2 -> %p \n", func_ptr2);

    using FuncType1 = void *(*)(int, int);
    auto real_func1 = reinterpret_cast<FuncType1>(func_ptr1);
    real_func1(100, 200);

    test = 124;
    real_func1(222, 111);

    return 0;
}
