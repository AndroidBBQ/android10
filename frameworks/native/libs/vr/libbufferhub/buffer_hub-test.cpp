#include <gtest/gtest.h>
#include <poll.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/consumer_buffer.h>
#include <private/dvr/producer_buffer.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <ui/BufferHubDefs.h>

#include <mutex>
#include <thread>

namespace {
#define RETRY_EINTR(fnc_call)                 \
  ([&]() -> decltype(fnc_call) {              \
    decltype(fnc_call) result;                \
    do {                                      \
      result = (fnc_call);                    \
    } while (result == -1 && errno == EINTR); \
    return result;                            \
  })()

using android::BufferHubDefs::isAnyClientAcquired;
using android::BufferHubDefs::isAnyClientGained;
using android::BufferHubDefs::isAnyClientPosted;
using android::BufferHubDefs::isClientAcquired;
using android::BufferHubDefs::isClientPosted;
using android::BufferHubDefs::isClientReleased;
using android::BufferHubDefs::kFirstClientBitMask;
using android::dvr::ConsumerBuffer;
using android::dvr::ProducerBuffer;
using android::pdx::LocalHandle;
using android::pdx::Status;
using LibBufferHubTest = ::testing::Test;

const int kWidth = 640;
const int kHeight = 480;
const int kFormat = HAL_PIXEL_FORMAT_RGBA_8888;
const int kUsage = 0;
// Maximum number of consumers for the buffer that only has one producer in the
// test.
const size_t kMaxConsumerCount =
    android::BufferHubDefs::kMaxNumberOfClients - 1;
const int kPollTimeoutMs = 100;

// Helper function to poll the eventfd in BufferHubBase.
template <class BufferHubBase>
int PollBufferEvent(const std::unique_ptr<BufferHubBase>& buffer,
                    int timeout_ms = kPollTimeoutMs) {
  pollfd p = {buffer->event_fd(), POLLIN, 0};
  return poll(&p, 1, timeout_ms);
}

}  // namespace

TEST_F(LibBufferHubTest, TestBasicUsage) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c1 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c1.get() != nullptr);
  // Check that consumers can spawn other consumers.
  std::unique_ptr<ConsumerBuffer> c2 =
      ConsumerBuffer::Import(c1->CreateConsumer());
  ASSERT_TRUE(c2.get() != nullptr);

  // Checks the state masks of client p, c1 and c2.
  EXPECT_EQ(p->client_state_mask(), kFirstClientBitMask);
  EXPECT_EQ(c1->client_state_mask(), kFirstClientBitMask << 1);
  EXPECT_EQ(c2->client_state_mask(), kFirstClientBitMask << 2);

  // Initial state: producer not available, consumers not available.
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c1)));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c2)));

  EXPECT_EQ(0, p->GainAsync());
  EXPECT_EQ(0, p->Post(LocalHandle()));

  // New state: producer not available, consumers available.
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(1, RETRY_EINTR(PollBufferEvent(c1)));
  EXPECT_EQ(1, RETRY_EINTR(PollBufferEvent(c2)));

  LocalHandle fence;
  EXPECT_EQ(0, c1->Acquire(&fence));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c1)));
  EXPECT_EQ(1, RETRY_EINTR(PollBufferEvent(c2)));

  EXPECT_EQ(0, c2->Acquire(&fence));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c2)));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c1)));

  EXPECT_EQ(0, c1->Release(LocalHandle()));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(0, c2->Discard());
  EXPECT_EQ(1, RETRY_EINTR(PollBufferEvent(p)));

  EXPECT_EQ(0, p->Gain(&fence));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c1)));
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c2)));
}

