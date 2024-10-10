#include <coreobjects/argument_info_factory.h>
#include <coreobjects/callable_info_factory.h>
#include <fmt/format.h>

#include <asam_cmp_common_lib/capture_common_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_COMMON

CaptureCommonFb::CaptureCommonFb(const ContextPtr& ctx, const ComponentPtr& parent, const StringPtr& localId)
    : FunctionBlock(CreateType(), ctx, parent, localId)
{
    initProperties();
}

FunctionBlockTypePtr CaptureCommonFb::CreateType()
{
    return FunctionBlockType("asam_cmp_capture", "AsamCmpCapture", "ASAM CMP Capture");
}

void CaptureCommonFb::initProperties()
{
    StringPtr propName = "DeviceId";
    auto prop = IntPropertyBuilder(propName, 0)
                    .setMinValue(static_cast<Int>(std::numeric_limits<uint16_t>::min()))
                    .setMaxValue(static_cast<Int>(std::numeric_limits<uint16_t>::max()))
                    .build();
    objPtr.addProperty(prop);
    objPtr.getOnPropertyValueWrite(propName) +=
        [this](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) { propertyChangedIfNotUpdating(); };

    propName = "AddInterface";
    prop = FunctionPropertyBuilder(propName, ProcedureInfo(List<IArgumentInfo>())).setReadOnly(true).build();
    objPtr.addProperty(prop);
    objPtr.asPtr<IPropertyObjectProtected>().setProtectedPropertyValue(propName, Procedure([this] { addInterface(); }));

    propName = "RemoveInterface";
    prop = FunctionPropertyBuilder(propName, ProcedureInfo(List<IArgumentInfo>(ArgumentInfo("nInd", ctInt)))).setReadOnly(true).build();
    objPtr.addProperty(prop);
    objPtr.asPtr<IPropertyObjectProtected>().setProtectedPropertyValue(propName, Procedure([this](IntPtr nInd) { removeInterface(nInd); }));
}

void CaptureCommonFb::initDeviceInfoProperties(bool readOnly)
{
    StringPtr propName = "DeviceDescription";
    auto prop = StringPropertyBuilder(propName, "").setReadOnly(readOnly).build();
    objPtr.addProperty(prop);
    objPtr.getOnPropertyValueWrite(propName) +=
        [this](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) { propertyChangedIfNotUpdating(); };

    propName = "SerialNumber";
    prop = StringPropertyBuilder(propName, "").setReadOnly(readOnly).build();
    objPtr.addProperty(prop);
    objPtr.getOnPropertyValueWrite(propName) +=
        [this](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) { propertyChangedIfNotUpdating(); };

    propName = "HardwareVersion";
    prop = StringPropertyBuilder(propName, "").setReadOnly(readOnly).build();
    objPtr.addProperty(prop);
    objPtr.getOnPropertyValueWrite(propName) +=
        [this](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) { propertyChangedIfNotUpdating(); };

    propName = "SoftwareVersion";
    prop = StringPropertyBuilder(propName, "").setReadOnly(readOnly).build();
    objPtr.addProperty(prop);
    objPtr.getOnPropertyValueWrite(propName) +=
        [this](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) { propertyChangedIfNotUpdating(); };

    propName = "VendorData";
    prop = StringPropertyBuilder(propName, "").setReadOnly(readOnly).build();
    objPtr.addProperty(prop);
    objPtr.getOnPropertyValueWrite(propName) +=
        [this](PropertyObjectPtr& obj, PropertyValueEventArgsPtr& args) { propertyChangedIfNotUpdating(); };
}

void CaptureCommonFb::addInterface()
{
    std::scoped_lock lock{sync};
    addInterfaceInternal();
}

void CaptureCommonFb::removeInterface(size_t nInd)
{
    std::scoped_lock lock{sync};
    removeInterfaceInternal(nInd);
}

void CaptureCommonFb::updateDeviceIdInternal()
{
    deviceId = objPtr.getPropertyValue("DeviceId");
}

void CaptureCommonFb::updateDeviceInfoInternal()
{
    deviceDescription = objPtr.getPropertyValue("DeviceDescription");
    serialNumber = objPtr.getPropertyValue("SerialNumber");
    hardwareVersion = objPtr.getPropertyValue("HardwareVersion");
    softwareVersion = objPtr.getPropertyValue("SoftwareVersion");
    vendorDataAsString = objPtr.getPropertyValue("VendorData").asPtr<IString>().toStdString();
    vendorData = std::vector<uint8_t>(begin(vendorDataAsString), end(vendorDataAsString));
}

void CaptureCommonFb::removeInterfaceInternal(size_t nInd)
{
    if (isUpdating)
        throw std::runtime_error("Removing interfaces is disabled during update");

    interfaceIdManager.removeId(functionBlocks.getItems().getItemAt(nInd).getPropertyValue("InterfaceId"));
    functionBlocks.removeItem(functionBlocks.getItems().getItemAt(nInd));
}

daq::ErrCode CaptureCommonFb::beginUpdate()
{
    daq::ErrCode result = FunctionBlock::beginUpdate();
    if (result == OPENDAQ_SUCCESS)
        isUpdating = true;

    return result;
}

void CaptureCommonFb::endApplyProperties(const UpdatingActions& propsAndValues, bool parentUpdating)
{
    std::scoped_lock lock{sync};
    FunctionBlock::endApplyProperties(propsAndValues, parentUpdating);

    if (needsPropertyChanged)
    {
        propertyChanged();
        needsPropertyChanged = false;
    }

    isUpdating = false;
}

void CaptureCommonFb::propertyChanged()
{
    updateDeviceIdInternal();
    updateDeviceInfoInternal();
}

void CaptureCommonFb::propertyChangedIfNotUpdating()
{
    if (!isUpdating)
    {
        std::scoped_lock lock{sync};
        propertyChanged();
    }
    else
        needsPropertyChanged = true;
}

END_NAMESPACE_ASAM_CMP_COMMON
