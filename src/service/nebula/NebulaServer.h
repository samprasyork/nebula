/*
 * Copyright 2017-present Shawn Cao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <grpcpp/grpcpp.h>
#include "QueryHandler.h"
#include "execution/io/trends/Trends.h"
#include "meta/Table.h"
#include "meta/TestTable.h"
#include "nebula.grpc.pb.h"
#include "type/Serde.h"

/**
 * A cursor template that help iterating a container.
 * (should we use std::iterator instead?)
 */
namespace nebula {
namespace service {

// build for specific product such as trends
class V1ServiceImpl final : public V1::Service {
  grpc::Status State(grpc::ServerContext*, const TableStateRequest*, TableStateResponse*);
  grpc::Status Query(grpc::ServerContext*, const QueryRequest*, QueryResponse*);

  // TODO(cao) - designed to serve trends table specifically, remove after having real meta service.
  nebula::execution::io::trends::TrendsTable table_;

  // query handler to handle all the queries
  QueryHandler handler_;

private:
  grpc::Status replyError(ErrorCode, QueryResponse*, size_t) const;

public:
  void loadTrends() {
    table_.loadTrends();
  }
};

} // namespace service
} // namespace nebula