TEST_F(LibBufferHubTest, TestEpoll) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);

  LocalHandle epoll_fd{epoll_create1(EPOLL_CLOEXEC)};
  ASSERT_TRUE(epoll_fd.IsValid());

  epoll_event event;
  std::array<epoll_event, 64> events;

  auto event_sources = p->GetEventSources();
  ASSERT_LT(event_sources.size(), events.size());

  for (const auto& event_source : event_sources) {
    event = {.events = event_source.event_mask | EPOLLET,
             .data = {.fd = p->event_fd()}};
    ASSERT_EQ(0, epoll_ctl(epoll_fd.Get(), EPOLL_CTL_ADD, event_source.event_fd,
                           &event));
  }

  event_sources = c->GetEventSources();
  ASSERT_LT(event_sources.size(), events.size());

  for (const auto& event_source : event_sources) {
    event = {.events = event_source.event_mask | EPOLLET,
             .data = {.fd = c->event_fd()}};
    ASSERT_EQ(0, epoll_ctl(epoll_fd.Get(), EPOLL_CTL_ADD, event_source.event_fd,
                           &event));
  }

  // No events should be signaled initially.
  ASSERT_EQ(0, epoll_wait(epoll_fd.Get(), events.data(), events.size(), 0));

  // Gain and post the producer and check for consumer signal.
  EXPECT_EQ(0, p->GainAsync());
  EXPECT_EQ(0, p->Post({}));
  ASSERT_EQ(1, epoll_wait(epoll_fd.Get(), events.data(), events.size(),
                          kPollTimeoutMs));
  ASSERT_TRUE(events[0].events & EPOLLIN);
  ASSERT_EQ(c->event_fd(), events[0].data.fd);

  // Save the event bits to translate later.
  event = events[0];

  // Check for events again. Edge-triggered mode should prevent any.
  EXPECT_EQ(0, epoll_wait(epoll_fd.Get(), events.data(), events.size(),
                          kPollTimeoutMs));
  EXPECT_EQ(0, epoll_wait(epoll_fd.Get(), events.data(), events.size(),
                          kPollTimeoutMs));
  EXPECT_EQ(0, epoll_wait(epoll_fd.Get(), events.data(), events.size(),
                          kPollTimeoutMs));
  EXPECT_EQ(0, epoll_wait(epoll_fd.Get(), events.data(), events.size(),
                          kPollTimeoutMs));

  // Translate the events.
  auto event_status = c->GetEventMask(event.events);
  ASSERT_TRUE(event_status);
  ASSERT_TRUE(event_status.get() & EPOLLIN);

  // Check for events again. Edge-triggered mode should prevent any.
  EXPECT_EQ(0, epoll_wait(epoll_fd.Get(), events.data(), events.size(),
                          kPollTimeoutMs));
}

TEST_F(LibBufferHubTest, TestStateMask) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);

  // It's ok to create up to kMaxConsumerCount consumer buffers.
  uint32_t client_state_masks = p->client_state_mask();
  std::array<std::unique_ptr<ConsumerBuffer>, kMaxConsumerCount> cs;
  for (size_t i = 0; i < kMaxConsumerCount; i++) {
    cs[i] = ConsumerBuffer::Import(p->CreateConsumer());
    ASSERT_TRUE(cs[i].get() != nullptr);
    // Expect all buffers have unique state mask.
    EXPECT_EQ(client_state_masks & cs[i]->client_state_mask(), 0U);
    client_state_masks |= cs[i]->client_state_mask();
  }
  EXPECT_EQ(client_state_masks, ~0U);

  // The 64th creation will fail with out-of-memory error.
  auto state = p->CreateConsumer();
  EXPECT_EQ(state.error(), E2BIG);

  // Release any consumer should allow us to re-create.
  for (size_t i = 0; i < kMaxConsumerCount; i++) {
    client_state_masks &= ~cs[i]->client_state_mask();
    cs[i] = nullptr;
    cs[i] = ConsumerBuffer::Import(p->CreateConsumer());
    ASSERT_TRUE(cs[i].get() != nullptr);
    // The released state mask will be reused.
    EXPECT_EQ(client_state_masks & cs[i]->client_state_mask(), 0U);
    client_state_masks |= cs[i]->client_state_mask();
  }
}

