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
#include <functional>
#include <unordered_map>
#include <tuple>
#include <any>
#include <android/log.h>

using namespace std;

template<int N>
struct Counter {
    static const int value = N;
};

class LambdaTram {
public:
    template<int ID, typename Ret, typename ...Args>
    static void* RegisterAndConvert(std::function<Ret(Args...)> func) {
        std::lock_guard<std::mutex> lock(map_mutex_);
        map_[ID] = func;
        return reinterpret_cast<void*>(&Trampoline<ID, Ret, Args...>);
    }

    template<int ID, typename Callable>
    static void* RegisterAndConvert(Callable&& func) {
        return RegisterAndConvert<ID>(std::function(func));
    }

    template<int ID, typename Ret, typename ...Args>
    static Ret Execute(Args... args) {
        std::lock_guard<std::mutex> lock(map_mutex_);
        auto it = map_.find(ID);
        if (it != map_.end()) {
            auto actual_lambda = std::any_cast<std::function<Ret(Args...)>>(it->second);
            return actual_lambda(args...);
        }
        return reinterpret_cast<Ret>(nullptr);
    }

    template<int ID>
    static void Unregister() {
        std::lock_guard<std::mutex> lock(map_mutex_);
        map_.erase(ID);
    }

private:
    template<int ID, typename Ret, typename... Args>
    static Ret Trampoline(Args... args) {
        auto it = map_.find(ID);
        try {
            if (it != map_.end()) {
                auto func = std::any_cast<std::function<Ret(Args...)>>(it->second);
                return func(args...);
            }
            return Ret();
        } catch (const std::bad_any_cast& e) {
            __android_log_print(ANDROID_LOG_ERROR, "LambdaTram", "bad_any_cast: %s", e.what());
            return nullptr;
        }
    }

private:
    static inline std::unordered_map<int, std::any> map_;
    static inline std::mutex map_mutex_;
};

template<typename T>
struct FunctionTraits;

template<typename Ret, typename... Args>
struct FunctionTraits<std::function<Ret(Args...)>> {
    using PointerType = Ret(*)(Args...);
};

#define DEFINE_LAMBDA_TYPE(func) \
    using LambdaType = FunctionTraits<decltype(func)>::PointerType; \

#define REGISTER_LAMBDA(func) \
    LambdaTram::RegisterAndConvert<Counter<__COUNTER__>::value>(func); \
    // DEFINE_LAMBDA_TYPE(func) \

#define UNREGISTER_LAMBDA() \
    LambdaTram::Unregister<Counter<__COUNTER__>::value>() \

#endif //IL2CPPHOOKER_LAMBDATRAM_HPP
