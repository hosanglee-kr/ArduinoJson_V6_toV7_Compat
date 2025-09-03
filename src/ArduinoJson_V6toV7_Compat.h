
// ArduinoJson_V6toV7_Compat.h
#pragma once
// ArduinoJson v6 -> v7 호환 래퍼
// 참고: https://arduinojson.org/v7/how-to/upgrade-from-v6/
//
// 이 래퍼는 컴파일 호환을 돕기 위한 것이고 실제 동작은 v7 API에 위임돼요!
// capacity()/memoryUsage()/shallowCopy() 등은 의미가 없고, overflowed()만 확인하세요!

#include <ArduinoJson.h>

#if __has_include(<Arduino.h>)
  #include <Arduino.h>
#elif __has_include(<WString.h>)
  #include <WString.h>
#endif

// v6 매크로 더미
#ifndef JSON_OBJECT_SIZE
#define JSON_OBJECT_SIZE(n) 0
#endif
#ifndef JSON_ARRAY_SIZE
#define JSON_ARRAY_SIZE(n) 0
#endif
#ifndef JSON_STRING_SIZE
#define JSON_STRING_SIZE(n) ((size_t)((n) + 1))
#endif

namespace AJ = ::ArduinoJson;

namespace AJv6 {

// ---------- DocCompat ----------
struct DocCompat : public AJ::JsonDocument {
  explicit DocCompat(size_t /*capacity*/ = 0) : AJ::JsonDocument() {}
  explicit DocCompat(AJ::Allocator* allocPtr) : AJ::JsonDocument(allocPtr) {}

  size_t capacity() const { return 0; }            // v7: 의미 없음
  size_t memoryUsage() const { return 0; }         // v7: 의미 없음
  void   garbageCollect() {}                       // v7: 자동 재사용
  void   shrinkToFit() { AJ::JsonDocument::shrinkToFit(); }
  bool   overflowed() const { return AJ::JsonDocument::overflowed(); }

  template <typename T>
  void shallowCopy(const T&) { /* v7: 얕은 복사 미지원 */ }
};

// ---------- JsonObjectCompat ----------
struct JsonObjectCompat {
  AJ::JsonObject impl;
  JsonObjectCompat() = default;
  JsonObjectCompat(const AJ::JsonObject& o) : impl(o) {}
  operator AJ::JsonObject&() { return impl; }
  operator const AJ::JsonObject&() const { return impl; }
  AJ::JsonObject* operator->() { return &impl; }
  const AJ::JsonObject* operator->() const { return &impl; }
  explicit operator bool() const { return impl.operator bool(); }

  // v6 → v7 변환
  AJ::JsonArray createNestedArray(const char* key) { return impl[key].to<AJ::JsonArray>(); }
#if __has_include(<Arduino.h>) || __has_include(<WString.h>)
  AJ::JsonArray createNestedArray(const String& key) { return impl[key].to<AJ::JsonArray>(); }
#endif
#ifdef PROGMEM
  AJ::JsonArray createNestedArray(const __FlashStringHelper* key) { return impl[key].to<AJ::JsonArray>(); }
#endif

  AJ::JsonObject createNestedObject(const char* key) { return impl[key].to<AJ::JsonObject>(); }
#if __has_include(<Arduino.h>) || __has_include(<WString.h>)
  AJ::JsonObject createNestedObject(const String& key) { return impl[key].to<AJ::JsonObject>(); }
#endif
#ifdef PROGMEM
  AJ::JsonObject createNestedObject(const __FlashStringHelper* key) { return impl[key].to<AJ::JsonObject>(); }
#endif

  bool containsKey(const char* key) const { return !impl[key].isNull(); }
#if __has_include(<Arduino.h>) || __has_include(<WString.h>)
  bool containsKey(const String& key) const { return !impl[key].isNull(); }
#endif
#ifdef PROGMEM
  bool containsKey(const __FlashStringHelper* key) const { return !impl[key].isNull(); }
#endif
};

// ---------- JsonArrayCompat ----------
struct JsonArrayCompat {
  AJ::JsonArray impl;
  JsonArrayCompat() = default;
  JsonArrayCompat(const AJ::JsonArray& a) : impl(a) {}
  operator AJ::JsonArray&() { return impl; }
  operator const AJ::JsonArray&() const { return impl; }
  AJ::JsonArray* operator->() { return &impl; }
  const AJ::JsonArray* operator->() const { return &impl; }
  explicit operator bool() const { return impl.operator bool(); }

