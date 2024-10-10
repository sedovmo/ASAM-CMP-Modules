#include <asam_cmp_data_sink/common.h>

#include <gtest/gtest.h>
#include <opendaq/context_factory.h>
#include <opendaq/module_ptr.h>
#include <opendaq/scheduler_factory.h>
#include <opendaq/search_filter_factory.h>

#include <asam_cmp_data_sink/capture_fb.h>

using namespace daq;

class CaptureFbTest : public testing::Test
{
protected:
    CaptureFbTest()
    {
        auto logger = Logger();
        captureFb = createWithImplementation<IFunctionBlock, modules::asam_cmp_data_sink_module::CaptureFb>(
            Context(Scheduler(logger), logger, nullptr, nullptr, nullptr), nullptr, "capture_module_0", callsMultiMap);
    }

protected:
    modules::asam_cmp_data_sink_module::CallsMultiMap callsMultiMap;
    FunctionBlockPtr captureFb;
};

TEST_F(CaptureFbTest, CreateCaptureModule)
{
    ASSERT_NE(captureFb, nullptr);
}

TEST_F(CaptureFbTest, FunctionBlockType)
{
    auto type = captureFb.getFunctionBlockType();
    ASSERT_EQ(type.getId(), "asam_cmp_capture");
    ASSERT_EQ(type.getName(), "AsamCmpCapture");
    ASSERT_EQ(type.getDescription(), "ASAM CMP Capture");
}

TEST_F(CaptureFbTest, CaptureModuleProperties)
{
    ASSERT_TRUE(captureFb.hasProperty("DeviceId"));
    ASSERT_TRUE(captureFb.hasProperty("AddInterface"));
    ASSERT_TRUE(captureFb.hasProperty("RemoveInterface"));
}

TEST_F(CaptureFbTest, DeviceIdProperty)
{
    int id = captureFb.getPropertyValue("DeviceId");
    auto newId = id + 1;
    captureFb.setPropertyValue("DeviceId", newId);
    ASSERT_EQ(captureFb.getPropertyValue("DeviceId"), newId);
}

TEST_F(CaptureFbTest, DeviceIdMin)
{
    constexpr auto minValue = static_cast<Int>(std::numeric_limits<uint16_t>::min());
    captureFb.setPropertyValue("DeviceId", minValue - 1);
    ASSERT_EQ(static_cast<Int>(captureFb.getPropertyValue("DeviceId")), minValue);
}

TEST_F(CaptureFbTest, DeviceIdMax)
{
    constexpr auto newMax = static_cast<Int>(std::numeric_limits<uint16_t>::max()) + 1;
    constexpr auto maxMax = static_cast<Int>(std::numeric_limits<uint16_t>::max());
    captureFb.setPropertyValue("DeviceId", newMax);
    ASSERT_EQ(static_cast<Int>(captureFb.getPropertyValue("DeviceId")), maxMax);
}

TEST_F(CaptureFbTest, TestCreateInterface)
{
    ProcedurePtr createProc = captureFb.getPropertyValue("AddInterface");

    ASSERT_EQ(captureFb.getFunctionBlocks().getCount(), 0);
    createProc();
    createProc();
    ASSERT_EQ(captureFb.getFunctionBlocks().getCount(), 2);

    int lstId = captureFb.getFunctionBlocks().getItemAt(1).getPropertyValue("InterfaceId");

    ProcedurePtr removeProc = captureFb.getPropertyValue("RemoveInterface");
    removeProc(0);

    ASSERT_EQ(captureFb.getFunctionBlocks().getCount(), 1);
    ASSERT_EQ(captureFb.getFunctionBlocks().getItemAt(0).getPropertyValue("InterfaceId"), lstId);
}

TEST_F(CaptureFbTest, TestBeginUpdateEndUpdate)
{
    uint32_t oldDeviceId = captureFb.getPropertyValue("DeviceId");

    size_t deviceId = (oldDeviceId == 1 ? 2 : 1);

    ProcedurePtr createProc = captureFb.getPropertyValue("AddInterface");
    ProcedurePtr removeProc = captureFb.getPropertyValue("RemoveInterface");

    captureFb.beginUpdate();
    captureFb.setPropertyValue("DeviceId", deviceId);

    ASSERT_EQ(captureFb.getPropertyValue("DeviceId"), oldDeviceId);

    ASSERT_ANY_THROW(createProc());
    ASSERT_ANY_THROW(removeProc(0));
    captureFb.endUpdate();

    ASSERT_EQ(captureFb.getPropertyValue("DeviceId"), deviceId);

    ASSERT_NO_THROW(createProc());
    ASSERT_NO_THROW(removeProc(0));
}
