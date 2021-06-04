
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include <cputimeinstate.h>

namespace android {
namespace bpf {

using std::vector;

TEST(TimeInStateTest, SingleUid) {
    vector<vector<uint64_t>> times;
    ASSERT_TRUE(getUidCpuFreqTimes(0, &times));
    EXPECT_FALSE(times.empty());
}

TEST(TimeInStateTest, AllUid) {
    vector<size_t> sizes;
    std::unordered_map<uint32_t, vector<vector<uint64_t>>> map;
    ASSERT_TRUE(getUidsCpuFreqTimes(&map));

    ASSERT_FALSE(map.empty());

    auto firstEntry = map.begin()->second;
    for (const auto &subEntry : firstEntry) sizes.emplace_back(subEntry.size());

    for (const auto &vec : map) {
        ASSERT_EQ(vec.second.size(), sizes.size());
        for (size_t i = 0; i < vec.second.size(); ++i) ASSERT_EQ(vec.second[i].size(), sizes[i]);
    }
}

TEST(TimeInStateTest, RemoveUid) {
    vector<vector<uint64_t>> times, times2;
    ASSERT_TRUE(getUidCpuFreqTimes(0, &times));
    ASSERT_FALSE(times.empty());

    uint64_t sum = 0;
    for (size_t i = 0; i < times.size(); ++i) {
        for (auto x : times[i]) sum += x;
    }
    ASSERT_GT(sum, (uint64_t)0);

    ASSERT_TRUE(clearUidCpuFreqTimes(0));

    ASSERT_TRUE(getUidCpuFreqTimes(0, &times2));
    ASSERT_EQ(times2.size(), times.size());
    for (size_t i = 0; i < times.size(); ++i) {
        ASSERT_EQ(times2[i].size(), times[i].size());
        for (size_t j = 0; j < times[i].size(); ++j) ASSERT_LE(times2[i][j], times[i][j]);
    }
}

} // namespace bpf
} // namespace android
