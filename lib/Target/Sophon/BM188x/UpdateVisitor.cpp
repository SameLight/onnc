//===- UpdateVisitor.cpp --------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "UpdateVisitor.h"
#include "Compute/Conv.h"
#include "Compute/Gemm.h"
#include "Compute/LRN.h"
#include "Compute/MaxPool.h"
#include "Compute/PRelu.h"
#include <onnc/IR/Compute/Tensor.h>

using namespace onnc;
using namespace onnc::BM188X;

//===----------------------------------------------------------------------===//
// UpdateVisitor
//===----------------------------------------------------------------------===//

UpdateVisitor::UpdateVisitor(const BM1880Backend *pBackend)
    : m_pBackend(pBackend)
{
}

const BM1880Backend::LayerCtable *
UpdateVisitor::getLayerCtable(const ComputeOperator *node)
{
  auto ctableName = node->getOutput(0)->getName();
  return m_pBackend->getLayerCtable(ctableName);
}

void UpdateVisitor::visit(BM188X::Conv &pConv)
{
  const auto *layerCtable = getLayerCtable(&pConv);
  pConv.setRShiftWidth(layerCtable->right_shift_width());
  pConv.setScaleRShiftWidth(
      layerCtable->convolution_param().scale_right_shift_width());
}

void UpdateVisitor::visit(BM188X::MaxPool &pMaxPool)
{
  const auto *layerCtable = getLayerCtable(&pMaxPool);
  pMaxPool.setRShiftWidth(layerCtable->right_shift_width());
  pMaxPool.setThresholdXQuantized(*layerCtable->threshold_x_quantized().data());
}

void UpdateVisitor::visit(BM188X::Gemm &pGemm)
{
  const auto *layerCtable = getLayerCtable(&pGemm);
  pGemm.setRShiftWidth(layerCtable->right_shift_width());
}

void UpdateVisitor::visit(BM188X::LRN &pLRN)
{
  auto outputName = pLRN.getOutput(0)->getName();
  const auto *layerCtable = getLayerCtable(&pLRN);
  for (int i = 0; i < layerCtable->blob_param_size(); ++i) {
    if (layerCtable->blob_param(i).name() == outputName) {
      pLRN.setLrnRightShiftWidth(
          layerCtable->blob_param(i).right_shift_width());
    } else if (layerCtable->blob_param(i).name() == "sum_sq") {
      pLRN.setSumRightShiftWidth(
          layerCtable->blob_param(i).right_shift_width());
    }
  }
  pLRN.getThresholdXQuantized()[0] = layerCtable->threshold_x_quantized(0);
  pLRN.getThresholdXQuantized()[1] = layerCtable->threshold_x_quantized(1);
}

void UpdateVisitor::visit(BM188X::PRelu &pPRelu)
{
  const auto *layerCtable = getLayerCtable(&pPRelu);
  const auto &lct_prelu = layerCtable->prelu_param();
  pPRelu.setGTRShiftWidth(lct_prelu.gt_right_shift_width());
  pPRelu.setLERShiftWidth(lct_prelu.le_right_shift_width());
  pPRelu.setGTScale(lct_prelu.gt_scale());

  // get slope tensor to determine ChannelShared and Slope
  const ::onnc::Value *value = pPRelu.getInput(1);
  const auto *tensor = dynamic_cast<const onnc::Int8Tensor *>(value);
  std::vector<int8_t> tensorValues = tensor->getValues();
  pPRelu.setChannelShared(tensorValues.size() == 1);
  pPRelu.setSlope(tensorValues[0]);
}
