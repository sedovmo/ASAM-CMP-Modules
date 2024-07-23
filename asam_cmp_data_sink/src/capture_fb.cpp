#include <asam_cmp_data_sink/capture_fb.h>
#include <asam_cmp_data_sink/interface_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

CaptureFb::CaptureFb(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId, CallsMultiMap& callsMap)
    : CaptureCommonFb(ctx, parent, localId)
    , callsMap(callsMap)
{
}

CaptureFb::CaptureFb(const ContextPtr& ctx,
                     const ComponentPtr& parent,
                     const StringPtr& localId,
                     CallsMultiMap& callsMap,
                     ASAM::CMP::DeviceStatus&& deviceStatus)
    : CaptureCommonFb(ctx, parent, localId)
    , deviceStatus(std::move(deviceStatus))
    , callsMap(callsMap)
{
    createFbs();
}

void CaptureFb::addInterfaceInternal()
{
    auto interfaceId = interfaceIdManager.getFirstUnusedId();
    addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, callsMap);
}

void CaptureFb::createFbs()
{
    for (size_t i = 0; i < deviceStatus.getInterfaceStatusCount(); ++i)
    {
        auto ifStatus = deviceStatus.getInterfaceStatus(i);
        auto interfaceId = ifStatus.getInterfaceId();
        addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, callsMap, std::move(ifStatus));
    }
}

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
