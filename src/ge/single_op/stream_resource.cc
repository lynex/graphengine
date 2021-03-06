/**
 * Copyright 2019-2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "single_op/stream_resource.h"

#include "framework/common/debug/ge_log.h"
#include "framework/common/debug/log.h"
#include "runtime/rt.h"
#include "single_op/single_op_model.h"

namespace ge {
StreamResource::StreamResource(uintptr_t resource_id) : resource_id_(resource_id) {}

StreamResource::~StreamResource() {
  for (auto mem : memory_list_) {
    if (mem != nullptr) {
      auto rt_ret = rtFree(mem);
      GE_IF_BOOL_EXEC(rt_ret != RT_ERROR_NONE, GELOGE(RT_FAILED, "rtFree failed"));
    }
  }

  for (auto weight : weight_list_) {
    if (weight != nullptr) {
      auto rt_ret = rtFree(weight);
      GE_IF_BOOL_EXEC(rt_ret != RT_ERROR_NONE, GELOGE(RT_FAILED, "rtFree failed"));
    }
  }
}

SingleOp *StreamResource::GetOperator(const void *key) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = op_map_.find(key);
  if (it == op_map_.end()) {
    return nullptr;
  }

  return it->second.get();
}

DynamicSingleOp *StreamResource::GetDynamicOperator(const void *key) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = dynamic_op_map_.find(key);
  if (it == dynamic_op_map_.end()) {
    return nullptr;
  }

  return it->second.get();
}

void StreamResource::SetStream(rtStream_t stream) { stream_ = stream; }

uint8_t *StreamResource::DoMallocMemory(const std::string &purpose, size_t size, size_t &max_allocated,
                                        std::vector<uint8_t *> &allocated) {
  if (size <= max_allocated && !allocated.empty()) {
    GELOGD("reuse last memory");
    return allocated.back();
  }

  uint8_t *buffer = nullptr;
  auto ret = rtMalloc(reinterpret_cast<void **>(&buffer), size, RT_MEMORY_HBM);
  if (ret != RT_ERROR_NONE) {
    GELOGE(RT_FAILED, "rtMalloc failed, size = %zu, ret = %d", size, ret);
    return nullptr;
  }
  GE_PRINT_DYNAMIC_MEMORY(rtMalloc, purpose.c_str(), size)

  ret = rtMemset(buffer, size, 0U, size);
  if (ret != RT_ERROR_NONE) {
    GELOGE(RT_FAILED, "rtMemset failed, ret = %d", ret);
    auto rt_ret = rtFree(buffer);
    GE_IF_BOOL_EXEC(rt_ret != RT_ERROR_NONE, GELOGE(RT_FAILED, "rtFree failed"));
    return nullptr;
  }

  GELOGD("Malloc new memory succeeded. size = %zu", size);
  max_allocated = size;
  allocated.emplace_back(buffer);
  return buffer;
}

uint8_t *StreamResource::MallocMemory(const std::string &purpose, size_t size) {
  GELOGD("To Malloc memory, size = %zu", size);
  uint8_t *buffer = DoMallocMemory(purpose, size, max_memory_size_, memory_list_);
  return buffer;
}

uint8_t *StreamResource::MallocWeight(const std::string &purpose, size_t size) {
  GELOGD("To Malloc weight, size = %zu", size);
  uint8_t *buffer = nullptr;
  auto ret = rtMalloc(reinterpret_cast<void **>(&buffer), size, RT_MEMORY_HBM);
  if (ret != RT_ERROR_NONE) {
    GELOGE(RT_FAILED, "rtMalloc failed, size = %zu, ret = %d", size, ret);
    return nullptr;
  }

  GE_PRINT_DYNAMIC_MEMORY(rtMalloc, purpose.c_str(), size)
  weight_list_.emplace_back(buffer);
  return buffer;
}

Status StreamResource::BuildDynamicOperator(const string &model_name, const ModelData &model_data,
                                            DynamicSingleOp **single_op) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = dynamic_op_map_.find(model_data.model_data);
  if (it != dynamic_op_map_.end()) {
    *single_op = it->second.get();
    return SUCCESS;
  }

  SingleOpModel model(model_name, model_data.model_data, model_data.model_len);
  auto ret = model.Init();
  if (ret != SUCCESS) {
    GELOGE(ret, "Init model failed. model = %s, ret = %u", model_name.c_str(), ret);
    return ret;
  }

  auto new_op =
    std::unique_ptr<DynamicSingleOp>(new (std::nothrow) DynamicSingleOp(resource_id_, &stream_mu_, stream_));
  GE_CHECK_NOTNULL(new_op);

  GELOGI("To build operator: %s", model_name.c_str());
  GE_CHK_STATUS_RET(model.BuildDynamicOp(*new_op), "Build op failed. op = %s, ret = %u", model_name.c_str(), ret);
  *single_op = new_op.get();
  dynamic_op_map_[model_data.model_data] = std::move(new_op);
  return SUCCESS;
}

Status StreamResource::BuildOperator(const string &model_name, const ModelData &model_data, SingleOp **single_op) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = op_map_.find(model_data.model_data);
  if (it != op_map_.end()) {
    *single_op = it->second.get();
    return SUCCESS;
  }

  SingleOpModel model(model_name, model_data.model_data, model_data.model_len);
  auto ret = model.Init();
  if (ret != SUCCESS) {
    GELOGE(ret, "Init model failed. model = %s, ret = %u", model_name.c_str(), ret);
    return ret;
  }

  auto new_op = std::unique_ptr<SingleOp>(new (std::nothrow) SingleOp(&stream_mu_, stream_));
  if (new_op == nullptr) {
    GELOGE(MEMALLOC_FAILED, "new SingleOp failed");
    return MEMALLOC_FAILED;
  }

  GELOGI("To build operator: %s", model_name.c_str());
  GE_CHK_STATUS_RET(model.BuildOp(*this, *new_op), "Build op failed. op = %s, ret = %u", model_name.c_str(), ret);

  *single_op = new_op.get();
  op_map_[model_data.model_data] = std::move(new_op);
  return SUCCESS;
}
}  // namespace ge