TEST_F(LibBufferHubTest, TestStateTransitions) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);

  LocalHandle fence;
  EXPECT_EQ(0, p->GainAsync());

  // Acquire in gained state should fail.
  EXPECT_EQ(-EBUSY, c->Acquire(&fence));

  // Post in gained state should succeed.
  EXPECT_EQ(0, p->Post(LocalHandle()));

  // Post and gain in posted state should fail.
  EXPECT_EQ(-EBUSY, p->Post(LocalHandle()));
  EXPECT_EQ(-EBUSY, p->Gain(&fence));

  // Acquire in posted state should succeed.
  EXPECT_EQ(0, c->Acquire(&fence));

  // Acquire, post, and gain in acquired state should fail.
  EXPECT_EQ(-EBUSY, c->Acquire(&fence));
  EXPECT_EQ(-EBUSY, p->Post(LocalHandle()));
  EXPECT_EQ(-EBUSY, p->Gain(&fence));

  // Release in acquired state should succeed.
  EXPECT_EQ(0, c->Release(LocalHandle()));
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));

  // Acquire and post in released state should fail.
  EXPECT_EQ(-EBUSY, c->Acquire(&fence));
  EXPECT_EQ(-EBUSY, p->Post(LocalHandle()));

  // Gain in released state should succeed.
  EXPECT_EQ(0, p->Gain(&fence));

  // Acquire in gained state should fail.
  EXPECT_EQ(-EBUSY, c->Acquire(&fence));
}

TEST_F(LibBufferHubTest, TestAsyncStateTransitions) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);

  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;
  EXPECT_EQ(0, p->GainAsync());

  // Acquire in gained state should fail.
  EXPECT_EQ(-EBUSY, c->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());
  EXPECT_FALSE(invalid_fence.IsValid());

  // Post in gained state should succeed.
  EXPECT_EQ(0, p->PostAsync(&metadata, invalid_fence));
  EXPECT_EQ(p->buffer_state(), c->buffer_state());
  EXPECT_TRUE(isAnyClientPosted(p->buffer_state()));

  // Post and gain in posted state should fail.
  EXPECT_EQ(-EBUSY, p->PostAsync(&metadata, invalid_fence));
  EXPECT_EQ(-EBUSY, p->GainAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());

  // Acquire in posted state should succeed.
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c)));
  EXPECT_EQ(0, c->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());
  EXPECT_EQ(p->buffer_state(), c->buffer_state());
  EXPECT_TRUE(isAnyClientAcquired(p->buffer_state()));

  // Acquire, post, and gain in acquired state should fail.
  EXPECT_EQ(-EBUSY, c->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());
  EXPECT_EQ(-EBUSY, p->PostAsync(&metadata, invalid_fence));
  EXPECT_EQ(-EBUSY, p->GainAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());

  // Release in acquired state should succeed.
  EXPECT_EQ(0, c->ReleaseAsync(&metadata, invalid_fence));
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(p->buffer_state(), c->buffer_state());
  EXPECT_TRUE(p->is_released());

  // Acquire and post in released state should fail.
  EXPECT_EQ(-EBUSY, c->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());
  EXPECT_EQ(-EBUSY, p->PostAsync(&metadata, invalid_fence));

  // Gain in released state should succeed.
  EXPECT_EQ(0, p->GainAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());
  EXPECT_EQ(p->buffer_state(), c->buffer_state());
  EXPECT_TRUE(isAnyClientGained(p->buffer_state()));

  // Acquire and gain in gained state should fail.
  EXPECT_EQ(-EBUSY, c->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_FALSE(invalid_fence.IsValid());
}

TEST_F(LibBufferHubTest, TestGainTwiceByTheSameProducer) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);

  ASSERT_EQ(0, p->GainAsync());
  ASSERT_EQ(0, p->GainAsync());
}

TEST_F(LibBufferHubTest, TestGainPostedBuffer) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  ASSERT_EQ(0, p->GainAsync());
  ASSERT_EQ(0, p->Post(LocalHandle()));
  ASSERT_TRUE(isAnyClientPosted(p->buffer_state()));

  // Gain in posted state should only succeed with gain_posted_buffer = true.
  LocalHandle invalid_fence;
  EXPECT_EQ(-EBUSY, p->Gain(&invalid_fence, false));
  EXPECT_EQ(0, p->Gain(&invalid_fence, true));
}

