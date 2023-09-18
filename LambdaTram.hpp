#ifndef IL2CPPHOOKER_LAMBDATRAM_HPP
#define IL2CPPHOOKER_LAMBDATRAM_HPP

/**
 * Description:
 * This class is used to register lambda function and convert it to a function pointer.
 * resolve the problem that Dobby can only pass function pointer as callback,
 * but not lambda expression with captured parameters
 *
 * Using example:
```
        xdl_info_t dynamicInfo = {};
        auto handle = xdl_open("libxxx.so", RTLD_LAZY);
        xdl_info(handle, XDL_DI_DLINFO, &dynamicInfo);
        void* base = dynamicInfo.dli_fbase;

        LOGE("dli_fbase -> {}", base);

        int test = 123;
        void* func_ptr1 = REGISTER_LAMBDA([&](int x, int y) -> void* {
            LOGE("Lambda called with {} {} {}", x, y, test);
            return nullptr;
        });

        std::function<void*(int, int)> my_lambda = [](int x, int y) -> void* {
            LOGE("Lambda called with {} {}", x, y);
            return nullptr;
        };
        auto func_ptr2 = REGISTER_LAMBDA(my_lambda);

        LOGD("func_ptr1 -> {} -> {}", func_ptr1, (void*)((u_long)func_ptr1 - (u_long)base));
        LOGD("func_ptr2 -> {} -> {}", func_ptr2, (void*)((u_long)func_ptr2 - (u_long)base));

        using FuncType1 = void* (*)(int, int);
        auto real_func1 = reinterpret_cast<FuncType1>(func_ptr1);
        real_func1(100, 200);

        test = 124;
        real_func1(222, 111);
```
 */

#include <iostream>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <type_traits>

using namespace std;

#define T int

template<T N>
struct Counter {
    static const T value = N;
};

template<T ID, typename Func>
struct FunctionWrapper;

template<T ID, typename Ret, typename ...Args>
struct FunctionWrapper<ID, std::function<Ret(Args...)>> {
    static std::function<Ret(Args...)> func;

    static Ret Trampoline(Args... args) {
        return func(args...);
    }
};

template<T ID, typename Ret, typename ...Args>
std::function<Ret(Args...)> FunctionWrapper<ID, std::function<Ret(Args...)>>::func;

class LambdaTram {
public:
    template<T ID, typename Ret, typename ...Args>
    static void* RegisterAndConvert(std::function<Ret(Args...)> func) {
        FunctionWrapper<ID, std::function<Ret(Args...)>>::func = func;
        return reinterpret_cast<void*>(&FunctionWrapper<ID, std::function<Ret(Args...)>>::Trampoline);
    }

    template<T ID, typename Callable>
    static void* RegisterAndConvert(Callable&& callable) {
        return RegisterAndConvert<ID>(std::function(callable));
    }

    template<T ID, typename Ret, typename ...Args>
    static Ret Execute(Args... args) {
        return FunctionWrapper<ID, std::function<Ret(Args...)>>::Trampoline(args...);
    }
};

#define REGISTER_LAMBDA(func) LambdaTram::RegisterAndConvert<Counter<__COUNTER__>::value>(func)
#define REGISTER_LAMBDA_ID(ID, func) LambdaTram::RegisterAndConvert<ID>(func)

#undef T

#endif //IL2CPPHOOKER_LAMBDATRAM_HPP
