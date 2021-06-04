/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "ExecutionBurstServer"

#include "ExecutionBurstServer.h"

#include <android-base/logging.h>

#include <cstring>
#include <limits>
#include <map>

#include "Tracing.h"

namespace android::nn {
namespace {

constexpr Timing kNoTiming = {std::numeric_limits<uint64_t>::max(),
                              std::numeric_limits<uint64_t>::max()};

// DefaultBurstExecutorWithCache adapts an IPreparedModel so that it can be
// used as an IBurstExecutorWithCache. Specifically, the cache simply stores the
// hidl_memory object, and the execution forwards calls to the provided
// IPreparedModel's "executeSynchronously" method. With this class, hidl_memory
// must be mapped and unmapped for each execution.
class DefaultBurstExecutorWithCache : public ExecutionBurstServer::IBurstExecutorWithCache {
   public:
    DefaultBurstExecutorWithCache(IPreparedModel* preparedModel) : mpPreparedModel(preparedModel) {}

    bool isCacheEntryPresent(int32_t slot) const override {
        const auto it = mMemoryCache.find(slot);
        return (it != mMemoryCache.end()) && it->second.valid();
    }

    void addCacheEntry(const hidl_memory& memory, int32_t slot) override {
        mMemoryCache[slot] = memory;
    }

    void removeCacheEntry(int32_t slot) override { mMemoryCache.erase(slot); }

    std::tuple<ErrorStatus, hidl_vec<OutputShape>, Timing> execute(
            const Request& request, const std::vector<int32_t>& slots,
            MeasureTiming measure) override {
        // convert slots to pools
        hidl_vec<hidl_memory> pools(slots.size());
        std::transform(slots.begin(), slots.end(), pools.begin(),
                       [this](int32_t slot) { return mMemoryCache[slot]; });

        // create full request
        Request fullRequest = request;
        fullRequest.pools = std::move(pools);

        // setup execution
        ErrorStatus returnedStatus = ErrorStatus::GENERAL_FAILURE;
        hidl_vec<OutputShape> returnedOutputShapes;
        Timing returnedTiming;
        auto cb = [&returnedStatus, &returnedOutputShapes, &returnedTiming](
                          ErrorStatus status, const hidl_vec<OutputShape>& outputShapes,
                          const Timing& timing) {
            returnedStatus = status;
            returnedOutputShapes = outputShapes;
            returnedTiming = timing;
        };

        // execute
        const Return<void> ret = mpPreparedModel->executeSynchronously(fullRequest, measure, cb);
        if (!ret.isOk() || returnedStatus != ErrorStatus::NONE) {
            LOG(ERROR) << "IPreparedModelAdapter::execute -- Error executing";
            return {returnedStatus, {}, kNoTiming};
        }

        return std::make_tuple(returnedStatus, std::move(returnedOutputShapes), returnedTiming);
    }

