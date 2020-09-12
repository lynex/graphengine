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

/*!
 * \file boosted_trees_ops.h
 * \brief
 */
#ifndef GE_OP_BOOSTED_TREES_OPS_H_
#define GE_OP_BOOSTED_TREES_OPS_H_

#include "graph/operator_reg.h"

namespace ge {

/**
*@brief Bucketizes each feature based on bucket boundaries.

*@par Inputs: 
*Input "float_values" is a 1D tensor. Input "bucket_boundaries" is \n
a list of 1D tensors.
* @li float_values: A list of rank 1 tensors each containing float \n
values for a single feature.
* @li bucket_boundaries: A list of rank 1 tensors each containing \n
the bucket boundaries for a single feature.

*@par Attributes: 
*@li num_features: Number of features \n

*@par Outputs: 
*@li y: A list of rank 1 tensors each containing the bucketized values for \n
a single feature.

*@attention Constraints: \n
*BoostedTreesBucketize runs on the Ascend AI CPU, which delivers poor performance. \n

*@par Third-party framework compatibility
*Compatible with the TensorFlow operator BoostedTreesBucketize.

*@par Restrictions:
*Warning: THIS FUNCTION IS EXPERIMENTAL. Please do not use.
*/
REG_OP(BoostedTreesBucketize)
    .DYNAMIC_INPUT(float_values, TensorType({DT_FLOAT}))
    .DYNAMIC_INPUT(bucket_boundaries, TensorType({DT_FLOAT}))
    .DYNAMIC_OUTPUT(y, TensorType({DT_INT32}))
    .REQUIRED_ATTR(num_features, Int)
    .OP_END_FACTORY_REG(BoostedTreesBucketize)

}  // namespace ge

#endif  // GE_OP_BOOSTED_TREES_OPS_H_
