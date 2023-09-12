# LambdaTram

This class is used to register lambda function and convert it to a function pointer.
resolve the problem that Dobby can only pass function pointer as callback, but not lambda expression with captured parameters

使用编译器宏 __COUNTER__ 确定每一个不同的ID， 并使用ID对应std::function保存起来
Trampoline 根据ID去查询静态变量 static inline std::unordered_map<int, std::any> map_; 中的 function 并调用
返回对应不同的 &Trampoline<ID, Ret, Args...>

---

Using Example:

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
