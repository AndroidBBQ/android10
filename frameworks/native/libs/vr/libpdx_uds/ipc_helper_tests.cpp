#include "uds/ipc_helper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::Return;
using testing::SetErrnoAndReturn;
using testing::_;

using android::pdx::BorrowedHandle;
using android::pdx::uds::SendInterface;
using android::pdx::uds::RecvInterface;
using android::pdx::uds::SendAll;
using android::pdx::uds::SendMsgAll;
using android::pdx::uds::RecvAll;
using android::pdx::uds::RecvMsgAll;

namespace {

// Useful constants for tests.
static constexpr intptr_t kPtr = 1234;
static constexpr int kSocketFd = 5678;
static const BorrowedHandle kSocket{kSocketFd};

// Helper functions to construct test data pointer values.
void* IntToPtr(intptr_t value) { return reinterpret_cast<void*>(value); }
const void* IntToConstPtr(intptr_t value) {
  return reinterpret_cast<const void*>(value);
}

// Mock classes for SendInterface/RecvInterface.
class MockSender : public SendInterface {
 public:
  MOCK_METHOD4(Send, ssize_t(int socket_fd, const void* data, size_t size,
                             int flags));
  MOCK_METHOD3(SendMessage,
               ssize_t(int socket_fd, const msghdr* msg, int flags));
};

class MockReceiver : public RecvInterface {
 public:
  MOCK_METHOD4(Receive,
               ssize_t(int socket_fd, void* data, size_t size, int flags));
  MOCK_METHOD3(ReceiveMessage, ssize_t(int socket_fd, msghdr* msg, int flags));
};

// Test case classes.
class SendTest : public testing::Test {
 public:
  SendTest() {
    ON_CALL(sender_, Send(_, _, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
    ON_CALL(sender_, SendMessage(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
  }

 protected:
  MockSender sender_;
};

class RecvTest : public testing::Test {
 public:
  RecvTest() {
    ON_CALL(receiver_, Receive(_, _, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
    ON_CALL(receiver_, ReceiveMessage(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
  }

 protected:
  MockReceiver receiver_;
};

class MessageTestBase : public testing::Test {
 public:
  MessageTestBase() {
    memset(&msg_, 0, sizeof(msg_));
    msg_.msg_iovlen = data_.size();
    msg_.msg_iov = data_.data();
  }

 protected:
  static constexpr intptr_t kPtr1 = kPtr;
  static constexpr intptr_t kPtr2 = kPtr + 200;
  static constexpr intptr_t kPtr3 = kPtr + 1000;

  MockSender sender_;
  msghdr msg_;
  std::vector<iovec> data_{
      {IntToPtr(kPtr1), 100}, {IntToPtr(kPtr2), 200}, {IntToPtr(kPtr3), 300}};
};

class SendMessageTest : public MessageTestBase {
 public:
  SendMessageTest() {
    ON_CALL(sender_, Send(_, _, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
    ON_CALL(sender_, SendMessage(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
  }

 protected:
  MockSender sender_;
};

class RecvMessageTest : public MessageTestBase {
 public:
  RecvMessageTest() {
    ON_CALL(receiver_, Receive(_, _, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
    ON_CALL(receiver_, ReceiveMessage(_, _, _))
        .WillByDefault(SetErrnoAndReturn(EIO, -1));
  }

 protected:
  MockReceiver receiver_;
};

// Actual tests.

// SendAll
TEST_F(SendTest, Complete) {
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr), 100, MSG_NOSIGNAL))
      .WillOnce(Return(100));

  auto status = SendAll(&sender_, kSocket, IntToConstPtr(kPtr), 100);
  EXPECT_TRUE(status);
}

TEST_F(SendTest, Signal) {
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr), 100, MSG_NOSIGNAL))
      .WillOnce(Return(20));
  EXPECT_CALL(sender_,
              Send(kSocketFd, IntToConstPtr(kPtr + 20), 80, MSG_NOSIGNAL))
      .WillOnce(Return(40));
  EXPECT_CALL(sender_,
              Send(kSocketFd, IntToConstPtr(kPtr + 60), 40, MSG_NOSIGNAL))
      .WillOnce(Return(40));

  auto status = SendAll(&sender_, kSocket, IntToConstPtr(kPtr), 100);
  EXPECT_TRUE(status);
}

TEST_F(SendTest, Eintr) {
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr), 100, MSG_NOSIGNAL))
      .WillOnce(SetErrnoAndReturn(EINTR, -1))
      .WillOnce(Return(100));

  auto status = SendAll(&sender_, kSocket, IntToConstPtr(kPtr), 100);
  EXPECT_TRUE(status);
}

TEST_F(SendTest, Error) {
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr), 100, MSG_NOSIGNAL))
      .WillOnce(SetErrnoAndReturn(EIO, -1));

  auto status = SendAll(&sender_, kSocket, IntToConstPtr(kPtr), 100);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
}

TEST_F(SendTest, Error2) {
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr), 100, MSG_NOSIGNAL))
      .WillOnce(Return(50));
  EXPECT_CALL(sender_,
              Send(kSocketFd, IntToConstPtr(kPtr + 50), 50, MSG_NOSIGNAL))
      .WillOnce(SetErrnoAndReturn(EIO, -1));

