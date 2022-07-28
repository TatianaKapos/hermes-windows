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
//#include "hermes/Platform/Unicode/icu.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <icu.h>
#include <codecvt>

// using namespace ::facebook;
using namespace ::hermes;

namespace hermes {
namespace platform_intl {

// Helper Functions - should move to another file
std::u16string NormalizeLangugeTag(const std::u16string locale) {
    if (locale.empty()) {
        //std::cout << "invalid language tag";
    }
   
    // conversion helper: UTF-8 to/from UTF-16
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
    std::string locale8 = conversion.to_bytes(locale);

    // ICU doesn't have a full-fledged canonicalization implementation that correctly replaces all preferred values
    // and grandfathered tags, as required by #sec-canonicalizelanguagetag.
    // However, passing the locale through uloc_forLanguageTag -> uloc_toLanguageTag gets us most of the way there
    // by replacing some(?) values, correctly capitalizing the tag, and re-ordering extensions 
    UErrorCode status = U_ZERO_ERROR;
    int parsedLength = 0;
    char localeID[ULOC_FULLNAME_CAPACITY] = { 0 };
    char canonicalized[ULOC_FULLNAME_CAPACITY] = { 0 };
    int forLangTagResultLength = uloc_forLanguageTag(locale8.c_str(), localeID, ULOC_FULLNAME_CAPACITY, &parsedLength, &status);

    int toLangTagResultLength = uloc_toLanguageTag(localeID, canonicalized, ULOC_FULLNAME_CAPACITY, true, &status);

    return conversion.from_bytes(canonicalized);
}

std::vector<std::u16string> CanonicalizeLocaleList(const std::vector<std::u16string>& locales) {

    // 1. If locales is undefined, then a. Return a new empty list
    if (locales.empty()) {
        //std::cout << "list is empty";
        return std::vector<std::u16string>{};
    }
    // 2. Let seen be a new empty List
    std::vector<std::u16string> seen = std::vector<std::u16string>{};
    
    // 3. If Type(locales) is String or Type(locales) is Object and locales has an
    // [[InitializedLocale]] internal slot, then
    // 4. Else
    //  > TODO: Windows and Apple don't yet support Locale object -
    //  > https://tc39.es/ecma402/#locale-objects As of now, 'locales' can only be a
    //  > string list/array. Validation occurs in NormalizeLangugeTag for windows, so this
    //  > function just takes a vector of strings.
    // 5-7. Let len be ? ToLength(? Get(O, "length")). Let k be 0. Repeat, while k < len
    for (int k = 0; k < locales.size(); k++) {
        // TODO: tag validation, note that Apples implementation does not do tag validation yet nor does Chakra with ICU
        // 7.c.iii.1 Let tag be kValue[[locale]]
        std::u16string tag = locales[k];
        // 7.c.vi Let canonicalizedTag be CanonicalizeUnicodeLocaleID(tag)
        std::u16string canonicalizedTag = NormalizeLangugeTag(tag);
        // 7.c.vii. If canonicalizedTag is not an element of seen, append canonicalizedTag as the last element of seen.
        if (std::find(seen.begin(), seen.end(), canonicalizedTag) == seen.end()) {
            seen.push_back(std::move(canonicalizedTag));
        }
    }
    return seen;
}

// returns an array with canonical locale names
// locales - a list of strings for which to get cononical locacle name (ex: ['EN-US', 'Fr'])
// return - array containing the canonical locale names (ex: ["en-US", "fr"])
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  return CanonicalizeLocaleList(locales);
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
  UCollator *coll;
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

    // conversion helper: UTF-8 to/from UTF-16
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
    
    impl_->locale = u"en-US";
    std::string locale8 = conversion.to_bytes(impl_->locale);
    UErrorCode err{ U_ZERO_ERROR };
    impl_->coll = ucol_open(locale8.c_str(), &err);

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

    auto result = ucol_strcoll(
        impl_->coll,
        (const UChar*)x.data(),
        x.size(),
        (const UChar*)y.data(),
        y.size());

