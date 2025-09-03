

// ArduinoJson_V6toV7_Compat_v002.h
#pragma once
// ArduinoJson v6 -> v7 호환 래퍼
// 참고: https://arduinojson.org/v7/how-to/upgrade-from-v6/
//
// ⚠️ 주의사항
// 이 래퍼는 "컴파일 호환"을 위한 것이며,
// 실제 동작은 모두 v7 API에 위임됩니다.
// capacity()/memoryUsage()/shallowCopy() 등은 더 이상 의미가 없습니다.
// 장기적으로는 v7 API로 직접 마이그레이션 권장합니다.
//

#include <ArduinoJson.h>
#if __has_include(<Arduino.h>)
  #include <Arduino.h>
#elif __has_include(<WString.h>)
  #include <WString.h>
#endif

// --- v6 용량 매크로 더미 (컴파일 호환 전용, 사용 지양) ---
#ifndef JSON_OBJECT_SIZE
#define JSON_OBJECT_SIZE(n) 0
#endif
#ifndef JSON_ARRAY_SIZE
#define JSON_ARRAY_SIZE(n) 0
#endif
#ifndef JSON_STRING_SIZE
#define JSON_STRING_SIZE(n) ((size_t)((n) + 1))
#endif

namespace AJv6 {
  using AJ = ::ArduinoJson;

  // -------- DocCompat --------
  struct DocCompat : public AJ::JsonDocument {
    explicit DocCompat(size_t /*capacity*/ = 0) : AJ::JsonDocument() {}
    explicit DocCompat(AJ::Allocator* allocPtr) : AJ::JsonDocument(allocPtr) {}

    // v6 제거 API 스텁
    size_t capacity() const { return 0; }
    size_t memoryUsage() const { return 0; }
    void   garbageCollect() {}
    void   shrinkToFit() { AJ::JsonDocument::shrinkToFit(); }

    // v7에서 권장: overflowed()
    bool   overflowed() const { return AJ::JsonDocument::overflowed(); }

    template <typename T>
    void shallowCopy(const T&) {
      // v7: 얕은 복사 불가, 항상 deep copy
    }
  };

  // -------- JsonObjectCompat --------
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

  // -------- JsonArrayCompat --------
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

  // -------- v6 타입 이름 유지 --------
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

  // -------- BasicJsonDocument<Allocator> --------
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

// --- 외부 노출 별칭 ---
template <size_t N>
using StaticJsonDocument = AJv6::StaticJsonDocument<N>;
using DynamicJsonDocument = AJv6::DynamicJsonDocument;
template <typename TAllocator = ::ArduinoJson::Allocator>
using BasicJsonDocument = AJv6::BasicJsonDocument<TAllocator>;

using JsonArray            = AJv6::JsonArrayCompat;
using JsonArrayConst       = ::ArduinoJson::JsonArrayConst;
using JsonObject           = AJv6::JsonObjectCompat;
using JsonObjectConst      = ::ArduinoJson::JsonObjectConst;
using JsonVariant          = ::ArduinoJson::JsonVariant;
using JsonVariantConst     = ::ArduinoJson::JsonVariantConst;
using JsonPair             = ::ArduinoJson::JsonPair;
using DeserializationError = ::ArduinoJson::DeserializationError;

// v6 함수 시그니처 재노출
using ::ArduinoJson::deserializeJson;
using ::ArduinoJson::measureJson;
using ::ArduinoJson::measureJsonPretty;
using ::ArduinoJson::serialized;
using ::ArduinoJson::serializeJson;
using ::ArduinoJson::serializeJsonPretty;

// containsKey 대체 헬퍼
inline bool jsonObjectContainsKey(const JsonObject& obj, const char* key) {
  return !((const ::ArduinoJson::JsonObject&)obj)[key].isNull();
}

// v6 스타일 array/object size 헬퍼
inline size_t jsonArraySize(const JsonArray& arr) { return arr.impl.size(); }
inline size_t jsonObjectSize(const JsonObject& obj) { return obj.impl.size(); }

// v6의 String append 동작 복원
#if defined(String) || __has_include(<WString.h>)
inline size_t serializeJson(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp; size_t n = ::ArduinoJson::serializeJson(doc, tmp); s += tmp; return n;
}
inline size_t serializeJsonPretty(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp; size_t n = ::ArduinoJson::serializeJsonPretty(doc, tmp); s += tmp; return n;
}
#endif
