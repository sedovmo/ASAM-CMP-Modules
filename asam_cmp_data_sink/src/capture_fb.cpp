#include <asam_cmp/capture_module_payload.h>

#include <asam_cmp_data_sink/capture_fb.h>
#include <asam_cmp_data_sink/interface_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

CaptureFb::CaptureFb(const ContextPtr& ctx,
                     const ComponentPtr& parent,
                     const StringPtr& localId,
                     DataPacketsPublisher& dataPacketsPublisher,
                     CapturePacketsPublisher& capturePacketsPublisher)
    : CaptureCommonFbImpl(ctx, parent, localId)
    , dataPacketsPublisher(dataPacketsPublisher)
    , capturePacketsPublisher(capturePacketsPublisher)
{
    initDeviceInfoProperties(true);
}

CaptureFb::CaptureFb(const ContextPtr& ctx,
                     const ComponentPtr& parent,
                     const StringPtr& localId,
                     DataPacketsPublisher& dataPacketsPublisher,
                     CapturePacketsPublisher& capturePacketsPublisher,
                     ASAM::CMP::DeviceStatus&& deviceStatus)
    : CaptureCommonFbImpl(ctx, parent, localId)
    , dataPacketsPublisher(dataPacketsPublisher)
    , capturePacketsPublisher(capturePacketsPublisher)
    , deviceStatus(std::move(deviceStatus))
{
    initDeviceInfoProperties(true);
    setProperties();
    createFbs();
}

void CaptureFb::receive(const std::shared_ptr<ASAM::CMP::Packet>& packet)
{
    setDeviceInfoProperties(*packet);
}

void CaptureFb::updateDeviceIdInternal()
{
    auto oldDeviceId = deviceId;
    CaptureCommonFbImpl::updateDeviceIdInternal();
    if (oldDeviceId == deviceId)
        return;

    capturePacketsPublisher.unsubscribe(oldDeviceId, this);
    capturePacketsPublisher.subscribe(deviceId, this);

    for (const FunctionBlockPtr& interfaceFb : functionBlocks.getItems())
    {
        uint32_t interfaceId = interfaceFb.getPropertyValue("InterfaceId");
        for (const auto& streamFb : interfaceFb.getFunctionBlocks())
        {
            uint8_t streamId = static_cast<Int>(streamFb.getPropertyValue("StreamId"));
            auto handler = streamFb.as<IAsamCmpPacketsSubscriber>(true);
            dataPacketsPublisher.unsubscribe({oldDeviceId, interfaceId, streamId}, handler);
            dataPacketsPublisher.subscribe({deviceId, interfaceId, streamId}, handler);
        }
    }
}

void CaptureFb::addInterfaceInternal()
{
    auto interfaceId = interfaceIdManager.getFirstUnusedId();
    addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, dataPacketsPublisher);
}

void CaptureFb::removeInterfaceInternal(size_t nInd)
{
    FunctionBlockPtr interfaceFb = functionBlocks.getItems().getItemAt(nInd);
    uint32_t interfaceId = interfaceFb.getPropertyValue("InterfaceId");
    for (const auto& streamFb : interfaceFb.getFunctionBlocks())
    {
        uint8_t streamId = static_cast<Int>(streamFb.getPropertyValue("StreamId"));
        auto handler = streamFb.as<IAsamCmpPacketsSubscriber>(true);
        dataPacketsPublisher.unsubscribe({deviceId, interfaceId, streamId}, handler);
    }

    CaptureCommonFbImpl::removeInterfaceInternal(nInd);
}

void CaptureFb::setProperties()
{
    objPtr.setPropertyValue("DeviceId", deviceStatus.getPacket().getDeviceId());

    setDeviceInfoProperties(deviceStatus.getPacket());
}

void CaptureFb::setDeviceInfoProperties(const ASAM::CMP::Packet& packet)
{
    auto& cmPayload = static_cast<const ASAM::CMP::CaptureModulePayload&>(packet.getPayload());

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
        addInterfaceWithParams<InterfaceFb>(interfaceId, deviceId, dataPacketsPublisher, std::move(ifStatus));
    }
}

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
