/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */


#include "hermes/Platform/Intl/PlatformIntl.h"
#include <hermes/hermes.h>
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

namespace {
using namespace hermes;
using namespace hermes::platform_intl;

// the simplest of testcases
TEST(DateTimeFormat, SimpleDates) {
  std::vector<std::u16string> input = std::vector<std::u16string>{u"en-us"};
  std::shared_ptr<hermes::vm::Runtime> runtime = hermes::vm::Runtime::create(
      hermes::vm::RuntimeConfig::Builder().withIntl(true).build());
  auto actual = platform_intl::getCanonicalLocales(*runtime.get(), input);
  auto value = actual.getValue().front();
  auto status = actual.getStatus();
  EXPECT_TRUE(status == vm::ExecutionStatus::RETURNED);
  EXPECT_TRUE(u"en-US" == value);

  input = std::vector<std::u16string>{u"FR"};
  actual = getCanonicalLocales(*runtime.get(), input);
  value = actual.getValue().front();
  status = actual.getStatus();
  EXPECT_TRUE(status == vm::ExecutionStatus::RETURNED);
  EXPECT_TRUE(u"fr" == value);
}

} // end anonymous namespace