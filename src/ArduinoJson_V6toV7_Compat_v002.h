// ArduinoJson_V6toV7_Compat_v002.h

#pragma once
// ArduinoJson v6 -> v7 호환 래퍼
// 가이드: https://arduinojson.org/v7/how-to/upgrade-from-v6/

#include <ArduinoJson.h>
#if __has_include(<Arduino.h>)
  #include <Arduino.h>
#elif __has_include(<WString.h>)
  #include <WString.h>
#endif

// --- v6 용량 매크로 더미: 컴파일 호환 전용, 사용 지양 ---
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

  // -------- DocCompat: JsonDocument 호환 래퍼 --------
  struct DocCompat : public AJ::JsonDocument {
    explicit DocCompat(size_t /*capacity*/ = 0) : AJ::JsonDocument() {}
    explicit DocCompat(AJ::Allocator* allocPtr) : AJ::JsonDocument(allocPtr) {}

    // v6 제거 API의 안전한 스텁들
    size_t capacity() const { return 0; }                 // v7: 의미 없음
    size_t memoryUsage() const { return 0; }              // v7: 의미 없음
    void   garbageCollect() {}                            // v7: 자동 재사용
    void   shrinkToFit() { AJ::JsonDocument::shrinkToFit(); } // 여전히 존재, 필요 드뭄
    bool   overflowed() const { return AJ::JsonDocument::overflowed(); } // v7 권장 확인

    template <typename T>
    void shallowCopy(const T& /*src*/) {
      // v7: 얕은 복사 불가, 대입은 항상 깊은 복사
      // 가이드를 따라 호출부 리팩토링 권장
    }
  };

  // -------- JsonObject / JsonArray 호환 래퍼 --------
  struct JsonObjectCompat {
    AJ::JsonObject impl;
    JsonObjectCompat() = default;
    JsonObjectCompat(const AJ::JsonObject& o) : impl(o) {}
    operator AJ::JsonObject&() { return impl; }
    operator const AJ::JsonObject&() const { return impl; }
    AJ::JsonObject* operator->() { return &impl; }
    const AJ::JsonObject* operator->() const { return &impl; }
    explicit operator bool() const { return impl.operator bool(); }

    // v6: createNestedArray/Object("k") → v7: ["k"].to<...>()
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

    // v6 containsKey 대체
    bool containsKey(const char* key) const { return !impl[key].isNull(); }
#if __has_include(<Arduino.h>) || __has_include(<WString.h>)
    bool containsKey(const String& key) const { return !impl[key].isNull(); }
#endif
#ifdef PROGMEM
    bool containsKey(const __FlashStringHelper* key) const { return !impl[key].isNull(); }
#endif
    // 가이드 권장 패턴 예시
    // if (impl["k"].is<int>()) { ... }  // 존재 + 타입검사 동시 수행
  };

  struct JsonArrayCompat {
    AJ::JsonArray impl;
    JsonArrayCompat() = default;
    JsonArrayCompat(const AJ::JsonArray& a) : impl(a) {}
    operator AJ::JsonArray&() { return impl; }
    operator const AJ::JsonArray&() const { return impl; }
    AJ::JsonArray* operator->() { return &impl; }
    const AJ::JsonArray* operator->() const { return &impl; }
    explicit operator bool() const { return impl.operator bool(); }

    // v6: createNestedArray/Object() → v7: add<...>()
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
    static constexpr size_t kCapacity = N; // 호환 상수
  };

  // -------- BasicJsonDocument<Allocator> 호환 --------
  // v7 권장: Allocator 상속 → JsonDocument doc(&allocator)
  // 이 래퍼는 v6 코드 시그니처 호환만 제공
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

// v6의 String append 동작 복원
#if defined(String) || __has_include(<WString.h>)
inline size_t serializeJson(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp; size_t n = ::ArduinoJson::serializeJson(doc, tmp); s += tmp; return n;
}
inline size_t serializeJsonPretty(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp; size_t n = ::ArduinoJson::serializeJsonPretty(doc, tmp); s += tmp; return n;
}
#endif
