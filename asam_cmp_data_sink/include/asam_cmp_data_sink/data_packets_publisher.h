#pragma once
#include <asam_cmp_data_sink/common.h>
#include <asam_cmp_data_sink/publisher.h>
#include <asam_cmp_data_sink/asam_cmp_packets_subscriber.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE


struct Endpoint
{
    uint16_t deviceId{0};
    uint32_t interfaceId{0};
    uint8_t streamId{0};

    bool operator==(const Endpoint& rhs) const
    {
        return (deviceId == rhs.deviceId) && (streamId == rhs.streamId) && (interfaceId == rhs.interfaceId);
    }
};

struct EndpointHash
{
    uint64_t operator()(const Endpoint& rhs) const
    {
        return (rhs.deviceId | (static_cast<uint64_t>(rhs.interfaceId) << 16) | (static_cast<uint64_t>(rhs.streamId) << 48));
    }
};

using DataPacketsPublisher = Publisher<Endpoint, IAsamCmpPacketsSubscriber, EndpointHash>;

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