    switch (result) {
        case UCOL_LESS:
            return -1;
        case UCOL_EQUAL:
            return 0;
        case UCOL_GREATER:
            return 1;
    }
    return 10;
}

/// https://402.ecma-international.org/8.0/#sec-getoption
/// Split into getOptionString and getOptionBool to help readability
std::u16string getOptionString(
    vm::Runtime *runtime,
    const Options& options,
    const std::u16string& property,
    std::u16string fallback) {
    auto valueIt = options.find(property);
    if (valueIt == options.end())
        return std::u16string(fallback);
    std::u16string value = valueIt->second.getString();
    return value;
}

bool getOptionBool(
    vm::Runtime *runtime,
    const Options& options,
    const std::u16string& property,
    bool fallback) {
    //  1. Assert: Type(options) is Object.
    //  2. Let value be ? Get(options, property).
    auto value = options.find(property);
    //  3. If value is undefined, return fallback.
    if (value == options.end()) {
        return fallback;
    }
    //  8. Return value.
    return value->second.getBool();
}

// Implementation of
// https://402.ecma-international.org/8.0/#sec-todatetimeoptions
Options toDateTimeOptions(
    vm::Runtime *runtime,
    Options options,
    std::u16string required,
    std::u16string defaults) {
  // 1. If options is undefined, let options be null; otherwise let options be ?
    // ToObject(options).
    // 2. Let options be OrdinaryObjectCreate(options).
    // 3. Let needDefaults be true.
    bool needDefaults = true;
    // 4. If required is "date" or "any", then
    if (required == u"date" || required == u"any") {
        // a. For each property name prop of « "weekday", "year", "month", "day" »,
        // do
        // TODO(T116352920): Make this a std::u16string props[] once we have
        // constexpr std::u16string.
        static const std::vector<std::u16string> props = {u"weekday", u"year", u"month", u"day"};
        for (const auto& prop : props) {
            // i. Let value be ? Get(options, prop).
            if (options.find(std::u16string(prop)) != options.end()) {
                // ii. If value is not undefined, let needDefaults be false.
                needDefaults = false;
            }
        }
    }
    // 5. If required is "time" or "any", then
    if (required == u"time" || required == u"any") {
        // a. For each property name prop of « "dayPeriod", "hour", "minute",
        // "second", "fractionalSecondDigits" », do
        static const std::vector<std::u16string> props = {u"dayPeriod", u"hour", u"minute", u"second", u"fractionalSecondDigits" };
        for (const auto& prop : props) {
            // i. Let value be ? Get(options, prop).
            if (options.find(std::u16string(prop)) != options.end()) {
                // ii. If value is not undefined, let needDefaults be false.
                needDefaults = false;
            }
        }
    }
    // 6. Let dateStyle be ? Get(options, "dateStyle").
    auto dateStyle = options.find(u"dateStyle");
    // 7. Let timeStyle be ? Get(options, "timeStyle").
    auto timeStyle = options.find(u"timeStyle");
    // 8. If dateStyle is not undefined or timeStyle is not undefined, let
    // needDefaults be false.
    if (dateStyle != options.end() || timeStyle != options.end()) {
        needDefaults = false;
    }
    // 9. If required is "date" and timeStyle is not undefined, then
    if (required == u"date" && timeStyle != options.end()) {
        // a. Throw a TypeError exception.
    }
    // 10. If required is "time" and dateStyle is not undefined, then
    if (required == u"time" && dateStyle != options.end()) {
        // a. Throw a TypeError exception.
    }
    // 11. If needDefaults is true and defaults is either "date" or "all", then
    if (needDefaults && (defaults == u"date" || defaults == u"all")) {
        // a. For each property name prop of « "year", "month", "day" », do
        static const std::vector<std::u16string> props = { u"year", u"month", u"day" };
        for (const auto& prop : props) {
            // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
            options.emplace(prop, Option(std::u16string(u"numeric")));
        }
    }
    // 12. If needDefaults is true and defaults is either "time" or "all", then
    if (needDefaults && (defaults == u"time" || defaults == u"all")) {
        // a. For each property name prop of « "hour", "minute", "second" », do
        static const std::vector<std::u16string> props = {u"hour", u"minute", u"second" };
        for (const auto& prop : props) {
            // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
            options.emplace(prop, Option(std::u16string(u"numeric")));
        }
    }
    // 13. return options
    return options;
}

// EX: Intl.DateTimeFormat('en-US').format(date) -> "12/20/2020"
// EX: new Intl.DateTimeFormat(['ban', 'id']).format(date) -> "20/12/2020"
struct DateTimeFormat::Impl {
  std::u16string locale;
  UDateFormat *dtf;
  UCalendar *calendar;
  std::u16string timeZone; 
  std::u16string weekday;
  std::u16string era;
  std::u16string year;
  std::u16string month;
  std::u16string day;
  std::u16string dayPeriod;
  std::u16string hour;
  std::u16string minute;
  std::u16string second;
  std::u16string timeZoneName;
  std::u16string dateStyle;
  std::u16string timeStyle;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}


// get the supported locales of dateTimeFormat
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}



