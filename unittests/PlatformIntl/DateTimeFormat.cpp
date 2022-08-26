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

// simplest of testcases, tests one locale without any options, Sunday 2nd May 2021 17:00:00 UTC
TEST(DateTimeFormat, DatesWithoutOptions) {
  std::vector<std::u16string> AmericanEnglish = std::vector<std::u16string>{u"en-us"};
  std::vector<std::u16string> KoreanKorea = std::vector<std::u16string>{u"ko-KR"};
  std::vector<std::u16string> french = std::vector<std::u16string>{u"fr"};
  std::shared_ptr<hermes::vm::Runtime> runtime = hermes::vm::Runtime::create(
      hermes::vm::RuntimeConfig::Builder().withIntl(true).build());
  Options testOptions = {};

  auto dtf = platform_intl::DateTimeFormat();
  dtf.initialize(*runtime.get(), AmericanEnglish, testOptions);
  auto result = dtf.format(1620000000000.00);
  EXPECT_EQ(result, u"5/2/2021");

  auto dtf2 = platform_intl::DateTimeFormat();
  dtf2.initialize(*runtime.get(), KoreanKorea, testOptions);
  auto result2 = dtf2.format(1620000000000.00);
  EXPECT_EQ(result2, u"2021. 5. 2.");

  auto dtf3 = platform_intl::DateTimeFormat();
  dtf3.initialize(*runtime.get(), french, testOptions);
  std::u16string result3 = dtf3.format(1620000000000.00);
  EXPECT_EQ(result3, u"02/05/2021");
}

// tests dateStyle and timeStyle options (full, long, medium, short), Sunday 2nd May 2021 17:00:00 UTC
TEST(DateTimeFormat, DatesWithTimeDateStyles) {
  std::vector<std::u16string> AmericanEnglish = std::vector<std::u16string>{u"en-us"};
  std::vector<std::u16string> SpanishPeru = std::vector<std::u16string>{u"es-PE"};
  std::vector<std::u16string> french = std::vector<std::u16string>{u"fr"};
  std::shared_ptr<hermes::vm::Runtime> runtime = hermes::vm::Runtime::create(
      hermes::vm::RuntimeConfig::Builder().withIntl(true).build());

  // Tests dateStyle = full and timeStye = full
  Options testOptions = {{u"dateStyle", Option(std::u16string(u"full"))},{u"timeStyle", Option(std::u16string(u"full"))}};
  auto dtf = platform_intl::DateTimeFormat();
  dtf.initialize(*runtime.get(), AmericanEnglish, testOptions);
  auto result = dtf.format(1620000000000.00);
  EXPECT_EQ(result, u"Sunday, May 2, 2021 at 5:00:00 PM Pacific Daylight Time");

  auto dtf2 = platform_intl::DateTimeFormat();
  dtf2.initialize(*runtime.get(), french, testOptions);
  auto result2 = dtf2.format(1620000000000.00);
  EXPECT_EQ(result2, u"dimanche 2 mai 2021 \u00E0 17:00:00 heure d\u2019\u00E9t\u00E9 du Pacifique"); //L"dimanche 2 mai 2021 à 17:00:00 heure d’été du Pacifique"

  auto dtf3 = platform_intl::DateTimeFormat();
  dtf3.initialize(*runtime.get(), SpanishPeru, testOptions);
  auto result3 = dtf3.format(1620000000000.00);
  EXPECT_EQ(result3, u"domingo, 2 de mayo de 2021, 17:00:00 hora de verano del Pac\u00EDfico"); //L"domingo, 2 de mayo de 2021, 17:00:00 hora de verano del Pacífico"

  // Tests dateStyle = short and timeStyle = short
  Options testOptions2 = {{u"dateStyle", Option(std::u16string(u"short"))},{u"timeStyle", Option(std::u16string(u"short"))}};
  auto dtf4 = platform_intl::DateTimeFormat();
  dtf4.initialize(*runtime.get(), AmericanEnglish, testOptions2);
  auto result4 = dtf4.format(1620000000000.00);
  EXPECT_EQ(result4, u"5/2/21, 5:00 PM");

  auto dtf5 = platform_intl::DateTimeFormat();
  dtf5.initialize(*runtime.get(), french, testOptions2);
  auto result5 = dtf5.format(1620000000000.00);
  EXPECT_EQ(result5, u"02/05/2021 17:00");

  auto dtf6 = platform_intl::DateTimeFormat();
  dtf6.initialize(*runtime.get(), SpanishPeru, testOptions2);
  auto result6 = dtf6.format(1620000000000.00);
  EXPECT_EQ(result6, u"2/05/21 17:00");

  // Tests dateStyle = long and timeStyle = medium
  Options testOptions3 = {{u"dateStyle", Option(std::u16string(u"long"))},{u"timeStyle", Option(std::u16string(u"medium"))}};
  auto dtf7 = platform_intl::DateTimeFormat();
  dtf7.initialize(*runtime.get(), AmericanEnglish, testOptions3);
  auto result7 = dtf7.format(1620000000000.00);
  EXPECT_EQ(result7, u"May 2, 2021 at 5:00:00 PM");

  auto dtf8 = platform_intl::DateTimeFormat();
  dtf8.initialize(*runtime.get(), french, testOptions3);
  auto result8 = dtf8.format(1620000000000.00);
  EXPECT_EQ(result8, u"2 mai 2021 \u00E0 17:00:00"); //L"2 mai 2021 à 17:00:00"

  auto dtf9 = platform_intl::DateTimeFormat();
  dtf9.initialize(*runtime.get(), SpanishPeru, testOptions3);
  auto result9 = dtf9.format(1620000000000.00);
  EXPECT_EQ(result9, u"2 de mayo de 2021, 17:00:00");
}

// Tests Date with Month (2-digit, numeric, narrow, short, long), Day (2-digit, numeric), and Year (2-digit, numeric) options
TEST(DateTimeFormat, DatesWithMonthDayYearOptions) {
  std::vector<std::u16string> DutchBelgium = std::vector<std::u16string>{u"nl-BE"};
  std::shared_ptr<hermes::vm::Runtime> runtime = hermes::vm::Runtime::create(
      hermes::vm::RuntimeConfig::Builder().withIntl(true).build());

  Options testOptions = {{u"day", Option(std::u16string(u"numeric"))},{u"month", Option(std::u16string(u"long"))},{u"year", Option(std::u16string(u"numeric"))}};
  auto dtf = platform_intl::DateTimeFormat();
  dtf.initialize(*runtime.get(), DutchBelgium, testOptions);
  auto result = dtf.format(1620000000000.00);
  EXPECT_EQ(result, u"2 mei 2021");

}

} // end anonymous namespace