#include <asam_cmp_data_sink/stream_fb.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

StreamFb::StreamFb(const ContextPtr& ctx,
                   const ComponentPtr& parent,
                   const StringPtr& localId,
                   const asam_cmp_common_lib::StreamCommonInit& init)
    : StreamCommonFbImpl(ctx, parent, localId, init)
{
}

void StreamFb::processData(const std::shared_ptr<ASAM::CMP::Packet>& packet)
{
}

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
