#pragma once
// Minimal self-registering test harness (no external dependency).

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace testing {

struct TestCase {
    const char* name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}
inline int& failures() {
    static int f = 0;
    return f;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) { registry().push_back({name, std::move(fn)}); }
};

inline void fail(const char* file, int line, const std::string& what) {
    std::printf("    FAILED %s:%d  %s\n", file, line, what.c_str());
    ++failures();
}

} // namespace testing

#define TEST_CONCAT2(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT2(a, b)
#define TEST(name)                                                                  \
    static void name();                                                             \
    static testing::Registrar TEST_CONCAT(reg_, name)(#name, name);                 \
    static void name()

#define CHECK(cond)                                                                 \
    do {                                                                            \
        if (!(cond)) testing::fail(__FILE__, __LINE__, #cond);                      \
    } while (0)

#define CHECK_NEAR(a, b, tol)                                                       \
    do {                                                                            \
        const double va_ = (a), vb_ = (b);                                          \
        if (!(std::fabs(va_ - vb_) <= (tol)))                                       \
            testing::fail(__FILE__, __LINE__,                                       \
                          std::string(#a " ~ " #b ": ") + std::to_string(va_) +     \
                              " vs " + std::to_string(vb_));                        \
    } while (0)