TEST_F(LibBufferHubTest, TestGainPostedBufferAsync) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  ASSERT_EQ(0, p->GainAsync());
  ASSERT_EQ(0, p->Post(LocalHandle()));
  ASSERT_TRUE(isAnyClientPosted(p->buffer_state()));

  // GainAsync in posted state should only succeed with gain_posted_buffer
  // equals true.
  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;
  EXPECT_EQ(-EBUSY, p->GainAsync(&metadata, &invalid_fence, false));
  EXPECT_EQ(0, p->GainAsync(&metadata, &invalid_fence, true));
}

TEST_F(LibBufferHubTest, TestGainPostedBuffer_noConsumer) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  ASSERT_EQ(0, p->GainAsync());
  ASSERT_EQ(0, p->Post(LocalHandle()));
  // Producer state bit is in released state after post, other clients shall be
  // in posted state although there is no consumer of this buffer yet.
  ASSERT_TRUE(isClientReleased(p->buffer_state(), p->client_state_mask()));
  ASSERT_TRUE(p->is_released());
  ASSERT_TRUE(isAnyClientPosted(p->buffer_state()));

  // Gain in released state should succeed.
  LocalHandle invalid_fence;
  EXPECT_EQ(0, p->Gain(&invalid_fence, false));
}

TEST_F(LibBufferHubTest, TestMaxConsumers) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  uint32_t producer_state_mask = p->client_state_mask();

  std::array<std::unique_ptr<ConsumerBuffer>, kMaxConsumerCount> cs;
  for (size_t i = 0; i < kMaxConsumerCount; ++i) {
    cs[i] = ConsumerBuffer::Import(p->CreateConsumer());
    ASSERT_TRUE(cs[i].get() != nullptr);
    EXPECT_TRUE(cs[i]->is_released());
    EXPECT_NE(producer_state_mask, cs[i]->client_state_mask());
  }

  EXPECT_EQ(0, p->GainAsync());
  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;

  // Post the producer should trigger all consumers to be available.
  EXPECT_EQ(0, p->PostAsync(&metadata, invalid_fence));
  EXPECT_TRUE(isClientReleased(p->buffer_state(), p->client_state_mask()));
  for (size_t i = 0; i < kMaxConsumerCount; ++i) {
    EXPECT_TRUE(
        isClientPosted(cs[i]->buffer_state(), cs[i]->client_state_mask()));
    EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(cs[i])));
    EXPECT_EQ(0, cs[i]->AcquireAsync(&metadata, &invalid_fence));
    EXPECT_TRUE(
        isClientAcquired(p->buffer_state(), cs[i]->client_state_mask()));
  }

  // All consumers have to release before the buffer is considered to be
  // released.
  for (size_t i = 0; i < kMaxConsumerCount; i++) {
    EXPECT_FALSE(p->is_released());
    EXPECT_EQ(0, cs[i]->ReleaseAsync(&metadata, invalid_fence));
  }

  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_TRUE(p->is_released());

  // Buffer state cross all clients must be consistent.
  for (size_t i = 0; i < kMaxConsumerCount; i++) {
    EXPECT_EQ(p->buffer_state(), cs[i]->buffer_state());
  }
}

TEST_F(LibBufferHubTest, TestCreateConsumerWhenBufferGained) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());
  EXPECT_TRUE(isAnyClientGained(p->buffer_state()));

  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_TRUE(isAnyClientGained(c->buffer_state()));

  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;

  // Post the gained buffer should signal already created consumer.
  EXPECT_EQ(0, p->PostAsync(&metadata, invalid_fence));
  EXPECT_TRUE(isAnyClientPosted(p->buffer_state()));
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c)));
  EXPECT_EQ(0, c->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_TRUE(isAnyClientAcquired(c->buffer_state()));
}

