//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "ClSpaceToBatchNdWorkload.hpp"

#include "ClWorkloadUtils.hpp"

#include <aclCommon/ArmComputeUtils.hpp>
#include <aclCommon/ArmComputeTensorUtils.hpp>
#include <backendsCommon/CpuTensorHandle.hpp>
#include <cl/ClLayerSupport.hpp>
#include <cl/ClTensorHandle.hpp>
#include <cl/ClLayerSupport.hpp>

#include <boost/polymorphic_pointer_cast.hpp>

namespace armnn
{
using namespace armcomputetensorutils;

arm_compute::Status ClSpaceToBatchNdWorkloadValidate(const TensorInfo& input,
                                                     const TensorInfo& output,
                                                     const SpaceToBatchNdDescriptor& descriptor)
{
    const arm_compute::TensorInfo aclInputInfo  = BuildArmComputeTensorInfo(input, descriptor.m_DataLayout);
    const arm_compute::TensorInfo aclOutputInfo = BuildArmComputeTensorInfo(output, descriptor.m_DataLayout);

    // ArmNN blockShape is [H, W] Cl asks for W, H
    int32_t blockHeight = boost::numeric_cast<int32_t>(descriptor.m_BlockShape[0]);
    int32_t blockWidth  = boost::numeric_cast<int32_t>(descriptor.m_BlockShape[1]);

    // ACL expects paddingTopBottom [pad_top, pad_bottom] and paddingLeftRight [pad_left, pad_right]
    arm_compute::Size2D paddingLeftRight = BuildArmComputeSize2D(
        descriptor.m_PadList[0].first, descriptor.m_PadList[0].second);
    arm_compute::Size2D paddingTopBottom  = BuildArmComputeSize2D(
        descriptor.m_PadList[1].first, descriptor.m_PadList[1].second);

    return arm_compute::CLSpaceToBatchLayer::validate(&aclInputInfo,
                                                      blockWidth,
                                                      blockHeight,
                                                      paddingTopBottom,
                                                      paddingLeftRight,
                                                      &aclOutputInfo);
}

ClSpaceToBatchNdWorkload::ClSpaceToBatchNdWorkload(
    const SpaceToBatchNdQueueDescriptor& descriptor, const WorkloadInfo& info)
    : BaseWorkload<SpaceToBatchNdQueueDescriptor>(descriptor, info)
{
    m_Data.ValidateInputsOutputs("ClSpaceToBatchNdWorkload", 1, 1);

    arm_compute::ICLTensor& input  =
        boost::polymorphic_pointer_downcast<IClTensorHandle>(m_Data.m_Inputs[0])->GetTensor();
    arm_compute::ICLTensor& output =
        boost::polymorphic_pointer_downcast<IClTensorHandle>(m_Data.m_Outputs[0])->GetTensor();

    // ArmNN blockShape is [H, W] Cl asks for W, H
    int32_t blockHeight = boost::numeric_cast<int32_t>(m_Data.m_Parameters.m_BlockShape[0]);
    int32_t blockWidth  = boost::numeric_cast<int32_t>(m_Data.m_Parameters.m_BlockShape[1]);

    // ACL expects paddingTopBottom [pad_top, pad_bottom] and paddingLeftRight [pad_left, pad_right]
    arm_compute::Size2D paddingLeftRight = BuildArmComputeSize2D(
        m_Data.m_Parameters.m_PadList[0].first, m_Data.m_Parameters.m_PadList[0].second);
    arm_compute::Size2D paddingTopBottom  = BuildArmComputeSize2D(
        m_Data.m_Parameters.m_PadList[1].first, m_Data.m_Parameters.m_PadList[1].second);

    arm_compute::DataLayout aclDataLayout = ConvertDataLayout(m_Data.m_Parameters.m_DataLayout);
    input.info()->set_data_layout(aclDataLayout);
    output.info()->set_data_layout(aclDataLayout);

    m_SpaceToBatchLayer.configure(&input,
                                  blockWidth,
                                  blockHeight,
                                  paddingTopBottom,
                                  paddingLeftRight,
                                  &output);
}

void ClSpaceToBatchNdWorkload::Execute() const
{
    ARMNN_SCOPED_PROFILING_EVENT_CL("ClSpaceToBatchNdWorkload_Execute");
    RunClFunction(m_SpaceToBatchLayer, CHECK_LOCATION());
}

} //namespace armnn
