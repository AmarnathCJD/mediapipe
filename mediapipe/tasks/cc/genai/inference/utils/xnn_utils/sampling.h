// Copyright 2024 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MEDIAPIPE_TASKS_GENAI_INFERENCE_UTILS_XNN_UTILS_SAMPLING_H_
#define MEDIAPIPE_TASKS_GENAI_INFERENCE_UTILS_XNN_UTILS_SAMPLING_H_

#include <sys/stat.h>

#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "mediapipe/tasks/cc/genai/inference/utils/xnn_utils/xnn_tensor.h"

namespace mediapipe::tasks::genai::xnn_utils {

class Sampler {
 public:
  enum class Type { kGreedy, kTopK, kTopP };

  // Creates a Sampler.
  // * If kGreedy sampler is used, Argmax will be returned ignoring all other
  //   arguments provided.
  // * If kTopK sampler is used, the top k logit values are selected. That is
  //   followed by temperature scaling and applying softmax. Finally, a sampled
  //   is drawn from the resulting distribution.
  // * If kTopP sampler is selcted, the top k logits are first selcted if k > 0.
  //   Otherwise, k = vocab size. This is followed by temperature scaling and
  //   applying softmax. Finally, the top p are selcted from the probabilities
  //   such that sum of p_i is greater than or equal to top_p. Lastly, a sample
  //   is drawn from the resulting distribution.
  static absl::StatusOr<std::unique_ptr<Sampler>> Create(Type type, int top_k,
                                                         float top_p,
                                                         float temperature,
                                                         int seed);
  // Given an input tensor of shape `(Batch, 1 [seq_len], vocab_size)`, runs
  // the configured sampling alogorithm to find a winning class. The results are
  // reported as a vector of integer indicies where each entry corresponds to a
  // batch.
  absl::StatusOr<std::vector<int>> Sample(Tensor& logits);

 private:
  Sampler(Type type, int top_k, float top_p, float temperature, int seed);
  absl::StatusOr<std::vector<int>> SampleGreedy(Tensor& logits);
  absl::StatusOr<std::vector<int>> SampleTopK(Tensor& logits);
  absl::StatusOr<std::vector<int>> SampleTopP(Tensor& logits);
  absl::Status SelectTopK(std::vector<std::pair<float, int>>& logits_ids,
                          int k);
  // `logits_ids` must be sorted and normalized.
  absl::Status SelectTopP(std::vector<std::pair<float, int>>& logits_ids,
                          float p);
  // `logits_ids` must be sorted.
  absl::Status ScaledSoftmax(std::vector<std::pair<float, int>>& logits_ids,
                             bool normalize);
  absl::StatusOr<int> DoSampling(
      std::vector<std::pair<float, int>>& logits_ids);

  Type type_;
  int top_k_;
  float top_p_;
  float temperature_;
  std::unique_ptr<std::mt19937> generator_;
};

}  // namespace mediapipe::tasks::genai::xnn_utils
#endif  // MEDIAPIPE_TASKS_GENAI_INFERENCE_UTILS_XNN_UTILS_SAMPLING_H_
