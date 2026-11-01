#include "test_harness.h"

int main() {
    int failedTests = 0;
    for (const testing::TestCase& t : testing::registry()) {
        const int before = testing::failures();
        t.fn();
        const bool ok = testing::failures() == before;
        if (!ok) ++failedTests;
        std::printf("[%s] %s\n", ok ? " ok " : "FAIL", t.name);
    }
    std::printf("\n%zu tests, %d failed, %d failed checks\n", testing::registry().size(), failedTests,
                testing::failures());
    return failedTests == 0 ? 0 : 1;
}