TEST_F(LibBufferHubTest, TestCreateTheFirstConsumerAfterPostingBuffer) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());
  EXPECT_TRUE(isAnyClientGained(p->buffer_state()));

  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;

  // Post the gained buffer before any consumer gets created.
  EXPECT_EQ(0, p->PostAsync(&metadata, invalid_fence));
  EXPECT_TRUE(p->is_released());
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(p)));

  // Newly created consumer will be signalled for the posted buffer although it
  // is created after producer posting.
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_TRUE(isClientPosted(c->buffer_state(), c->client_state_mask()));
  EXPECT_EQ(0, c->AcquireAsync(&metadata, &invalid_fence));
}

TEST_F(LibBufferHubTest, TestCreateConsumerWhenBufferReleased) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);

  std::unique_ptr<ConsumerBuffer> c1 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c1.get() != nullptr);

  EXPECT_EQ(0, p->GainAsync());
  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;

  // Post, acquire, and release the buffer..
  EXPECT_EQ(0, p->PostAsync(&metadata, invalid_fence));
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c1)));
  EXPECT_EQ(0, c1->AcquireAsync(&metadata, &invalid_fence));
  EXPECT_EQ(0, c1->ReleaseAsync(&metadata, invalid_fence));

  // Note that the next PDX call is on the producer channel, which may be
  // executed before Release impulse gets executed by bufferhubd. Thus, here we
  // need to wait until the releasd is confirmed before creating another
  // consumer.
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_TRUE(p->is_released());

  // Create another consumer immediately after the release, should not make the
  // buffer un-released.
  std::unique_ptr<ConsumerBuffer> c2 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c2.get() != nullptr);

  EXPECT_TRUE(p->is_released());
  EXPECT_EQ(0, p->GainAsync(&metadata, &invalid_fence));
  EXPECT_TRUE(isAnyClientGained(p->buffer_state()));
}

TEST_F(LibBufferHubTest, TestWithCustomMetadata) {
  struct Metadata {
    int64_t field1;
    int64_t field2;
  };
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(Metadata));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());
  Metadata m = {1, 3};
  EXPECT_EQ(0, p->Post(LocalHandle(), &m, sizeof(Metadata)));
  EXPECT_LE(0, RETRY_EINTR(PollBufferEvent(c)));
  LocalHandle fence;
  Metadata m2 = {};
  EXPECT_EQ(0, c->Acquire(&fence, &m2, sizeof(m2)));
  EXPECT_EQ(m.field1, m2.field1);
  EXPECT_EQ(m.field2, m2.field2);
  EXPECT_EQ(0, c->Release(LocalHandle()));
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p, /*timeout_ms=*/0)));
}

TEST_F(LibBufferHubTest, TestPostWithWrongMetaSize) {
  struct Metadata {
    int64_t field1;
    int64_t field2;
  };
  struct OverSizedMetadata {
    int64_t field1;
    int64_t field2;
    int64_t field3;
  };
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(Metadata));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());

  // It is illegal to post metadata larger than originally requested during
  // buffer allocation.
  OverSizedMetadata evil_meta = {};
  EXPECT_NE(0, p->Post(LocalHandle(), &evil_meta, sizeof(OverSizedMetadata)));
  EXPECT_GE(0, RETRY_EINTR(PollBufferEvent(c)));

  // It is ok to post metadata smaller than originally requested during
  // buffer allocation.
  EXPECT_EQ(0, p->Post(LocalHandle()));
}

TEST_F(LibBufferHubTest, TestAcquireWithWrongMetaSize) {
  struct Metadata {
    int64_t field1;
    int64_t field2;
  };
  struct OverSizedMetadata {
    int64_t field1;
    int64_t field2;
    int64_t field3;
  };
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(Metadata));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());

  Metadata m = {1, 3};
  EXPECT_EQ(0, p->Post(LocalHandle(), &m, sizeof(m)));

  LocalHandle fence;
  int64_t sequence;
  OverSizedMetadata e;

  // It is illegal to acquire metadata larger than originally requested during
  // buffer allocation.
  EXPECT_NE(0, c->Acquire(&fence, &e, sizeof(e)));

  // It is ok to acquire metadata smaller than originally requested during
  // buffer allocation.
  EXPECT_EQ(0, c->Acquire(&fence, &sequence, sizeof(sequence)));
  EXPECT_EQ(m.field1, sequence);
}

