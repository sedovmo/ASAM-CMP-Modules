#include <asam_cmp_data_sink/capture_fb.h>
#include <asam_cmp_data_sink/common.h>

#include <gtest/gtest.h>
#include <opendaq/context_factory.h>
#include <opendaq/module_ptr.h>
#include <opendaq/scheduler_factory.h>
#include <opendaq/search_filter_factory.h>

using namespace daq;
using namespace testing;

class InterfaceFbTest : public ::testing::Test
{
protected:
    InterfaceFbTest()
    {
        auto logger = Logger();
        captureFb = createWithImplementation<IFunctionBlock, modules::asam_cmp_data_sink_module::CaptureFb>(
            Context(Scheduler(logger), logger, TypeManager(), nullptr), nullptr, "capture_module_0", callsMultiMap);

        captureFb.getPropertyValue("AddInterface").execute();
        interfaceFb = captureFb.getFunctionBlocks().getItemAt(0);
    }

protected:
    FunctionBlockPtr captureFb;
    FunctionBlockPtr interfaceFb;
    modules::asam_cmp_data_sink_module::CallsMultiMap callsMultiMap;
};

TEST_F(InterfaceFbTest, NotNull)
{
    ASSERT_NE(interfaceFb, nullptr);
}

TEST_F(InterfaceFbTest, CaptureModuleProperties)
{
    ASSERT_TRUE(interfaceFb.hasProperty("InterfaceId"));
    ASSERT_TRUE(interfaceFb.hasProperty("PayloadType"));
    ASSERT_TRUE(interfaceFb.hasProperty("AddStream"));
    ASSERT_TRUE(interfaceFb.hasProperty("RemoveStream"));
}

TEST_F(InterfaceFbTest, InterfaceId)
{
    captureFb.getPropertyValue("AddInterface").execute();
    ASSERT_EQ(captureFb.getFunctionBlocks().getCount(), 2);

    const auto interfaceFb2 = captureFb.getFunctionBlocks().getItemAt(1);

    uint32_t id1 = interfaceFb.getPropertyValue("InterfaceId");
    uint32_t id2 = interfaceFb2.getPropertyValue("InterfaceId");
    ASSERT_NE(id1, id2);

    interfaceFb2.setPropertyValue("InterfaceId", id1);
    id1 = interfaceFb.getPropertyValue("InterfaceId");
    id2 = interfaceFb2.getPropertyValue("InterfaceId");
    ASSERT_EQ(id1, id2);

    interfaceFb2.setPropertyValue("InterfaceId", static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1);
    ASSERT_EQ(interfaceFb2.getPropertyValue("InterfaceId"), std::numeric_limits<uint32_t>::max());

    interfaceFb2.setPropertyValue("InterfaceId", id2 + 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(interfaceFb2.getPropertyValue("InterfaceId"), id2 + 1);
}

TEST_F(InterfaceFbTest, AddRemoveStream)
{
    ProcedurePtr addStream = interfaceFb.getPropertyValue("AddStream");
    interfaceFb.setPropertyValue("PayloadType", 1);
    addStream();
    addStream();

    auto s1 = interfaceFb.getFunctionBlocks().getItemAt(0), s2 = interfaceFb.getFunctionBlocks().getItemAt(1);

    ASSERT_NE(s1.getPropertyValue("StreamId"), s2.getPropertyValue("StreamId"));

    interfaceFb.getPropertyValue("RemoveStream").execute(0);

    ASSERT_EQ(interfaceFb.getFunctionBlocks().getCount(), 1);
}

TEST_F(InterfaceFbTest, TestBeginUpdateEndUpdate)
{
    const size_t oldInterfaceId = interfaceFb.getPropertyValue("InterfaceId");
    const size_t oldPayloadType = interfaceFb.getPropertyValue("PayloadType");

    const size_t interfaceId = (oldInterfaceId == 1 ? 2 : 1);
    const size_t payloadType = (oldPayloadType == 0 ? 1 : 0);

    const ProcedurePtr createProc = interfaceFb.getPropertyValue("AddStream");
    const ProcedurePtr removeProc = interfaceFb.getPropertyValue("RemoveStream");

    interfaceFb.beginUpdate();
    interfaceFb.setPropertyValue("InterfaceId", interfaceId);
    interfaceFb.setPropertyValue("PayloadType", payloadType);

    ASSERT_EQ(interfaceFb.getPropertyValue("InterfaceId"), oldInterfaceId);
    ASSERT_EQ(interfaceFb.getPropertyValue("PayloadType"), oldPayloadType);
    ASSERT_ANY_THROW(createProc());
    ASSERT_ANY_THROW(removeProc(0));
    interfaceFb.endUpdate();

    ASSERT_EQ(interfaceFb.getPropertyValue("InterfaceId"), interfaceId);
    ASSERT_EQ(interfaceFb.getPropertyValue("PayloadType"), payloadType);
    ASSERT_NO_THROW(createProc());
    ASSERT_NO_THROW(removeProc(0));
}

TEST_F(InterfaceFbTest, TestBeginUpdateEndUpdateWithWrongId)
{
    captureFb.getPropertyValue("AddInterface").execute();

    const auto interfaceFb2 = captureFb.getFunctionBlocks().getItemAt(1);
    const size_t id2 = interfaceFb2.getPropertyValue("InterfaceId");

    const size_t oldInterfaceId = interfaceFb.getPropertyValue("InterfaceId");
    const size_t oldPayloadType = interfaceFb.getPropertyValue("PayloadType");

    const size_t payloadType = (oldPayloadType == 0 ? 1 : 0);

    interfaceFb.beginUpdate();
    interfaceFb.setPropertyValue("InterfaceId", id2);
    interfaceFb.setPropertyValue("PayloadType", payloadType);

    ASSERT_EQ(interfaceFb.getPropertyValue("InterfaceId"), oldInterfaceId);
    ASSERT_EQ(interfaceFb.getPropertyValue("PayloadType"), oldPayloadType);
    interfaceFb.endUpdate();

    ASSERT_EQ(interfaceFb.getPropertyValue("InterfaceId"), id2);
    ASSERT_EQ(interfaceFb.getPropertyValue("PayloadType"), payloadType);
}
