/*
 * Copyright 2022-2023 Blueberry d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <asam_cmp/packet.h>

#include <asam_cmp_common_lib/stream_common_fb_impl.h>
#include <asam_cmp_data_sink/common.h>
#include <asam_cmp_data_sink/data_handler.h>

BEGIN_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE

#pragma pack(push, 1)
struct CANData
{
    uint32_t arbId;
    uint8_t length;
    uint8_t data[64];
};
#pragma pack(pop)

class StreamFb final : public asam_cmp_common_lib::StreamCommonFbImpl<IDataHandler>
{
public:
    explicit StreamFb(const ContextPtr& ctx,
                      const ComponentPtr& parent,
                      const StringPtr& localId,
                      const asam_cmp_common_lib::StreamCommonInit& init);
    ~StreamFb() override = default;

public:  // IStreamCommon
    void setPayloadType(PayloadType type) override;

protected:
    void processData(const std::shared_ptr<ASAM::CMP::Packet>& packet) override;

protected:
    void createSignals();
    void buildDataDescriptor();
    void buildDomainDescriptor();
    void buildCanDescriptor();
    StringPtr getEpoch() const;

private:
    SignalConfigPtr dataSignal;
    SignalConfigPtr domainSignal;
};

END_NAMESPACE_ASAM_CMP_DATA_SINK_MODULE