  auto status = SendAll(&sender_, kSocket, IntToConstPtr(kPtr), 100);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
}

// RecvAll
TEST_F(RecvTest, Complete) {
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr), 100,
                                 MSG_WAITALL | MSG_CMSG_CLOEXEC))
      .WillOnce(Return(100));

  auto status = RecvAll(&receiver_, kSocket, IntToPtr(kPtr), 100);
  EXPECT_TRUE(status);
}

TEST_F(RecvTest, Signal) {
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr), 100, _))
      .WillOnce(Return(20));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr + 20), 80, _))
      .WillOnce(Return(40));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr + 60), 40, _))
      .WillOnce(Return(40));

  auto status = RecvAll(&receiver_, kSocket, IntToPtr(kPtr), 100);
  EXPECT_TRUE(status);
}

TEST_F(RecvTest, Eintr) {
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr), 100, _))
      .WillOnce(SetErrnoAndReturn(EINTR, -1))
      .WillOnce(Return(100));

  auto status = RecvAll(&receiver_, kSocket, IntToPtr(kPtr), 100);
  EXPECT_TRUE(status);
}

TEST_F(RecvTest, Error) {
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr), 100, _))
      .WillOnce(SetErrnoAndReturn(EIO, -1));

  auto status = RecvAll(&receiver_, kSocket, IntToPtr(kPtr), 100);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
}

TEST_F(RecvTest, Error2) {
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr), 100, _))
      .WillOnce(Return(30));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr + 30), 70, _))
      .WillOnce(SetErrnoAndReturn(EIO, -1));

  auto status = RecvAll(&receiver_, kSocket, IntToPtr(kPtr), 100);
  ASSERT_FALSE(status);
  EXPECT_EQ(EIO, status.error());
}

