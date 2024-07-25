#include <asam_cmp/can_payload.h>
#include <opendaq/dimension_factory.h>
#include <opendaq/packet_factory.h>

#include <asam_cmp_data_sink/stream_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

StreamFb::StreamFb(const ContextPtr& ctx,
                   const ComponentPtr& parent,
                   const StringPtr& localId,
                   const asam_cmp_common_lib::StreamCommonInit& init)
    : StreamCommonFbImpl(ctx, parent, localId, init)
{
    createSignals();
    buildDataDescriptor();
    buildDomainDescriptor();
}

void StreamFb::setPayloadType(PayloadType type)
{
    StreamCommonFbImpl::setPayloadType(type);
    buildDataDescriptor();
}

void StreamFb::processData(const std::shared_ptr<ASAM::CMP::Packet>& packet)
{
    if (packet->getPayload().getType() != payloadType)
        return;

    constexpr uint64_t newSamples = 1;
    auto& canPayload = static_cast<const ASAM::CMP::CanPayload&>(packet->getPayload());
    auto timestamp = packet->getTimestamp();

    const auto domainPacket = DataPacket(domainSignal.getDescriptor(), newSamples, timestamp);
    const auto dataPacket = DataPacketWithDomain(domainPacket, dataSignal.getDescriptor(), newSamples);

    auto outDomainPacketBuf = static_cast<int64_t*>(domainPacket.getRawData());
    if (outDomainPacketBuf)
        *outDomainPacketBuf = timestamp;

    auto dataBuffer = static_cast<CANData*>(dataPacket.getRawData());
    dataBuffer->arbId = canPayload.getId();
    dataBuffer->length = canPayload.getDataLength();
    memcpy(dataBuffer->data, canPayload.getData(), dataBuffer->length);

    dataSignal.sendPacket(dataPacket);
    domainSignal.sendPacket(domainPacket);
}

void StreamFb::createSignals()
{
    dataSignal = createAndAddSignal("data");
    dataSignal.setName("Data");
    domainSignal = createAndAddSignal("domain", nullptr, false);
    domainSignal.setName("Domain");
    dataSignal.setDomainSignal(domainSignal);
}

void StreamFb::buildDataDescriptor()
{
    switch (payloadType.getType())
    {
        case PayloadType::can:
        case PayloadType::canFd:
            buildCanDescriptor();
            break;
    }
}

void StreamFb::buildDomainDescriptor()
{
    const auto domainDescriptor = DataDescriptorBuilder()
                                      .setSampleType(SampleType::Int64)
                                      .setUnit(Unit("s", -1, "seconds", "time"))
                                      .setTickResolution(Ratio(1, 1000000000))
                                      .setOrigin(getEpoch())
                                      .setName("Time");

    domainSignal.setDescriptor(domainDescriptor.build());
}

void StreamFb::buildCanDescriptor()
{
    const auto arbIdDescriptor = DataDescriptorBuilder().setName("ArbId").setSampleType(SampleType::Int32).build();
    const auto lengthDescriptor = DataDescriptorBuilder().setName("Length").setSampleType(SampleType::Int8).build();

    const auto dataDescriptor =
        DataDescriptorBuilder()
            .setName("Data")
            .setSampleType(SampleType::UInt8)
            .setDimensions(List<IDimension>(DimensionBuilder().setRule(LinearDimensionRule(0, 1, 64)).setName("Dimension").build()))
            .build();

    const auto canMsgDescriptor = DataDescriptorBuilder()
                                      .setSampleType(SampleType::Struct)
                                      .setStructFields(List<IDataDescriptor>(arbIdDescriptor, lengthDescriptor, dataDescriptor))
                                      .setName("CAN")
                                      .build();

    dataSignal.setDescriptor(canMsgDescriptor);
}

StringPtr StreamFb::getEpoch() const
{
    const std::time_t epochTime = std::chrono::system_clock::to_time_t(std::chrono::time_point<std::chrono::system_clock>{});

    char buf[48];
    strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&epochTime));

    return {buf};
}

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
