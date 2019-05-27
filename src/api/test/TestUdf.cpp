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

#include <glog/logging.h>
#include <gtest/gtest.h>
#include "api/dsl/Expressions.h"
#include "api/udf/In.h"
#include "api/udf/Like.h"
#include "api/udf/MyUdf.h"
#include "api/udf/Prefix.h"
#include "execution/eval/ValueEval.h"
#include "surface/DataSurface.h"
#include "surface/MockSurface.h"

namespace nebula {
namespace api {
namespace test {

TEST(UDFTest, TestNot) {
  nebula::surface::MockRowData row;
  nebula::execution::eval::EvalContext ctx;
  ctx.reset(row);

  auto f = std::make_shared<nebula::api::dsl::ConstExpression<bool>>(false);
  nebula::api::udf::Not n("n", f->asEval());
  bool valid = true;
  EXPECT_EQ(n.eval(ctx, valid), true);

  auto t = std::make_shared<nebula::api::dsl::ConstExpression<bool>>(true);
  nebula::api::udf::Not y("n", t->asEval());
  valid = true;
  EXPECT_EQ(y.eval(ctx, valid), false);
}

TEST(UDFTest, TestLike) {
  std::vector<std::tuple<std::string, std::string, bool>> data{
    { "abcdefg", "abc%", true },
    { "Shawn says hi", "%says%", true },
    { "long time no see", "%see", true },
    { "nebula is cool", "%is", false },
    { "nebula is awesome", "nebula%", true },
    { "hi there ", "%i th%", true },
    { "hi there ", "i th%", false },
    { "hi there", "%there", true }
  };
  nebula::surface::MockRowData row;
  nebula::execution::eval::EvalContext ctx;
  ctx.reset(row);

  for (const auto& item : data) {
    const auto& s = std::get<0>(item);
    const auto& p = std::get<1>(item);
    auto r = std::get<2>(item);
    LOG(INFO) << "Match " << s << " with " << p << " is " << r;
    auto c = std::make_shared<nebula::api::dsl::ConstExpression<std::string_view>>(s);
    nebula::api::udf::Like l("l", c->asEval(), p);
    bool valid = true;
    EXPECT_EQ(l.eval(ctx, valid), r);
  }
}

TEST(UDFTest, TestPrefix) {
  std::vector<std::tuple<std::string, std::string, bool>> data{
    { "abcdefg", "abc", true },
    { "Shawn says hi", "says", false },
    { "long time no see", "long time", true },
    { "nebula is cool", "is", false },
    { "nebula is awesome", "nebula", true },
    { "hi there ", "%i th", false },
    { "hi there ", "i th", false },
    { "hi there", "hi there", true }
  };
  nebula::surface::MockRowData row;
  nebula::execution::eval::EvalContext ctx;
  ctx.reset(row);

  for (const auto& item : data) {
    const auto& s = std::get<0>(item);
    const auto& p = std::get<1>(item);
    auto r = std::get<2>(item);
    LOG(INFO) << "Match " << s << " with " << p << " is " << r;
    auto c = std::make_shared<nebula::api::dsl::ConstExpression<std::string_view>>(s);
    nebula::api::udf::Prefix prefix("p", c->asEval(), p);
    bool valid = true;
    EXPECT_EQ(prefix.eval(ctx, valid), r);
  }
}

TEST(UDFTest, TestPrefixContext) {
  std::vector<std::tuple<std::string, std::string, bool>> data{
    { "abcdefg", "abc", true },
  };

  nebula::surface::MockRowData row;
  nebula::execution::eval::EvalContext context;
  context.reset(row);

  auto c = std::make_shared<nebula::api::dsl::ConstExpression<std::string_view>>("abcdfeg");
  nebula::api::udf::Prefix prefix("p", c->asEval(), "abc");
  for (auto i = 0; i < 1000; ++i) {
    bool valid = true;
    // auto result = prefix.eval(row, valid);
    auto result = context.eval<bool>(prefix, valid);
    EXPECT_EQ(valid, true);
    EXPECT_EQ(result, true);
  }
}

TEST(UDFTest, TestIn) {
  {
    // set, target, in or not in, result
    using S = std::vector<std::string>;
    std::vector<std::tuple<S, std::string, bool, bool>> data{
      { { "abcdefg", "abc" }, "abc", true, true },
      { { "x", "y", "z" }, "x", true, true },
      { { "x", "y", "z" }, "a", true, false },
      { { "x", "y", "z" }, "x", false, false },
      { { "x", "y", "z" }, "a", false, true },
      { { "x", "y", "z" }, "z", false, false },
    };

    nebula::surface::MockRowData row;
    nebula::execution::eval::EvalContext ctx;
    ctx.reset(row);

    for (const auto& item : data) {
      const auto& s = std::get<0>(item);
      const auto& t = std::get<1>(item);
      const auto& f = std::get<2>(item);
      const auto& r = std::get<3>(item);

      auto c = std::make_shared<nebula::api::dsl::ConstExpression<std::string_view>>(t);
      if (f) {
        nebula::api::udf::In<nebula::type::Kind::VARCHAR> in("i", c->asEval(), s);
        bool valid = true;
        EXPECT_EQ(in.eval(ctx, valid), r);
      } else {
        nebula::api::udf::In<nebula::type::Kind::VARCHAR> in("i", c->asEval(), s, f);
        bool valid = true;
        EXPECT_EQ(in.eval(ctx, valid), r);
      }
    }
  }

  {
    // set, target, in or not in, result
    using S = std::vector<int32_t>;
    std::vector<std::tuple<S, int32_t, bool, bool>> data{
      { { 0, 1, 2 }, 1, true, true },
      { { 66, 73, 54 }, 54, true, true },
      { { 23, 45, 67, 89 }, 11, true, false },
      { { 11, 22, 33 }, 22, false, false },
      { { 11, 22, 33 }, 44, false, true },
      { { 23, 34, 45, 56 }, 45, false, false },
    };

    nebula::surface::MockRowData row;
    nebula::execution::eval::EvalContext ctx;
    ctx.reset(row);

    for (const auto& item : data) {
      const auto& s = std::get<0>(item);
      const auto& t = std::get<1>(item);
      const auto& f = std::get<2>(item);
      const auto& r = std::get<3>(item);

      auto c = std::make_shared<nebula::api::dsl::ConstExpression<int32_t>>(t);
      if (f) {
        nebula::api::udf::In<nebula::type::Kind::INTEGER> in("i", c->asEval(), s);
        bool valid = true;
        EXPECT_EQ(in.eval(ctx, valid), r);
      } else {
        nebula::api::udf::In<nebula::type::Kind::INTEGER> in("i", c->asEval(), s, f);
        bool valid = true;
        EXPECT_EQ(in.eval(ctx, valid), r);
      }
    }
  }
}

} // namespace test
} // namespace api
} // namespace nebula