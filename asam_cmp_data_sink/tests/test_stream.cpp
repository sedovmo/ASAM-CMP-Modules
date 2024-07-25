#include <asam_cmp_data_sink/calls_multi_map.h>
#include <asam_cmp_data_sink/capture_fb.h>
#include <asam_cmp_data_sink/common.h>
#include <asam_cmp_data_sink/data_handler.h>

#include <asam_cmp/can_payload.h>
#include <asam_cmp/packet.h>
#include <gtest/gtest.h>
#include <opendaq/context_factory.h>
#include <opendaq/module_ptr.h>
#include <opendaq/reader_factory.h>
#include <opendaq/scheduler_factory.h>
#include <opendaq/search_filter_factory.h>

using namespace std::chrono_literals;

using namespace daq;
using daq::modules::asam_cmp_data_sink_module::CallsMultiMap;
using daq::modules::asam_cmp_data_sink_module::IDataHandler;

bool waitForSamples(const GenericReaderPtr<IReader>& reader, std::chrono::milliseconds timeout = 100ms)
{
    auto startTime = std::chrono::steady_clock::now();
    auto curTime = startTime;
    while (reader.getAvailableCount() == 0 && curTime - startTime < timeout)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        curTime = std::chrono::steady_clock::now();
    }

    return curTime - startTime < timeout;
}

class AsamCmpStreamFixture : public testing::Test
{
protected:
#pragma pack(push, 1)
    struct CANData
    {
        uint32_t arbId;
        uint8_t length;
        uint8_t data[64];
    };
#pragma pack(pop)

protected:
    void SetUp()
    {
        auto logger = Logger();
        captureFb = createWithImplementation<IFunctionBlock, modules::asam_cmp_data_sink_module::CaptureFb>(
            Context(Scheduler(logger), logger, nullptr, nullptr, nullptr), nullptr, "capture_module_0", callsMultiMap);

        captureFb.getPropertyValue("AddInterface").execute();
        interfaceFb = captureFb.getFunctionBlocks().getItemAt(0);
        interfaceFb.getPropertyValue("AddStream").execute();
        funcBlock = interfaceFb.getFunctionBlocks().getItemAt(0);
    }

protected:
    CallsMultiMap callsMultiMap;
    FunctionBlockPtr captureFb;
    FunctionBlockPtr interfaceFb;
    FunctionBlockPtr funcBlock;
};

TEST_F(AsamCmpStreamFixture, NotNull)
{
    ASSERT_NE(funcBlock, nullptr);
}

TEST_F(AsamCmpStreamFixture, FunctionBlockType)
{
    auto type = funcBlock.getFunctionBlockType();
    ASSERT_EQ(type.getId(), "asam_cmp_stream");
    ASSERT_EQ(type.getName(), "AsamCmpStream");
    ASSERT_EQ(type.getDescription(), "Asam CMP Stream");
}

TEST_F(AsamCmpStreamFixture, StreamIdProperty)
{
    const int streamId = funcBlock.getPropertyValue("StreamId");
    const int newStreamId = streamId + 1;
    funcBlock.setPropertyValue("StreamId", newStreamId);
    ASSERT_EQ(funcBlock.getPropertyValue("StreamId"), newStreamId);
}

TEST_F(AsamCmpStreamFixture, SignalsCount)
{
    const auto outputSignals = funcBlock.getSignalsRecursive();
    ASSERT_EQ(outputSignals.getCount(), 1);
}

TEST_F(AsamCmpStreamFixture, ReceivePacketWithWrongPayloadType)
{
    ASAM::CMP::CanPayload canPayload;
    uint32_t data = 33;
    canPayload.setData(reinterpret_cast<uint8_t*>(&data), sizeof(data));
    const auto packet = std::make_shared<ASAM::CMP::Packet>();
    packet->setPayload(canPayload);

    interfaceFb.setPropertyValue("PayloadType", 3);
    const auto dataHandler = funcBlock.as<IDataHandler>(true);

    const auto outputSignal = funcBlock.getSignalsRecursive()[0];
    const StreamReaderPtr reader = StreamReader(outputSignal);

    dataHandler->processData(packet);
    const bool waited = waitForSamples(reader);
    ASSERT_FALSE(waited);
}

TEST_F(AsamCmpStreamFixture, ReadOutputSignal)
{
    ASAM::CMP::CanPayload canPayload;
    uint32_t data = 33;
    canPayload.setData(reinterpret_cast<uint8_t*>(&data), sizeof(data));
    const auto packet = std::make_shared<ASAM::CMP::Packet>();
    packet->setPayload(canPayload);
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    packet->setTimestamp(timestamp);

    interfaceFb.setPropertyValue("PayloadType", 1);
    const auto dataHandler = funcBlock.as<IDataHandler>(true);

    const auto outputSignal = funcBlock.getSignalsRecursive()[0];
    const StreamReaderPtr reader = StreamReader(outputSignal, SampleType::Struct, SampleType::Int64);

    dataHandler->processData(packet);
    const bool waited = waitForSamples(reader);
    ASSERT_TRUE(waited);

    CANData sample;
    int64_t domainSample;
    size_t count = 1;
    reader.readWithDomain(&sample, &domainSample, &count);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(domainSample, timestamp);
    ASSERT_EQ(sample.length, sizeof(data));
    uint32_t checkData = *reinterpret_cast<uint32_t*>(sample.data);
    ASSERT_EQ(checkData, data);
}