TEST_F(LibBufferHubTest, TestAcquireWithNoMeta) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());

  int64_t sequence = 3;
  EXPECT_EQ(0, p->Post(LocalHandle(), &sequence, sizeof(sequence)));

  LocalHandle fence;
  EXPECT_EQ(0, c->Acquire(&fence));
}

TEST_F(LibBufferHubTest, TestWithNoMeta) {
  std::unique_ptr<ProducerBuffer> p =
      ProducerBuffer::Create(kWidth, kHeight, kFormat, kUsage);
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());

  LocalHandle fence;

  EXPECT_EQ(0, p->Post(LocalHandle()));
  EXPECT_EQ(0, c->Acquire(&fence));
}

TEST_F(LibBufferHubTest, TestFailureToPostMetaFromABufferWithoutMeta) {
  std::unique_ptr<ProducerBuffer> p =
      ProducerBuffer::Create(kWidth, kHeight, kFormat, kUsage);
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());

  int64_t sequence = 3;
  EXPECT_NE(0, p->Post(LocalHandle(), &sequence, sizeof(sequence)));
}

namespace {

int PollFd(int fd, int timeout_ms) {
  pollfd p = {fd, POLLIN, 0};
  return poll(&p, 1, timeout_ms);
}

}  // namespace

TEST_F(LibBufferHubTest, TestAcquireFence) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, /*metadata_size=*/0);
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c.get() != nullptr);
  EXPECT_EQ(0, p->GainAsync());

  DvrNativeBufferMetadata meta;
  LocalHandle f1(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));

  // Post with unsignaled fence.
  EXPECT_EQ(0, p->PostAsync(&meta, f1));

  // Should acquire a valid fence.
  LocalHandle f2;
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c)));
  EXPECT_EQ(0, c->AcquireAsync(&meta, &f2));
  EXPECT_TRUE(f2.IsValid());
  // The original fence and acquired fence should have different fd number.
  EXPECT_NE(f1.Get(), f2.Get());
  EXPECT_GE(0, PollFd(f2.Get(), 0));

  // Signal the original fence will trigger the new fence.
  eventfd_write(f1.Get(), 1);
  // Now the original FD has been signaled.
  EXPECT_LT(0, PollFd(f2.Get(), kPollTimeoutMs));

  // Release the consumer with an invalid fence.
  EXPECT_EQ(0, c->ReleaseAsync(&meta, LocalHandle()));

  // Should gain an invalid fence.
  LocalHandle f3;
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(0, p->GainAsync(&meta, &f3));
  EXPECT_FALSE(f3.IsValid());

  // Post with a signaled fence.
  EXPECT_EQ(0, p->PostAsync(&meta, f1));

  // Should acquire a valid fence and it's already signalled.
  LocalHandle f4;
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c)));
  EXPECT_EQ(0, c->AcquireAsync(&meta, &f4));
  EXPECT_TRUE(f4.IsValid());
  EXPECT_LT(0, PollFd(f4.Get(), kPollTimeoutMs));

  // Release with an unsignalled fence and signal it immediately after release
  // without producer gainning.
  LocalHandle f5(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
  EXPECT_EQ(0, c->ReleaseAsync(&meta, f5));
  eventfd_write(f5.Get(), 1);

  // Should gain a valid fence, which is already signaled.
  LocalHandle f6;
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));
  EXPECT_EQ(0, p->GainAsync(&meta, &f6));
  EXPECT_TRUE(f6.IsValid());
  EXPECT_LT(0, PollFd(f6.Get(), kPollTimeoutMs));
}