   private:
    IPreparedModel* const mpPreparedModel;
    std::map<int32_t, hidl_memory> mMemoryCache;
};

}  // anonymous namespace

// serialize result
std::vector<FmqResultDatum> serialize(ErrorStatus errorStatus,
                                      const std::vector<OutputShape>& outputShapes, Timing timing) {
    // count how many elements need to be sent for a request
    size_t count = 2 + outputShapes.size();
    for (const auto& outputShape : outputShapes) {
        count += outputShape.dimensions.size();
    }

    // create buffer to temporarily store elements
    std::vector<FmqResultDatum> data;
    data.reserve(count);

    // package packetInfo
    {
        FmqResultDatum datum;
        datum.packetInformation({/*.packetSize=*/static_cast<uint32_t>(count),
                                 /*.errorStatus=*/errorStatus,
                                 /*.numberOfOperands=*/static_cast<uint32_t>(outputShapes.size())});
        data.push_back(datum);
    }

    // package output shape data
    for (const auto& operand : outputShapes) {
        // package operand information
        FmqResultDatum::OperandInformation info{};
        info.isSufficient = operand.isSufficient;
        info.numberOfDimensions = static_cast<uint32_t>(operand.dimensions.size());

        FmqResultDatum datum;
        datum.operandInformation(info);
        data.push_back(datum);

        // package operand dimensions
        for (uint32_t dimension : operand.dimensions) {
            FmqResultDatum datum;
            datum.operandDimensionValue(dimension);
            data.push_back(datum);
        }
    }

    // package executionTiming
    {
        FmqResultDatum datum;
        datum.executionTiming(timing);
        data.push_back(datum);
    }

    // return result
    return data;
}

// deserialize request
std::optional<std::tuple<Request, std::vector<int32_t>, MeasureTiming>> deserialize(
        const std::vector<FmqRequestDatum>& data) {
    using discriminator = FmqRequestDatum::hidl_discriminator;

    size_t index = 0;

    // validate packet information
    if (data.size() == 0 || data[index].getDiscriminator() != discriminator::packetInformation) {
        LOG(ERROR) << "FMQ Request packet ill-formed";
        return std::nullopt;
    }

    // unpackage packet information
    const FmqRequestDatum::PacketInformation& packetInfo = data[index].packetInformation();
    index++;
    const uint32_t packetSize = packetInfo.packetSize;
    const uint32_t numberOfInputOperands = packetInfo.numberOfInputOperands;
    const uint32_t numberOfOutputOperands = packetInfo.numberOfOutputOperands;
    const uint32_t numberOfPools = packetInfo.numberOfPools;

    // verify packet size
    if (data.size() != packetSize) {
        LOG(ERROR) << "FMQ Request packet ill-formed";
        return std::nullopt;
    }

    // unpackage input operands
    std::vector<RequestArgument> inputs;
    inputs.reserve(numberOfInputOperands);
    for (size_t operand = 0; operand < numberOfInputOperands; ++operand) {
        // validate input operand information
        if (data[index].getDiscriminator() != discriminator::inputOperandInformation) {
            LOG(ERROR) << "FMQ Request packet ill-formed";
            return std::nullopt;
        }

        // unpackage operand information
        const FmqRequestDatum::OperandInformation& operandInfo =
                data[index].inputOperandInformation();
        index++;
        const bool hasNoValue = operandInfo.hasNoValue;
        const DataLocation location = operandInfo.location;
        const uint32_t numberOfDimensions = operandInfo.numberOfDimensions;

        // unpackage operand dimensions
        std::vector<uint32_t> dimensions;
        dimensions.reserve(numberOfDimensions);
        for (size_t i = 0; i < numberOfDimensions; ++i) {
            // validate dimension
            if (data[index].getDiscriminator() != discriminator::inputOperandDimensionValue) {
                LOG(ERROR) << "FMQ Request packet ill-formed";
                return std::nullopt;
            }

            // unpackage dimension
            const uint32_t dimension = data[index].inputOperandDimensionValue();
            index++;

            // store result
            dimensions.push_back(dimension);
        }

        // store result
        inputs.push_back(
                {/*.hasNoValue=*/hasNoValue, /*.location=*/location, /*.dimensions=*/dimensions});
    }

    // unpackage output operands
    std::vector<RequestArgument> outputs;
    outputs.reserve(numberOfOutputOperands);
    for (size_t operand = 0; operand < numberOfOutputOperands; ++operand) {
        // validate output operand information
        if (data[index].getDiscriminator() != discriminator::outputOperandInformation) {
            LOG(ERROR) << "FMQ Request packet ill-formed";
            return std::nullopt;
        }

        // unpackage operand information
        const FmqRequestDatum::OperandInformation& operandInfo =
                data[index].outputOperandInformation();
        index++;
        const bool hasNoValue = operandInfo.hasNoValue;
        const DataLocation location = operandInfo.location;
        const uint32_t numberOfDimensions = operandInfo.numberOfDimensions;

        // unpackage operand dimensions
        std::vector<uint32_t> dimensions;
        dimensions.reserve(numberOfDimensions);
        for (size_t i = 0; i < numberOfDimensions; ++i) {
            // validate dimension
            if (data[index].getDiscriminator() != discriminator::outputOperandDimensionValue) {
                LOG(ERROR) << "FMQ Request packet ill-formed";
                return std::nullopt;
            }

            // unpackage dimension
            const uint32_t dimension = data[index].outputOperandDimensionValue();
            index++;

            // store result
            dimensions.push_back(dimension);
        }

        // store result
        outputs.push_back(
                {/*.hasNoValue=*/hasNoValue, /*.location=*/location, /*.dimensions=*/dimensions});
    }

    // unpackage pools
    std::vector<int32_t> slots;
    slots.reserve(numberOfPools);
    for (size_t pool = 0; pool < numberOfPools; ++pool) {
        // validate input operand information
        if (data[index].getDiscriminator() != discriminator::poolIdentifier) {
            LOG(ERROR) << "FMQ Request packet ill-formed";
            return std::nullopt;
        }

        // unpackage operand information
        const int32_t poolId = data[index].poolIdentifier();
        index++;

        // store result
        slots.push_back(poolId);
    }

    // validate measureTiming
    if (data[index].getDiscriminator() != discriminator::measureTiming) {
        LOG(ERROR) << "FMQ Request packet ill-formed";
        return std::nullopt;
    }

    // unpackage measureTiming
    const MeasureTiming measure = data[index].measureTiming();
    index++;

    // validate packet information
    if (index != packetSize) {
        LOG(ERROR) << "FMQ Result packet ill-formed";
        return std::nullopt;
    }

    // return request
    Request request = {/*.inputs=*/inputs, /*.outputs=*/outputs, /*.pools=*/{}};
    return std::make_tuple(std::move(request), std::move(slots), measure);
}

// RequestChannelReceiver methods

std::unique_ptr<RequestChannelReceiver> RequestChannelReceiver::create(
        const FmqRequestDescriptor& requestChannel) {
    std::unique_ptr<FmqRequestChannel> fmqRequestChannel =
            std::make_unique<FmqRequestChannel>(requestChannel);
    if (!fmqRequestChannel->isValid()) {
        LOG(ERROR) << "Unable to create RequestChannelReceiver";
        return nullptr;
    }
    const bool blocking = fmqRequestChannel->getEventFlagWord() != nullptr;
    return std::make_unique<RequestChannelReceiver>(std::move(fmqRequestChannel), blocking);
}

RequestChannelReceiver::RequestChannelReceiver(std::unique_ptr<FmqRequestChannel> fmqRequestChannel,
                                               bool blocking)
    : mFmqRequestChannel(std::move(fmqRequestChannel)), mBlocking(blocking) {}

std::optional<std::tuple<Request, std::vector<int32_t>, MeasureTiming>>
RequestChannelReceiver::getBlocking() {
    const auto packet = getPacketBlocking();
    if (!packet) {
        return std::nullopt;
    }

    return deserialize(*packet);
}

void RequestChannelReceiver::invalidate() {
    mTeardown = true;

    // force unblock
    // ExecutionBurstServer is by default waiting on a request packet. If the
    // client process destroys its burst object, the server will still be
    // waiting on the futex (assuming mBlocking is true). This force unblock
    // wakes up any thread waiting on the futex.
    if (mBlocking) {
        // TODO: look for a different/better way to signal/notify the futex to
        // wake up any thread waiting on it
        FmqRequestDatum datum;
        datum.packetInformation({/*.packetSize=*/0, /*.numberOfInputOperands=*/0,
                                 /*.numberOfOutputOperands=*/0, /*.numberOfPools=*/0});
        mFmqRequestChannel->writeBlocking(&datum, 1);
    }
}

std::optional<std::vector<FmqRequestDatum>> RequestChannelReceiver::getPacketBlocking() {
    using discriminator = FmqRequestDatum::hidl_discriminator;

    if (mTeardown) {
        return std::nullopt;
    }

    // wait for request packet and read first element of request packet
    FmqRequestDatum datum;
    bool success = false;
    if (mBlocking) {
        success = mFmqRequestChannel->readBlocking(&datum, 1);
    } else {
        while ((success = !mTeardown.load(std::memory_order_relaxed)) &&
               !mFmqRequestChannel->read(&datum, 1)) {
        }
    }

    NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_EXECUTION, "ExecutionBurstServer getting packet");

