#include <asam_cmp/capture_module_payload.h>

#include <asam_cmp_data_sink/capture_fb.h>
#include <asam_cmp_data_sink/interface_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

CaptureFb::CaptureFb(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId, DataPacketsPublisher& publisher)
    : CaptureCommonFb(ctx, parent, localId)
    , publisher(publisher)
{
    initDeviceInfoProperties(true);
}

CaptureFb::CaptureFb(const ContextPtr& ctx,
                     const ComponentPtr& parent,
                     const StringPtr& localId,
                     DataPacketsPublisher& publisher,
                     ASAM::CMP::DeviceStatus&& deviceStatus)
    : CaptureCommonFb(ctx, parent, localId)
    , deviceStatus(std::move(deviceStatus))
    , publisher(publisher)
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
            uint8_t streamId = static_cast<Int>(streamFb.getPropertyValue("StreamId"));
            auto handler = streamFb.as<IAsamCmpPacketsSubscriber>(true);
            publisher.unsubscribe({oldDeviceId, interfaceId, streamId}, handler);
            publisher.subscribe({deviceId, interfaceId, streamId}, handler);
        }
    }
}

void CaptureFb::addInterfaceInternal()
{
    auto interfaceId = interfaceIdManager.getFirstUnusedId();
    addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, publisher);
}

void CaptureFb::removeInterfaceInternal(size_t nInd)
{
    FunctionBlockPtr interfaceFb = functionBlocks.getItems().getItemAt(nInd);
    uint32_t interfaceId = interfaceFb.getPropertyValue("InterfaceId");
    for (const auto& streamFb : interfaceFb.getFunctionBlocks())
    {
        uint8_t streamId = static_cast<Int>(streamFb.getPropertyValue("StreamId"));
        auto handler = streamFb.as<IAsamCmpPacketsSubscriber>(true);
        publisher.unsubscribe({deviceId, interfaceId, streamId}, handler);
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
        addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, publisher, std::move(ifStatus));
    }
}

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
