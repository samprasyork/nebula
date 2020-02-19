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

#include <fmt/format.h>

#include "surface/eval/UDF.h"

/**
 * Define expressions used in the nebula DSL.
 */
namespace nebula {
namespace api {
namespace udf {

// UDAF - min
template <nebula::type::Kind NK, typename BaseType = nebula::surface::eval::UDAF<NK>>
class Min : public BaseType {
public:
  using StoreType = typename BaseType::StoreType;
  Min(const std::string& name, std::unique_ptr<nebula::surface::eval::ValueEval> expr)
    : BaseType(name,
               std::move(expr),
               [](StoreType ov, StoreType nv) {
                 return std::min<StoreType>(ov, nv);
               }) {}
  virtual ~Min() = default;
};

} // namespace udf
} // namespace api
} // namespace nebula