    // retrieve remaining elements
    // NOTE: all of the data is already available at this point, so there's no
    // need to do a blocking wait to wait for more data. This is known because
    // in FMQ, all writes are published (made available) atomically. Currently,
    // the producer always publishes the entire packet in one function call, so
    // if the first element of the packet is available, the remaining elements
    // are also available.
    const size_t count = mFmqRequestChannel->availableToRead();
    std::vector<FmqRequestDatum> packet(count + 1);
    std::memcpy(&packet.front(), &datum, sizeof(datum));
    success &= mFmqRequestChannel->read(packet.data() + 1, count);

    // terminate loop
    if (mTeardown) {
        return std::nullopt;
    }

    // ensure packet was successfully received
    if (!success) {
        LOG(ERROR) << "Error receiving packet";
        return std::nullopt;
    }

    return std::make_optional(std::move(packet));
}

// ResultChannelSender methods

std::unique_ptr<ResultChannelSender> ResultChannelSender::create(
        const FmqResultDescriptor& resultChannel) {
    std::unique_ptr<FmqResultChannel> fmqResultChannel =
            std::make_unique<FmqResultChannel>(resultChannel);
    if (!fmqResultChannel->isValid()) {
        LOG(ERROR) << "Unable to create RequestChannelSender";
        return nullptr;
    }
    const bool blocking = fmqResultChannel->getEventFlagWord() != nullptr;
    return std::make_unique<ResultChannelSender>(std::move(fmqResultChannel), blocking);
}

