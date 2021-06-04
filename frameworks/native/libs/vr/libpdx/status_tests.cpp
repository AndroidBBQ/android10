#include <pdx/status.h>

#include <gtest/gtest.h>

#include <memory>

using android::pdx::ErrorStatus;
using android::pdx::Status;

TEST(Status, DefaultInit) {
  Status<int> status;
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(status.empty());
  EXPECT_EQ(0, status.get());
  EXPECT_EQ(0, status.error());
}

TEST(Status, InitalizeSuccess) {
  Status<int> status_int{0};
  EXPECT_FALSE(status_int.empty());
  EXPECT_TRUE(status_int.ok());
  EXPECT_EQ(0, status_int.get());
  status_int = Status<int>(3);
  EXPECT_FALSE(status_int.empty());
  EXPECT_TRUE(status_int.ok());
  EXPECT_EQ(3, status_int.get());
  status_int = Status<int>(-3);
  EXPECT_FALSE(status_int.empty());
  EXPECT_TRUE(status_int.ok());
  EXPECT_EQ(-3, status_int.get());

  Status<std::string> status_str{"foo"};
  EXPECT_FALSE(status_str.empty());
  EXPECT_TRUE(status_str.ok());
  EXPECT_EQ("foo", status_str.get());
}

TEST(Status, InitalizeError) {
  Status<int> status_int = ErrorStatus(12);
  EXPECT_FALSE(status_int.empty());
  EXPECT_FALSE(status_int.ok());
  EXPECT_EQ(0, status_int.get());
  EXPECT_EQ(12, status_int.error());

  Status<std::string> status_str = ErrorStatus(EIO);
  EXPECT_FALSE(status_str.empty());
  EXPECT_FALSE(status_str.ok());
  EXPECT_EQ(EIO, status_str.error());
}

TEST(Status, ErrorMessage) {
  Status<int> status = ErrorStatus(EIO);
  EXPECT_EQ(status.GetErrorMessage(), strerror(EIO));

  status = ErrorStatus(EINVAL);
  EXPECT_EQ(status.GetErrorMessage(), strerror(EINVAL));
}

TEST(Status, Copy) {
  Status<int> status1;
  Status<int> status2;

  status1 = Status<int>{12};
  status2 = ErrorStatus(13);
  EXPECT_FALSE(status1.empty());
  EXPECT_FALSE(status2.empty());
  EXPECT_TRUE(status1.ok());
  EXPECT_FALSE(status2.ok());
  EXPECT_EQ(12, status1.get());
  EXPECT_EQ(0, status1.error());
  EXPECT_EQ(0, status2.get());
  EXPECT_EQ(13, status2.error());

  status1 = status2;
  EXPECT_FALSE(status1.empty());
  EXPECT_FALSE(status2.empty());
  EXPECT_FALSE(status1.ok());
  EXPECT_FALSE(status2.ok());
  EXPECT_EQ(0, status1.get());
  EXPECT_EQ(13, status1.error());
  EXPECT_EQ(0, status2.get());
  EXPECT_EQ(13, status2.error());
}

TEST(Status, Move) {
  Status<std::unique_ptr<int>> status1;
  Status<std::unique_ptr<int>> status2;

  status1 = Status<std::unique_ptr<int>>{std::make_unique<int>(int{11})};
  status2 = Status<std::unique_ptr<int>>{std::make_unique<int>(int{12})};
  EXPECT_FALSE(status1.empty());
  EXPECT_FALSE(status2.empty());
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  EXPECT_EQ(11, *status1.get());
  EXPECT_EQ(12, *status2.get());

  Status<std::unique_ptr<int>> status3 = std::move(status2);
  EXPECT_FALSE(status1.empty());
  EXPECT_TRUE(status2.empty());
  EXPECT_FALSE(status3.empty());
  EXPECT_TRUE(status1.ok());
  EXPECT_FALSE(status2.ok());
  EXPECT_TRUE(status3.ok());
  EXPECT_EQ(11, *status1.get());
  EXPECT_EQ(nullptr, status2.get());
  EXPECT_EQ(12, *status3.get());

  std::swap(status1, status3);
  EXPECT_EQ(12, *status1.get());
  EXPECT_EQ(11, *status3.get());

  status3 = std::move(status1);
  EXPECT_TRUE(status1.empty());
  EXPECT_EQ(12, *status3.get());
}

TEST(Status, Take) {
  Status<std::unique_ptr<int>> status{std::make_unique<int>(int{123})};
  EXPECT_FALSE(status.empty());
  EXPECT_NE(nullptr, status.get());

  auto data = status.take();
  EXPECT_TRUE(status.empty());
  EXPECT_EQ(nullptr, status.get());
  EXPECT_EQ(123, *data);
}
