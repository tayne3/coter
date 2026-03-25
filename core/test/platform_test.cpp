#include "coter/core/platform.h"

#include <catch.hpp>
#include <string>
#include <thread>
#include <vector>

TEST_CASE("offset_of") {
    struct TestStruct {
        char   a;
        double b;
        int    c;
        char   d[4];
        float  e;
    };

    const size_t offset_a = OFFSET_OF(TestStruct, a);
    REQUIRE(offset_a == 0);

    const size_t expected_b = (sizeof(char) + alignof(double) - 1) & ~(alignof(double) - 1);
    const size_t offset_b   = OFFSET_OF(TestStruct, b);
    REQUIRE(offset_b == expected_b);

    const size_t expected_c = (offset_b + sizeof(double) + alignof(int) - 1) & ~(alignof(int) - 1);
    const size_t offset_c   = OFFSET_OF(TestStruct, c);
    REQUIRE(offset_c == expected_c);

    const size_t expected_d = (offset_c + sizeof(int) + alignof(char) - 1) & ~(alignof(char) - 1);
    const size_t offset_d   = OFFSET_OF(TestStruct, d);
    REQUIRE(offset_d == expected_d);

    const size_t expected_e = (offset_d + sizeof(char[4]) + alignof(float) - 1) & ~(alignof(float) - 1);
    const size_t offset_e   = OFFSET_OF(TestStruct, e);
    REQUIRE(offset_e == expected_e);

    const size_t struct_size   = sizeof(TestStruct);
    const size_t expected_size = (offset_e + sizeof(float) + alignof(TestStruct) - 1) & ~(alignof(TestStruct) - 1);
    REQUIRE(struct_size == expected_size);

    const size_t offset_d2_expected = offset_d + 2 * sizeof(char);
    REQUIRE(offset_d2_expected == offset_d + 2);
}

TEST_CASE("container_of") {
    struct TestStruct {
        double a;
        int    b;
        char   c[4];
        float  d;
    };

    TestStruct test_instance1{1.0, 2, {'a', 'b', 'c', 'd'}, 3.14f};
    auto*      container_ptr1 = CONTAINER_OF(&test_instance1.c, TestStruct, c);
    REQUIRE(container_ptr1 == &test_instance1);

    auto* container_ptr2 = CONTAINER_OF(&test_instance1.b, TestStruct, b);
    REQUIRE(container_ptr2 == &test_instance1);

    auto* container_ptr3 = CONTAINER_OF(&test_instance1.c[0], TestStruct, c);
    REQUIRE(container_ptr3 == &test_instance1);

    REQUIRE(container_ptr1->a == Approx(1.0));
    REQUIRE(container_ptr1->b == 2);
    REQUIRE(container_ptr1->c[0] == 'a');
    REQUIRE(container_ptr1->d == Approx(3.14f));

    TestStruct test_array[3] = {
        {1.1, 11, {'w', 'x', 'y', 'z'}, 1.1f},
        {2.2, 22, {'a', 'b', 'c', 'd'}, 2.2f},
        {3.3, 33, {'m', 'n', 'o', 'p'}, 3.3f},
    };
    auto* container_ptr5 = CONTAINER_OF(&test_array[1].c, TestStruct, c);
    REQUIRE(container_ptr5 == &test_array[1]);
}

TEST_CASE("pid_positive") {
    const int pid = ct_getpid();
    REQUIRE(pid > 0);
}

TEST_CASE("pid_consistent_concurrent", "[concurrency]") {
    const int                pid     = ct_getpid();
    const int                threads = 4;
    const int                loops   = 50;
    std::vector<std::thread> ts;
    ts.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        ts.emplace_back([&]() {
            for (int i = 0; i < loops; ++i) { REQUIRE(ct_getpid() == pid); }
        });
    }
    for (auto& th : ts) th.join();
}

TEST_CASE("mkdir_existing_returns_error", "[fs]") {
    const int         pid  = ct_getpid();
    const ct_time64_t salt = ct_getuptime_ms();
    std::string       dir  = std::string("ct_tmp_exist_") + std::to_string(pid) + "_" + std::to_string((long long)salt);
    int               mkret1 = ct_mkdir(dir.c_str());
    const bool        mkok1  = (mkret1 == 0) || (mkret1 == -1);
    REQUIRE(mkok1);
    int mkret2 = ct_mkdir(dir.c_str());
    REQUIRE(mkret2 == -1);
    ct_rmdir(dir.c_str());
}

TEST_CASE("fs_stat_and_mkdir", "[fs]") {
    const int         pid   = ct_getpid();
    const ct_time64_t salt  = ct_getuptime_ms();
    std::string       dir   = std::string("ct_tmp_") + std::to_string(pid) + "_" + std::to_string((long long)salt);
    int               mkret = ct_mkdir(dir.c_str());
    const bool        mkok  = (mkret == 0) || (mkret == -1);
    REQUIRE(mkok);
    std::string file = dir + STR_SEPARATOR "test.txt";
    FILE*       fp   = fopen(file.c_str(), "wb");
    REQUIRE(fp != nullptr);
    const char* msg = "hello";
    REQUIRE(fwrite(msg, 1, 5, fp) == 5);
    fflush(fp);
    const int fd = ct_fileno(fp);
    ct_stat_t stf{};
    REQUIRE(ct_fstat(fd, &stf) == 0);
    REQUIRE(S_ISREG(stf.st_mode));
    REQUIRE((size_t)stf.st_size == 5);
    fclose(fp);
    ct_stat_t st{};
    REQUIRE(ct_stat(file.c_str(), &st) == 0);
    REQUIRE(S_ISREG(st.st_mode));
    REQUIRE((size_t)st.st_size == 5);
    ct_stat_t std{};
    REQUIRE(ct_stat(dir.c_str(), &std) == 0);
    REQUIRE(S_ISDIR(std.st_mode));
    ct_remove(file.c_str());
    ct_rmdir(dir.c_str());
}

TEST_CASE("access_permissions", "[fs]") {
    const int         pid  = ct_getpid();
    const ct_time64_t salt = ct_getuptime_ms();
    std::string       dir = std::string("ct_tmp_access_") + std::to_string(pid) + "_" + std::to_string((long long)salt);
    int               mkret = ct_mkdir(dir.c_str());
    const bool        mkok  = (mkret == 0) || (mkret == -1);
    REQUIRE(mkok);
    std::string file = dir + STR_SEPARATOR "perm.txt";

    FILE* fp = fopen(file.c_str(), "wb");
    REQUIRE(fp != nullptr);
    const char* msg = "abc";
    REQUIRE(fwrite(msg, 1, 3, fp) == 3);
    fclose(fp);

    REQUIRE(ct_access(dir.c_str(), F_OK) == 0);
    REQUIRE(ct_access(file.c_str(), F_OK) == 0);
    REQUIRE(ct_access(file.c_str(), R_OK) == 0);
    REQUIRE(ct_access(file.c_str(), W_OK) == 0);
    REQUIRE(ct_access(file.c_str(), R_OK | W_OK) == 0);

#ifdef CT_OS_WIN
    REQUIRE(ct_access(file.c_str(), X_OK) == 0);
#else
    REQUIRE(ct_access(file.c_str(), X_OK) == -1);
    REQUIRE(chmod(file.c_str(), 0755) == 0);
    REQUIRE(ct_access(file.c_str(), X_OK) == 0);
#endif

    REQUIRE(ct_access("ct_missing_access_test", F_OK) == -1);

    ct_remove(file.c_str());
    ct_rmdir(dir.c_str());
}
