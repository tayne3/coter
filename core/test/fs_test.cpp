#include "coter/core/fs.h"

#include <string>
#include <thread>
#include <vector>

#include "coter/core/time.h"
#include "coter/testing/doctest.h"


TEST_CASE("pid_positive") {
    const int pid = ct_getpid();
    REQUIRE(pid > 0);
}

TEST_CASE("pid_consistent_concurrent" * doctest::test_suite("concurrency")) {
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

TEST_CASE("mkdir_existing_returns_error" * doctest::test_suite("fs")) {
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

TEST_CASE("fs_stat_and_mkdir" * doctest::test_suite("fs")) {
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

TEST_CASE("access_permissions" * doctest::test_suite("fs")) {
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