ResultChannelSender::ResultChannelSender(std::unique_ptr<FmqResultChannel> fmqResultChannel,
                                         bool blocking)
    : mFmqResultChannel(std::move(fmqResultChannel)), mBlocking(blocking) {}

bool ResultChannelSender::send(ErrorStatus errorStatus,
                               const std::vector<OutputShape>& outputShapes, Timing timing) {
    const std::vector<FmqResultDatum> serialized = serialize(errorStatus, outputShapes, timing);
    return sendPacket(serialized);
}

bool ResultChannelSender::sendPacket(const std::vector<FmqResultDatum>& packet) {
    if (packet.size() > mFmqResultChannel->availableToWrite()) {
        LOG(ERROR)
                << "ResultChannelSender::sendPacket -- packet size exceeds size available in FMQ";
        const std::vector<FmqResultDatum> errorPacket =
                serialize(ErrorStatus::GENERAL_FAILURE, {}, kNoTiming);
        if (mBlocking) {
            return mFmqResultChannel->writeBlocking(errorPacket.data(), errorPacket.size());
        } else {
            return mFmqResultChannel->write(errorPacket.data(), errorPacket.size());
        }
    }

    if (mBlocking) {
        return mFmqResultChannel->writeBlocking(packet.data(), packet.size());
    } else {
        return mFmqResultChannel->write(packet.data(), packet.size());
    }
}

// ExecutionBurstServer methods

sp<ExecutionBurstServer> ExecutionBurstServer::create(
        const sp<IBurstCallback>& callback, const MQDescriptorSync<FmqRequestDatum>& requestChannel,
        const MQDescriptorSync<FmqResultDatum>& resultChannel,
        std::shared_ptr<IBurstExecutorWithCache> executorWithCache) {
    // check inputs
    if (callback == nullptr || executorWithCache == nullptr) {
        LOG(ERROR) << "ExecutionBurstServer::create passed a nullptr";
        return nullptr;
    }

    // create FMQ objects
    std::unique_ptr<RequestChannelReceiver> requestChannelReceiver =
            RequestChannelReceiver::create(requestChannel);
    std::unique_ptr<ResultChannelSender> resultChannelSender =
            ResultChannelSender::create(resultChannel);

    // check FMQ objects
    if (!requestChannelReceiver || !resultChannelSender) {
        LOG(ERROR) << "ExecutionBurstServer::create failed to create FastMessageQueue";
        return nullptr;
    }

    // make and return context
    return new ExecutionBurstServer(callback, std::move(requestChannelReceiver),
                                    std::move(resultChannelSender), std::move(executorWithCache));
}

sp<ExecutionBurstServer> ExecutionBurstServer::create(
        const sp<IBurstCallback>& callback, const MQDescriptorSync<FmqRequestDatum>& requestChannel,
        const MQDescriptorSync<FmqResultDatum>& resultChannel, IPreparedModel* preparedModel) {
    // check relevant input
    if (preparedModel == nullptr) {
        LOG(ERROR) << "ExecutionBurstServer::create passed a nullptr";
        return nullptr;
    }

    // adapt IPreparedModel to have caching
    const std::shared_ptr<DefaultBurstExecutorWithCache> preparedModelAdapter =
            std::make_shared<DefaultBurstExecutorWithCache>(preparedModel);

    // make and return context
    return ExecutionBurstServer::create(callback, requestChannel, resultChannel,
                                        preparedModelAdapter);
}