  AJ::JsonArray  createNestedArray()  { return impl.add<AJ::JsonArray>(); }
  AJ::JsonObject createNestedObject() { return impl.add<AJ::JsonObject>(); }
};

// ---------- v6 타입 이름 보존 ----------
struct DynamicJsonDocument : public DocCompat {
  explicit DynamicJsonDocument(size_t capacity = 0) : DocCompat(capacity) {}
  DynamicJsonDocument(const DynamicJsonDocument& o) : DocCompat() { *this = o; }
  DynamicJsonDocument(DynamicJsonDocument&& o) noexcept : DocCompat() { *this = std::move(o); }
  DynamicJsonDocument& operator=(const DynamicJsonDocument& o) { AJ::JsonDocument::operator=(o); return *this; }
  DynamicJsonDocument& operator=(DynamicJsonDocument&& o) noexcept { AJ::JsonDocument::operator=(std::move(o)); return *this; }
};

template <size_t N>
struct StaticJsonDocument : public DocCompat {
  StaticJsonDocument() : DocCompat(N) {}
  static constexpr size_t kCapacity = N;
};

// ---------- BasicJsonDocument<Allocator> ----------
template <typename TAllocator = AJ::Allocator>
struct BasicJsonDocument : public DocCompat {
  using allocator_type = TAllocator;

  explicit BasicJsonDocument(size_t /*capacity*/ = 0,
                             const allocator_type& alloc = allocator_type())
    : DocCompat(dupAndPass_(alloc)) {
    _alloc_ptr_   = reinterpret_cast<allocator_type*>(_last_passed_);
    _owned_alloc_ = _alloc_ptr_;
  }

  explicit BasicJsonDocument(allocator_type* externalAlloc)
    : DocCompat(static_cast<AJ::Allocator*>(externalAlloc)) {
    _alloc_ptr_   = externalAlloc;
    _owned_alloc_ = nullptr;
  }

  ~BasicJsonDocument() { if (_owned_alloc_) delete _owned_alloc_; }

  const allocator_type& getAllocator() const { return *_alloc_ptr_; }
  allocator_type*       getAllocatorPtr() const { return _alloc_ptr_; }

private:
  allocator_type* _alloc_ptr_   = nullptr;
  allocator_type* _owned_alloc_ = nullptr;

  static AJ::Allocator* _last_passed_;
  static AJ::Allocator* dupAndPass_(const allocator_type& alloc) {
    allocator_type* p = new allocator_type(alloc);
    _last_passed_ = p;
    return static_cast<AJ::Allocator*>(p);
  }
};

template <typename TAllocator>
AJ::Allocator* BasicJsonDocument<TAllocator>::_last_passed_ = nullptr;

} // namespace AJv6

// ---- 전역 별칭 내보내기: 필요 없으면 매크로로 끄기 ----
#ifndef ARDUINOJSON_V6COMPAT_NO_GLOBAL_USING
template <size_t N>
using StaticJsonDocument = AJv6::StaticJsonDocument<N>;
using DynamicJsonDocument = AJv6::DynamicJsonDocument;
template <typename TAllocator = ::ArduinoJson::Allocator>
using BasicJsonDocument = AJv6::BasicJsonDocument<TAllocator>;
using JsonArray         = AJv6::JsonArrayCompat;
using JsonArrayConst    = ::ArduinoJson::JsonArrayConst;
using JsonObject        = AJv6::JsonObjectCompat;
using JsonObjectConst   = ::ArduinoJson::JsonObjectConst;
using JsonVariant       = ::ArduinoJson::JsonVariant;
using JsonVariantConst  = ::ArduinoJson::JsonVariantConst;
using JsonPair          = ::ArduinoJson::JsonPair;
using DeserializationError = ::ArduinoJson::DeserializationError;
#endif

// v6 함수 재노출
using ::ArduinoJson::deserializeJson;
using ::ArduinoJson::measureJson;
using ::ArduinoJson::measureJsonPretty;
using ::ArduinoJson::serialized;
using ::ArduinoJson::serializeJson;
using ::ArduinoJson::serializeJsonPretty;

// 헬퍼는 AJv6 타입으로 고정해서 모호성 제거
inline bool jsonObjectContainsKey(const AJv6::JsonObjectCompat& obj, const char* key) {
  return !obj.impl[key].isNull();
}
inline size_t jsonArraySize(const AJv6::JsonArrayCompat& arr) { return arr.impl.size(); }
inline size_t jsonObjectSize(const AJv6::JsonObjectCompat& obj) { return obj.impl.size(); }

// v6 스타일 String append 직렬화 복원
#if defined(String) || __has_include(<WString.h>)
inline size_t serializeJson(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp; size_t n = ::ArduinoJson::serializeJson(doc, tmp); s += tmp; return n;
}
inline size_t serializeJsonPretty(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp; size_t n = ::ArduinoJson::serializeJsonPretty(doc, tmp); s += tmp; return n;
}
#endif

// 선택사항 오타 별칭 매크로
#ifdef ARDUINOJSON_V6COMPAT_ENABLE_TYPO_ALIASES
  #ifndef createnestarray
  #define createnestarray createNestedArray
  #endif
  #ifndef createnestedarray
  #define createnestedarray createNestedArray
  #endif
  #ifndef createnestedobject
  #define createnestedobject createNestedObject
  #endif
  #ifndef createneastedobject
  #define createneastedobject createNestedObject
  #endif
#endif