TEST_F(LibBufferHubTest, TestOrphanedAcquire) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c1 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c1.get() != nullptr);
  const uint32_t client_state_mask1 = c1->client_state_mask();

  EXPECT_EQ(0, p->GainAsync());
  DvrNativeBufferMetadata meta;
  EXPECT_EQ(0, p->PostAsync(&meta, LocalHandle()));

  LocalHandle fence;
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c1)));
  EXPECT_EQ(0, c1->AcquireAsync(&meta, &fence));

  // Destroy the consumer who has acquired but not released the buffer.
  c1 = nullptr;

  // The buffer is now available for the producer to gain.
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));

  // Newly added consumer is not able to acquire the buffer.
  std::unique_ptr<ConsumerBuffer> c2 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c2.get() != nullptr);
  const uint32_t client_state_mask2 = c2->client_state_mask();
  EXPECT_NE(client_state_mask1, client_state_mask2);
  EXPECT_EQ(0, RETRY_EINTR(PollBufferEvent(c2)));
  EXPECT_EQ(-EBUSY, c2->AcquireAsync(&meta, &fence));

  // Producer should be able to gain.
  EXPECT_EQ(0, p->GainAsync(&meta, &fence, false));
}

TEST_F(LibBufferHubTest, TestAcquireLastPosted) {
  std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p.get() != nullptr);
  std::unique_ptr<ConsumerBuffer> c1 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c1.get() != nullptr);
  const uint32_t client_state_mask1 = c1->client_state_mask();

  EXPECT_EQ(0, p->GainAsync());
  DvrNativeBufferMetadata meta;
  EXPECT_EQ(0, p->PostAsync(&meta, LocalHandle()));
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c1)));

  // c2 is created when the buffer is in posted state. buffer state for c1 is
  // posted. Thus, c2 should be automatically set to posted and able to acquire.
  std::unique_ptr<ConsumerBuffer> c2 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c2.get() != nullptr);
  const uint32_t client_state_mask2 = c2->client_state_mask();
  EXPECT_NE(client_state_mask1, client_state_mask2);
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c2)));
  LocalHandle invalid_fence;
  EXPECT_EQ(0, c2->AcquireAsync(&meta, &invalid_fence));

  EXPECT_EQ(0, c1->AcquireAsync(&meta, &invalid_fence));

  // c3 is created when the buffer is in acquired state. buffer state for c1 and
  // c2 are acquired. Thus, c3 should be automatically set to posted and able to
  // acquire.
  std::unique_ptr<ConsumerBuffer> c3 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c3.get() != nullptr);
  const uint32_t client_state_mask3 = c3->client_state_mask();
  EXPECT_NE(client_state_mask1, client_state_mask3);
  EXPECT_NE(client_state_mask2, client_state_mask3);
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(c3)));
  EXPECT_EQ(0, c3->AcquireAsync(&meta, &invalid_fence));

  // Releasing c2 and c3 in normal ways.
  EXPECT_EQ(0, c2->Release(LocalHandle()));
  EXPECT_EQ(0, c3->ReleaseAsync(&meta, LocalHandle()));

  // Destroy the c1 who has not released the buffer.
  c1 = nullptr;

  // The buffer is now available for the producer to gain.
  EXPECT_LT(0, RETRY_EINTR(PollBufferEvent(p)));

  // C4 is created in released state. Thus, it cannot gain the just posted
  // buffer.
  std::unique_ptr<ConsumerBuffer> c4 =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(c4.get() != nullptr);
  const uint32_t client_state_mask4 = c4->client_state_mask();
  EXPECT_NE(client_state_mask3, client_state_mask4);
  EXPECT_GE(0, RETRY_EINTR(PollBufferEvent(c3)));
  EXPECT_EQ(-EBUSY, c3->AcquireAsync(&meta, &invalid_fence));

  // Producer should be able to gain.
  EXPECT_EQ(0, p->GainAsync(&meta, &invalid_fence));
}