ExecutionBurstServer::ExecutionBurstServer(
        const sp<IBurstCallback>& callback, std::unique_ptr<RequestChannelReceiver> requestChannel,
        std::unique_ptr<ResultChannelSender> resultChannel,
        std::shared_ptr<IBurstExecutorWithCache> executorWithCache)
    : mCallback(callback),
      mRequestChannelReceiver(std::move(requestChannel)),
      mResultChannelSender(std::move(resultChannel)),
      mExecutorWithCache(std::move(executorWithCache)) {
    // TODO: highly document the threading behavior of this class
    mWorker = std::thread([this] { task(); });
}

ExecutionBurstServer::~ExecutionBurstServer() {
    // set teardown flag
    mTeardown = true;
    mRequestChannelReceiver->invalidate();

    // wait for task thread to end
    mWorker.join();
}

Return<void> ExecutionBurstServer::freeMemory(int32_t slot) {
    mExecutorWithCache->removeCacheEntry(slot);
    return Void();
}

void ExecutionBurstServer::ensureCacheEntriesArePresentLocked(const std::vector<int32_t>& slots) {
    const auto slotIsKnown = [this](int32_t slot) {
        return mExecutorWithCache->isCacheEntryPresent(slot);
    };

    // find unique unknown slots
    std::vector<int32_t> unknownSlots = slots;
    auto unknownSlotsEnd = unknownSlots.end();
    std::sort(unknownSlots.begin(), unknownSlotsEnd);
    unknownSlotsEnd = std::unique(unknownSlots.begin(), unknownSlotsEnd);
    unknownSlotsEnd = std::remove_if(unknownSlots.begin(), unknownSlotsEnd, slotIsKnown);
    unknownSlots.erase(unknownSlotsEnd, unknownSlots.end());

    // quick-exit if all slots are known
    if (unknownSlots.empty()) {
        return;
    }

    ErrorStatus errorStatus = ErrorStatus::GENERAL_FAILURE;
    std::vector<hidl_memory> returnedMemories;
    auto cb = [&errorStatus, &returnedMemories](ErrorStatus status,
                                                const hidl_vec<hidl_memory>& memories) {
        errorStatus = status;
        returnedMemories = memories;
    };

    const Return<void> ret = mCallback->getMemories(unknownSlots, cb);

    if (!ret.isOk() || errorStatus != ErrorStatus::NONE ||
        returnedMemories.size() != unknownSlots.size()) {
        LOG(ERROR) << "Error retrieving memories";
        return;
    }

    // add memories to unknown slots
    for (size_t i = 0; i < unknownSlots.size(); ++i) {
        mExecutorWithCache->addCacheEntry(returnedMemories[i], unknownSlots[i]);
    }
}

void ExecutionBurstServer::task() {
    // loop until the burst object is being destroyed
    while (!mTeardown) {
        // receive request
        auto arguments = mRequestChannelReceiver->getBlocking();

        // if the request packet was not properly received, return a generic
        // error and skip the execution
        //
        // if the  burst is being torn down, skip the execution exection so the
        // "task" function can end
        if (!arguments) {
            if (!mTeardown) {
                mResultChannelSender->send(ErrorStatus::GENERAL_FAILURE, {}, kNoTiming);
            }
            continue;
        }

        // otherwise begin tracing execution
        NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_EXECUTION,
                     "ExecutionBurstServer getting memory, executing, and returning results");

        // unpack the arguments; types are Request, std::vector<int32_t>, and
        // MeasureTiming, respectively
        const auto [requestWithoutPools, slotsOfPools, measure] = std::move(*arguments);

        // ensure executor with cache has required memory
        std::lock_guard<std::mutex> hold(mMutex);
        ensureCacheEntriesArePresentLocked(slotsOfPools);

        // perform computation; types are ErrorStatus, hidl_vec<OutputShape>,
        // and Timing, respectively
        const auto [errorStatus, outputShapes, returnedTiming] =
                mExecutorWithCache->execute(requestWithoutPools, slotsOfPools, measure);

        // return result
        mResultChannelSender->send(errorStatus, outputShapes, returnedTiming);
    }
}

}  // namespace android::nn
