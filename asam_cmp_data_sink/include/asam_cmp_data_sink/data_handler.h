#pragma once
#include <asam_cmp/packet.h>
#include <coretypes/baseobject.h>
#include <memory>

#include <asam_cmp_data_sink/common.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

DECLARE_OPENDAQ_INTERFACE(IDataHandler, IBaseObject)
{
    virtual void processData(const std::shared_ptr<ASAM::CMP::Packet>& packet) = 0;
};

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
