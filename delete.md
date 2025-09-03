// ArduinoJson_V6toV7_Compat_v001.h

#pragma once
// ArduinoJson v6에서 v7로 마이그레이션을 위한 호환성 래퍼 (Wrapper)
// 이 파일을 <ArduinoJson.h> 대신 include하면 기존 v6 코드를 대부분 수정 없이 컴파일할 수 있습니다.
// 공식 업그레이드 가이드: https://arduinojson.org/v7/how-to/upgrade-from-v6/

#include <ArduinoJson.h>
#if __has_include(<Arduino.h>)
  #include <Arduino.h>
#elif __has_include(<WString.h>)
  #include <WString.h>
#endif

// ===============================
// v6 용량 계산 매크로 더미
// v7은 메모리가 동적으로 확장되므로 이러한 매크로는 더 이상 필요하지 않습니다.
// 기존 코드의 컴파일 호환성을 위해 더미로 정의합니다.
// ===============================
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

  // ===============================
  // DocCompat: JsonDocument 호환성 래퍼
  // v6에서 제거된 멤버 함수들을 에뮬레이션합니다.
  // ===============================
  struct DocCompat : public AJ::JsonDocument {
    // v6와의 생성자 시그니처 호환성을 위해 capacity 인자를 받지만 무시합니다.
    explicit DocCompat(size_t /*capacity*/ = 0) : AJ::JsonDocument() {}
    explicit DocCompat(AJ::Allocator* allocPtr) : AJ::JsonDocument(allocPtr) {}

    // v6에서 제거된 멤버 함수들
    size_t capacity() const { return 0; }        // v7은 ElasticDocument로 변경되어 고정 용량 개념이 사라졌습니다.
    size_t memoryUsage() const { return 0; }     // v7에서는 문서 크기를 가늠할 수 있는 함수가 없습니다.
    void   garbageCollect() {}                   // v7은 메모리를 자동으로 재사용하므로 불필요합니다.
    void   shrinkToFit() { AJ::JsonDocument::shrinkToFit(); } // 이 함수는 v7에서도 동일하게 동작합니다.

    template <typename T>
    void shallowCopy(const T& /*src*/) {
      // v7은 얕은 복사(shallow copy)를 지원하지 않습니다.
      // 필요하다면 operator=를 사용하여 깊은 복사(deep copy)를 수행해야 합니다.
      // 이 함수는 컴파일 호환성을 위한 스텁(stub)입니다.
    }
  };

  // ===============================
  // JsonObject / JsonArray 호환 래퍼
  // v6의 `createNestedArray/Object` 함수를 v7 방식으로 변환하여 호출합니다.
  // ===============================
  struct JsonObjectCompat {
    AJ::JsonObject impl;
    JsonObjectCompat() = default;
    JsonObjectCompat(const AJ::JsonObject& o) : impl(o) {}
    operator AJ::JsonObject&() { return impl; }
    operator const AJ::JsonObject&() const { return impl; }
    AJ::JsonObject* operator->() { return &impl; }
    const AJ::JsonObject* operator->() const { return &impl; }
    explicit operator bool() const { return impl.operator bool(); }

    // v6: object.createNestedArray("key") -> v7: object["key"].to<JsonArray>()
    AJ::JsonArray createNestedArray(const char* key) {
      return impl[key].to<AJ::JsonArray>();
    }
    AJ::JsonArray createNestedArray(const __FlashStringHelper* key) {
      return impl[key].to<AJ::JsonArray>();
    }
    // v6: object.createNestedObject("key") -> v7: object["key"].to<JsonObject>()
    AJ::JsonObject createNestedObject(const char* key) {
      return impl[key].to<AJ::JsonObject>();
    }
    AJ::JsonObject createNestedObject(const __FlashStringHelper* key) {
      return impl[key].to<AJ::JsonObject>();
    }

    // v6의 `containsKey()` 함수를 `isNull()` 체크로 대체합니다.
    bool containsKey(const char* key) const {
      return !impl[key].isNull();
    }
    bool containsKey(const __FlashStringHelper* key) const {
      return !impl[key].isNull();
    }
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

    // v6: array.createNestedArray() -> v7: array.add<JsonArray>()
    AJ::JsonArray createNestedArray() {
      return impl.add<AJ::JsonArray>();
    }
    // v6: array.createNestedObject() -> v7: array.add<JsonObject>()
    AJ::JsonObject createNestedObject() {
      return impl.add<AJ::JsonObject>();
    }
  };

  // ===============================
  // DynamicJsonDocument / StaticJsonDocument 호환
  // v6와 동일한 클래스 이름을 제공합니다.
  // ===============================
  struct DynamicJsonDocument : public DocCompat {
    explicit DynamicJsonDocument(size_t capacity = 0) : DocCompat(capacity) {}
    DynamicJsonDocument(const DynamicJsonDocument& o) : DocCompat() { *this = o; }
    DynamicJsonDocument(DynamicJsonDocument&& o) noexcept : DocCompat() { *this = std::move(o); }
    DynamicJsonDocument& operator=(const DynamicJsonDocument& o) {
      AJ::JsonDocument::operator=(o);
      return *this;
    }
    DynamicJsonDocument& operator=(DynamicJsonDocument&& o) noexcept {
      AJ::JsonDocument::operator=(std::move(o));
      return *this;
    }
  };

  template <size_t N>
  struct StaticJsonDocument : public DocCompat {
    StaticJsonDocument() : DocCompat(N) {}
    static constexpr size_t kCapacity = N;
  };

  // ===============================
  // BasicJsonDocument<Allocator> 호환
  // v7의 Allocator 포인터 요구사항을 충족시키기 위해 래핑합니다.
  // ===============================
  template <typename TAllocator = AJ::Allocator>
  struct BasicJsonDocument : public DocCompat {
    using allocator_type = TAllocator;

    // v6 시그니처 유지. capacity 인자는 무시됩니다.
    explicit BasicJsonDocument(size_t /*capacity*/ = 0,
                               const allocator_type& alloc = allocator_type())
      : DocCompat(allocPtrInit_(alloc)), _ownedAlloc_(_allocPtr_) {}

    // 외부 소유의 allocator를 직접 전달하는 생성자
    explicit BasicJsonDocument(allocator_type* externalAlloc)
      : DocCompat(externalAlloc), _allocPtr_(externalAlloc), _ownedAlloc_(nullptr) {}

    ~BasicJsonDocument() {
      if (_ownedAlloc_) delete _ownedAlloc_; // 우리가 생성한 Allocator만 해제합니다.
    }

    const allocator_type& getAllocator() const { return *_allocPtr_; }
    allocator_type* getAllocatorPtr() const { return _allocator_ptr; }

   private:
    allocator_type* _allocator_ptr = nullptr;
    allocator_type* _owned_allocator = nullptr;

    static AJ::Allocator* allocPtrInit_(const allocator_type& alloc) {
      // v7은 JsonDocument(&allocator) 형태를 요구합니다.
      // 여기서 힙에 복제본을 만들어 수명 문제를 해결합니다.
      allocator_type* p = new allocator_type(alloc);
      return static_cast<AJ::Allocator*>(p);
    }
  };

} // namespace AJv6