TEST_F(LibBufferHubTest, TestDetachBufferFromProducer) {
  // TODO(b/112338294) rewrite test after migration
  return;

  /* std::unique_ptr<ProducerBuffer> p = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  std::unique_ptr<ConsumerBuffer> c =
      ConsumerBuffer::Import(p->CreateConsumer());
  ASSERT_TRUE(p.get() != nullptr);
  ASSERT_TRUE(c.get() != nullptr);

  DvrNativeBufferMetadata metadata;
  LocalHandle invalid_fence;
  int p_id = p->id();

  // Detach in posted state should fail.
  EXPECT_EQ(0, p->GainAsync());
  EXPECT_EQ(0, p->PostAsync(&metadata, invalid_fence));
  EXPECT_GT(RETRY_EINTR(PollBufferEvent(c)), 0);
  auto s1 = p->Detach();
  EXPECT_FALSE(s1);

  // Detach in acquired state should fail.
  EXPECT_EQ(0, c->AcquireAsync(&metadata, &invalid_fence));
  s1 = p->Detach();
  EXPECT_FALSE(s1);

  // Detach in released state should fail.
  EXPECT_EQ(0, c->ReleaseAsync(&metadata, invalid_fence));
  EXPECT_GT(RETRY_EINTR(PollBufferEvent(p)), 0);
  s1 = p->Detach();
  EXPECT_FALSE(s1);

  // Detach in gained state should succeed.
  EXPECT_EQ(0, p->GainAsync(&metadata, &invalid_fence));
  s1 = p->Detach();
  EXPECT_TRUE(s1);

  LocalChannelHandle handle = s1.take();
  EXPECT_TRUE(handle.valid());

  // Both producer and consumer should have hangup.
  EXPECT_GT(RETRY_EINTR(PollBufferEvent(p)), 0);
  auto s2 = p->GetEventMask(POLLHUP);
  EXPECT_TRUE(s2);
  EXPECT_EQ(s2.get(), POLLHUP);

  EXPECT_GT(RETRY_EINTR(PollBufferEvent(c)), 0);
  s2 = p->GetEventMask(POLLHUP);
  EXPECT_TRUE(s2);
  EXPECT_EQ(s2.get(), POLLHUP);

  auto s3 = p->CreateConsumer();
  EXPECT_FALSE(s3);
  // Note that here the expected error code is EOPNOTSUPP as the socket towards
  // ProducerChannel has been teared down.
  EXPECT_EQ(s3.error(), EOPNOTSUPP);

  s3 = c->CreateConsumer();
  EXPECT_FALSE(s3);
  // Note that here the expected error code is EPIPE returned from
  // ConsumerChannel::HandleMessage as the socket is still open but the producer
  // is gone.
  EXPECT_EQ(s3.error(), EPIPE);

  // Detached buffer handle can be use to construct a new BufferHubBuffer
  // object.
  auto d = BufferHubBuffer::Import(std::move(handle));
  EXPECT_FALSE(handle.valid());
  EXPECT_TRUE(d->IsConnected());
  EXPECT_TRUE(d->IsValid());

  EXPECT_EQ(d->id(), p_id); */
}

TEST_F(LibBufferHubTest, TestDetach) {
  // TODO(b/112338294) rewrite test after migration
  return;

  /* std::unique_ptr<ProducerBuffer> p1 = ProducerBuffer::Create(
      kWidth, kHeight, kFormat, kUsage, sizeof(uint64_t));
  ASSERT_TRUE(p1.get() != nullptr);
  int p1_id = p1->id();

  // Detached the producer from gained state.
  EXPECT_EQ(0, p1->GainAsync());
  auto status_or_handle = p1->Detach();
  EXPECT_TRUE(status_or_handle.ok());
  LocalChannelHandle h1 = status_or_handle.take();
  EXPECT_TRUE(h1.valid());

  // Detached buffer handle can be use to construct a new BufferHubBuffer
  // object.
  auto b1 = BufferHubBuffer::Import(std::move(h1));
  EXPECT_FALSE(h1.valid());
  EXPECT_TRUE(b1->IsValid());
  int b1_id = b1->id();
  EXPECT_EQ(b1_id, p1_id); */
}
