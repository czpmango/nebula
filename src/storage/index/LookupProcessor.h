/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef STORAGE_QUERY_LOOKUP_H_
#define STORAGE_QUERY_LOOKUP_H_

#include "common/base/Base.h"
#include "storage/index/LookupBaseProcessor.h"

namespace nebula {
namespace storage {

extern ProcessorCounters kLookupCounters;

class LookupProcessor
    : public LookupBaseProcessor<cpp2::LookupIndexRequest, cpp2::LookupIndexResp> {
 public:
  static LookupProcessor* instance(StorageEnv* env,
                                   const ProcessorCounters* counters = &kLookupCounters,
                                   folly::Executor* executor = nullptr,
                                   std::unordered_map<std::string, Value> paramMap = {}) {
    return new LookupProcessor(env, counters, executor, paramMap);
  }

  void process(const cpp2::LookupIndexRequest& req) override;

  // const std::unordered_map<std::string, Value>& parameterMap() const { return paramMap_; }

  // void setParameterMap(std::unordered_map<std::string, Value> params) {
  //   paramMap_ = std::move(params);
  // }

 protected:
  LookupProcessor(StorageEnv* env,
                  const ProcessorCounters* counters,
                  folly::Executor* executor,
                  std::unordered_map<std::string, Value> paramMap = {})
      : LookupBaseProcessor<cpp2::LookupIndexRequest, cpp2::LookupIndexResp>(
            env, counters, executor, paramMap) {}

  void onProcessFinished() override;

 private:
  void runInSingleThread(const cpp2::LookupIndexRequest& req);
  void runInMultipleThread(const cpp2::LookupIndexRequest& req);

  folly::Future<std::pair<nebula::cpp2::ErrorCode, PartitionID>> runInExecutor(
      IndexFilterItem* filterItem, nebula::DataSet* result, PartitionID partId);

  void doProcess(const cpp2::LookupIndexRequest& req);

  //  private:
  //   std::unordered_map<std::string, Value> paramMap_;
};

}  // namespace storage
}  // namespace nebula
#endif  // STORAGE_QUERY_LOOKUP_H_