// SendMsgAll
TEST_F(SendMessageTest, Complete) {
  EXPECT_CALL(sender_, SendMessage(kSocketFd, &msg_, MSG_NOSIGNAL))
      .WillOnce(Return(600));

  auto status = SendMsgAll(&sender_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(SendMessageTest, Partial) {
  EXPECT_CALL(sender_, SendMessage(kSocketFd, &msg_, _)).WillOnce(Return(70));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr1 + 70), 30, _))
      .WillOnce(Return(30));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr2), 200, _))
      .WillOnce(Return(190));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr2 + 190), 10, _))
      .WillOnce(Return(10));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr3), 300, _))
      .WillOnce(Return(300));

  auto status = SendMsgAll(&sender_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(SendMessageTest, Partial2) {
  EXPECT_CALL(sender_, SendMessage(kSocketFd, &msg_, _)).WillOnce(Return(310));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr3 + 10), 290, _))
      .WillOnce(Return(290));

  auto status = SendMsgAll(&sender_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(SendMessageTest, Eintr) {
  EXPECT_CALL(sender_, SendMessage(kSocketFd, &msg_, _))
      .WillOnce(SetErrnoAndReturn(EINTR, -1))
      .WillOnce(Return(70));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr1 + 70), 30, _))
      .WillOnce(SetErrnoAndReturn(EINTR, -1))
      .WillOnce(Return(30));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr2), 200, _))
      .WillOnce(Return(200));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr3), 300, _))
      .WillOnce(Return(300));

  auto status = SendMsgAll(&sender_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(SendMessageTest, Error) {
  EXPECT_CALL(sender_, SendMessage(kSocketFd, &msg_, _))
      .WillOnce(SetErrnoAndReturn(EBADF, -1));

  auto status = SendMsgAll(&sender_, kSocket, &msg_);
  ASSERT_FALSE(status);
  EXPECT_EQ(EBADF, status.error());
}

TEST_F(SendMessageTest, Error2) {
  EXPECT_CALL(sender_, SendMessage(kSocketFd, &msg_, _)).WillOnce(Return(20));
  EXPECT_CALL(sender_, Send(kSocketFd, IntToConstPtr(kPtr1 + 20), 80, _))
      .WillOnce(SetErrnoAndReturn(EBADF, -1));

  auto status = SendMsgAll(&sender_, kSocket, &msg_);
  ASSERT_FALSE(status);
  EXPECT_EQ(EBADF, status.error());
}

// RecvMsgAll
TEST_F(RecvMessageTest, Complete) {
  EXPECT_CALL(receiver_,
              ReceiveMessage(kSocketFd, &msg_, MSG_WAITALL | MSG_CMSG_CLOEXEC))
      .WillOnce(Return(600));

  auto status = RecvMsgAll(&receiver_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(RecvMessageTest, Partial) {
  EXPECT_CALL(receiver_, ReceiveMessage(kSocketFd, &msg_, _))
      .WillOnce(Return(70));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr1 + 70), 30, _))
      .WillOnce(Return(30));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr2), 200, _))
      .WillOnce(Return(190));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr2 + 190), 10, _))
      .WillOnce(Return(10));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr3), 300, _))
      .WillOnce(Return(300));

  auto status = RecvMsgAll(&receiver_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(RecvMessageTest, Partial2) {
  EXPECT_CALL(receiver_, ReceiveMessage(kSocketFd, &msg_, _))
      .WillOnce(Return(310));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr3 + 10), 290, _))
      .WillOnce(Return(290));

  auto status = RecvMsgAll(&receiver_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(RecvMessageTest, Eintr) {
  EXPECT_CALL(receiver_, ReceiveMessage(kSocketFd, &msg_, _))
      .WillOnce(SetErrnoAndReturn(EINTR, -1))
      .WillOnce(Return(70));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr1 + 70), 30, _))
      .WillOnce(SetErrnoAndReturn(EINTR, -1))
      .WillOnce(Return(30));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr2), 200, _))
      .WillOnce(Return(200));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr3), 300, _))
      .WillOnce(Return(300));

  auto status = RecvMsgAll(&receiver_, kSocket, &msg_);
  EXPECT_TRUE(status);
}

TEST_F(RecvMessageTest, Error) {
  EXPECT_CALL(receiver_, ReceiveMessage(kSocketFd, &msg_, _))
      .WillOnce(SetErrnoAndReturn(EBADF, -1));

  auto status = RecvMsgAll(&receiver_, kSocket, &msg_);
  ASSERT_FALSE(status);
  EXPECT_EQ(EBADF, status.error());
}

TEST_F(RecvMessageTest, Error2) {
  EXPECT_CALL(receiver_, ReceiveMessage(kSocketFd, &msg_, _))
      .WillOnce(Return(20));
  EXPECT_CALL(receiver_, Receive(kSocketFd, IntToPtr(kPtr1 + 20), 80, _))
      .WillOnce(SetErrnoAndReturn(EBADF, -1));

  auto status = RecvMsgAll(&receiver_, kSocket, &msg_);
  ASSERT_FALSE(status);
  EXPECT_EQ(EBADF, status.error());
}

}  // namespace