// initalize dtf
vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  
   auto requestedLocalesRes = CanonicalizeLocaleList(locales);
   impl_->locale = locales.front();
   // conversion helper: UTF-8 to/from UTF-16
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
    std::string locale8 = conversion.to_bytes(impl_->locale);

    // 2. Let options be ? ToDateTimeOptions(options, "any", "date").
    Options options = toDateTimeOptions(runtime, inputOptions, u"any", u"date");
    std::unordered_map<std::u16string, std::u16string> opt;
    auto matcherRes = getOptionString(runtime, options,u"localeMatcher",u"best fit");
    opt.emplace(u"localeMatcher", matcherRes);
    auto calendarRes = getOptionString(runtime, options, u"calendar", {});
    opt.emplace(u"ca", calendarRes);
    opt.emplace(u"nu", u"");
    auto collationRes = getOptionString(runtime, options, u"collation", {});
    opt.emplace(u"co", collationRes);

    static const std::vector<std::u16string> dateStyles = {u"full", u"long", u"medium", u"short" };
    auto dateStyleRes = getOptionString(runtime, options, u"dateStyle", u"full");
    // 33. Set dateTimeFormat.[[DateStyle]] to dateStyle.
    impl_->dateStyle = dateStyleRes;

    static const std::vector<std::u16string> timeStyles = { u"full", u"long", u"medium", u"short" };
    auto timeStyleRes = getOptionString(runtime, options, u"timeStyle", u"full");
    // 33. Set dateTimeFormat.[[DateStyle]] to dateStyle.
    impl_->timeStyle = timeStyleRes;

    UDateFormatStyle dateStyle = UDAT_DEFAULT;
    UDateFormatStyle timeStyle = UDAT_DEFAULT;
    if (!impl_->dateStyle.empty()) {
        if (impl_->dateStyle.compare(u"full") == 0) {
            dateStyle = UDAT_FULL;
        }
        else if (impl_->dateStyle.compare(u"short") == 0) {
            dateStyle = UDAT_SHORT;
        }
        else if (impl_->dateStyle.compare(u"medium") == 0) {
            dateStyle = UDAT_MEDIUM;
        }
        else if (impl_->dateStyle.compare(u"long") == 0) {
            dateStyle = UDAT_LONG;
        }
    }

    if (!impl_->timeStyle.empty()) {
        if (impl_->timeStyle.compare(u"full") == 0) {
            timeStyle = UDAT_FULL;
        }
        else if (impl_->timeStyle.compare(u"short") == 0) {
            timeStyle = UDAT_SHORT;
        }
        else if (impl_->dateStyle.compare(u"medium") == 0) {
            timeStyle = UDAT_MEDIUM;
        }
        else if (impl_->dateStyle.compare(u"long") == 0) {
            timeStyle = UDAT_LONG;
        }
    }

    UErrorCode status = U_ZERO_ERROR;
    impl_->dtf = udat_open(timeStyle, dateStyle, locale8.c_str(), 0, -1, NULL, -1, &status);
    // DateTimeFormat is expected to use the "proleptic Gregorian calendar", which means that the Julian calendar should never be used.
    // To accomplish this, we can set the switchover date between julian/gregorian
    // to the ECMAScript beginning of time, which is -8.64e15 according to ecma262 #sec-time-values-and-time-range
    impl_->calendar = const_cast<UCalendar*>(udat_getCalendar(impl_->dtf));
    const char* calType = ucal_getType(impl_->calendar, &status);
    if (strcmp(calType, "gregorian") == 0)
    {
        double beginningOfTime = -8.64e15;
        ucal_setGregorianChange(impl_->calendar, beginningOfTime, &status);
        double actualGregorianChange = ucal_getGregorianChange(impl_->calendar, &status);
    }

  return vm::ExecutionStatus::RETURNED;
}


Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  options.emplace(u"timeZone", Option(impl_->timeZone));
  options.emplace(u"calendar", Option(impl_->calendar));
  options.emplace(u"weekday", impl_->weekday);
  options.emplace(u"era", impl_->era);
  options.emplace(u"year", impl_->year);
  options.emplace(u"month", impl_->month);
  options.emplace(u"day", impl_->day);
  options.emplace(u"hour", impl_->hour);
  options.emplace(u"minute", impl_->minute);
  options.emplace(u"second", impl_->second);
  options.emplace(u"timeZoneName", impl_->timeZoneName);
  options.emplace(u"dateStyle", impl_->dateStyle);
  options.emplace(u"timeStyle", impl_->timeStyle);
  return options;
}

//formats date according to the locale/options
std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  // conversion helper: UTF-8 to/from UTF-16
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;

    auto timeInSeconds = jsTimeValue / 1000;
    UDate *date = new UDate(timeInSeconds);
    UErrorCode status = U_ZERO_ERROR;
    UChar *myString;
    int myStrlen = 0;

    myStrlen = udat_format(impl_->dtf, *date, NULL, myStrlen, NULL, &status);
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR;
        myString = (UChar*)malloc(sizeof(UChar) * (myStrlen + 1));
        udat_format(impl_->dtf, *date, myString, myStrlen + 1, NULL, &status);
            
        char *dts = (char*)malloc(sizeof(UChar) * (myStrlen + 1));    
        return myString;
      }
        
    return u"failed";
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
