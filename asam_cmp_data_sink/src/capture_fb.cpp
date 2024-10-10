#include <asam_cmp/capture_module_payload.h>

#include <asam_cmp_data_sink/capture_fb.h>
#include <asam_cmp_data_sink/interface_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

CaptureFb::CaptureFb(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId, CallsMultiMap& callsMap)
    : CaptureCommonFb(ctx, parent, localId)
    , callsMap(callsMap)
{
    initDeviceInfoProperties(true);
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
    initDeviceInfoProperties(true);
    setProperties();
    createFbs();
}

void CaptureFb::updateDeviceIdInternal()
{
    auto oldDeviceId = deviceId;
    CaptureCommonFb::updateDeviceIdInternal();
    if (oldDeviceId == deviceId)
        return;

    for (const FunctionBlockPtr& interfaceFb : functionBlocks.getItems())
    {
        uint32_t interfaceId = interfaceFb.getPropertyValue("InterfaceId");
        for (const auto& streamFb : interfaceFb.getFunctionBlocks())
        {
            Int streamId = streamFb.getPropertyValue("StreamId");
            auto handler = streamFb.as<IDataHandler>(true);
            callsMap.erase(oldDeviceId, interfaceId, streamId, handler);
            callsMap.insert(deviceId, interfaceId, streamId, handler);
        }
    }
}

void CaptureFb::addInterfaceInternal()
{
    auto interfaceId = interfaceIdManager.getFirstUnusedId();
    addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, callsMap);
}

void CaptureFb::removeInterfaceInternal(size_t nInd)
{
    FunctionBlockPtr interfaceFb = functionBlocks.getItems().getItemAt(nInd);
    uint32_t interfaceId = interfaceFb.getPropertyValue("InterfaceId");
    for (const auto& streamFb : interfaceFb.getFunctionBlocks())
    {
        Int streamId = streamFb.getPropertyValue("StreamId");
        auto handler = streamFb.as<IDataHandler>(true);
        callsMap.erase(deviceId, interfaceId, streamId, handler);
    }

    CaptureCommonFb::removeInterfaceInternal(nInd);
}

void CaptureFb::setProperties()
{
    objPtr.setPropertyValue("DeviceId", deviceStatus.getPacket().getDeviceId());

    auto& cmPayload = static_cast<const ASAM::CMP::CaptureModulePayload&>(deviceStatus.getPacket().getPayload());
    setPropertyValueInternal(String("DeviceDescription").asPtr<IString>(true),
                             String(cmPayload.getDeviceDescription().data()).asPtr<IString>(true),
                             false,
                             true,
                             false);
    setPropertyValueInternal(
        String("SerialNumber").asPtr<IString>(true), String(cmPayload.getSerialNumber().data()).asPtr<IString>(true), false, true, false);
    setPropertyValueInternal(String("HardwareVersion").asPtr<IString>(true),
                             String(cmPayload.getHardwareVersion().data()).asPtr<IString>(true),
                             false,
                             true,
                             false);
    setPropertyValueInternal(String("SoftwareVersion").asPtr<IString>(true),
                             String(cmPayload.getSoftwareVersion().data()).asPtr<IString>(true),
                             false,
                             true,
                             false);
    setPropertyValueInternal(String("VendorData").asPtr<IString>(true),
                             String(std::string(cmPayload.getVendorDataStringView())).asPtr<IString>(true),
                             false,
                             true,
                             false);
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
