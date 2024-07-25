#include <asam_cmp_data_sink/common.h>
#include <asam_cmp_data_sink/module_dll.h>

#include <gtest/gtest.h>
#include <opendaq/context_factory.h>
#include <opendaq/module_ptr.h>
#include <opendaq/scheduler_factory.h>
#include <opendaq/search_filter_factory.h>

using namespace daq;

class AsamCmpStreamTest : public testing::Test
{
protected:
    void SetUp()
    {
        auto logger = Logger();
        createModule(&module, Context(Scheduler(logger), logger, nullptr, nullptr, nullptr));

        auto fbs = module.getAvailableFunctionBlockTypes();
        if (fbs.hasKey("asam_cmp_capture"))
        {
            rootFb = module.createFunctionBlock("asam_cmp_capture", nullptr, "id");
            captureModule = rootFb.getFunctionBlocks().getItemAt(0);
        }
        else
        {
            rootFb = module.createFunctionBlock("asam_cmp_data_sink_module", nullptr, "id");
            dataSink = rootFb.getFunctionBlocks(search::Recursive(search::LocalId("asam_cmp_data_sink")))[0];
            dataSink.getPropertyValue("AddCaptureModuleEmpty").execute();
        }
        captureModule = rootFb.getFunctionBlocks(search::Recursive(search::LocalId("capture_module_0")))[0];

        ProcedurePtr createProc = captureModule.getPropertyValue("AddInterface");
        createProc();
        interface = captureModule.getFunctionBlocks().getItemAt(0);
    }

protected:
    ModulePtr module;
    FunctionBlockPtr rootFb;
    FunctionBlockPtr dataSink;
    FunctionBlockPtr captureModule;
    FunctionBlockPtr interface;
};

TEST_F(AsamCmpStreamTest, CreateStream)
{
    ProcedurePtr createProc = interface.getPropertyValue("AddStream");
    interface.setPropertyValue("PayloadType", 1);
    createProc();
    createProc();

    auto s1 = interface.getFunctionBlocks().getItemAt(0), s2 = interface.getFunctionBlocks().getItemAt(1);

    ASSERT_NE(s1.getPropertyValue("StreamId"), s2.getPropertyValue("StreamId"));
}
