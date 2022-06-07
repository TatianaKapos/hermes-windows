/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/* Top of mind questions
* 
* Are these all the methods we *need* to say we support Intl? Seems to be missing a few (like ListFormat, Locale, PluralRules, ect.).
* Matching current Android implentation, first generation, Number, DateTime, Collotor
* 
* Do we have an internal API or methods that deal with locales? Not sure how to know all the different locales/internalizations?
* ICU-4C - open source libary, add .dll
* 
* Do you know anyone who has working on Intl on V8 or Chakra? Wondering if we should reuse some of their code?
* Referenced their code, Chakra least complete 
* 
* Are there already tests for windows intl or are those specifically for android?
* 
*/

#include "hermes/Platform/Intl/PlatformIntl.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <icu.h>

// using namespace ::facebook;
using namespace ::hermes;

namespace hermes {
namespace platform_intl {


// returns an array with canonical locale names
// locales - a list of strings for which to get cononical locacle name (ex: ['EN-US', 'Fr'])
// return - array containing the canonical locale names (ex: ["en-US", "fr"])
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  return std::vector<std::u16string>{u"fr-FR", u"es-ES"};
}


// returns string value to lower case
// locale - indicates locale to be used to convert to lowercase (ex: ['EN-US'])
vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"lowered");
}

// returns string value to uppercase
// locale - indicates locale to be used to convert to uppercase (ex: ['EN-US'])
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"uppered");
}

// object enables language-sensitive string comparison, initalized with specified locale string
struct Collator::Impl {
  std::u16string locale;
};

// what are theses lines of code doing? ------------------------------------------------------------------------------------
Collator::Collator() : impl_(std::make_unique<Impl>()) {}
Collator::~Collator() {}


// returns an array containing provided locales that are supported w/o having to fallback to runtime's default locale 
// what are thoses we support? ---------------------------------------------------------------------------------------------
vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}


// initalizes Collator object
// locales - list of locales
// options - list of options, see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator/Collator
vm::ExecutionStatus Collator::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

// return the options given when the collator object was initalize
Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return x.compare(y);
}

struct DateTimeFormat::Impl {
  std::u16string locale;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  auto s = std::to_string(jsTimeValue);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(jsTimeValue);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

struct NumberFormat::Impl {
  std::u16string locale;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string NumberFormat::format(double number) noexcept {
  auto s = std::to_string(number);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
NumberFormat::formatToParts(double number) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(number);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

} // namespace platform_intl
} // namespace hermes
