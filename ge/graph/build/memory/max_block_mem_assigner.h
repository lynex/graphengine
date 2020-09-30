/**
 * Copyright 2020 Huawei Technologies Co., Ltd
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

#ifndef GE_GRAPH_BUILD_MEMORY_MAX_BLOCK_MEM_ASSIGNER_H_
#define GE_GRAPH_BUILD_MEMORY_MAX_BLOCK_MEM_ASSIGNER_H_
#include <utility>
#include <vector>
#include "graph/build/memory/block_mem_assigner.h"

namespace ge {
class MaxBlockMemAssigner : public BlockMemAssigner {
 public:
  MaxBlockMemAssigner(ComputeGraphPtr compute_graph, const std::map<std::string, std::string> &anchor_to_symbol,
                      const std::map<std::string, std::list<NodeIndexIO>> &symbol_to_anchors)
      : BlockMemAssigner(std::move(compute_graph), anchor_to_symbol, symbol_to_anchors) {}

  MaxBlockMemAssigner(const MaxBlockMemAssigner &) = delete;

  MaxBlockMemAssigner &operator=(const MaxBlockMemAssigner &) = delete;

  ~MaxBlockMemAssigner() override = default;

  Status GetMemoryRanges(std::vector<int64_t> &ranges) override;
};
}  // namespace ge
#endif  // GE_GRAPH_BUILD_MEMORY_MAX_BLOCK_MEM_ASSIGNER_H_
