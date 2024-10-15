#pragma once
#include <asam_cmp_data_sink/common.h>
#include <asam_cmp_data_sink/publisher.h>
#include <asam_cmp_data_sink/asam_cmp_packets_subscriber.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

using CapturePacketsPublisher = Publisher<uint16_t, IAsamCmpPacketsSubscriber>;

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