// ===============================
// 외부에 v6 이름으로 노출합니다.
// ===============================
template <size_t N>
using StaticJsonDocument = AJv6::StaticJsonDocument<N>;
using DynamicJsonDocument = AJv6::DynamicJsonDocument;
template <typename TAllocator = ::ArduinoJson::Allocator>
using BasicJsonDocument = AJv6::BasicJsonDocument<TAllocator>;

using JsonArray   = AJv6::JsonArrayCompat;
using JsonObject  = AJv6::JsonObjectCompat;
using JsonPair    = ::ArduinoJson::JsonPair;
using JsonVariant = ::ArduinoJson::JsonVariant;
using DeserializationError = ::ArduinoJson::DeserializationError;

// ===============================
// v6 함수 시그니처 재노출
// ===============================
using ::ArduinoJson::deserializeJson;
using ::ArduinoJson::measureJson;
using ::ArduinoJson::measureJsonPretty;
using ::ArduinoJson::serialized;
using ::ArduinoJson::serializeJson;
using ::ArduinoJson::serializeJsonPretty;

// ===============================
// `containsKey` 대체를 위한 헬퍼 함수
// ===============================
inline bool jsonObjectContainsKey(const JsonObject& obj, const char* key) {
  return !((const ::ArduinoJson::JsonObject&)obj)[key].isNull();
}

// ===============================
// v6의 `String` append 동작 복원
// v7은 `String` 내용을 교체하지만, v6는 내용을 추가했습니다.
// `String&` 오버로드를 추가하여 기존의 동작을 재현합니다.
// ===============================
#if defined(String) || __has_include(<WString.h>)
inline size_t serializeJson(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp;
  size_t n = ::ArduinoJson::serializeJson(doc, tmp);
  s += tmp; // v6처럼 내용을 추가합니다.
  return n;
}
inline size_t serializeJsonPretty(const ::ArduinoJson::JsonDocument& doc, String& s) {
  String tmp;
  size_t n = ::ArduinoJson::serializeJsonPretty(doc, tmp);
  s += tmp; // v6처럼 내용을 추가합니다.
  return n;
}
#endif

