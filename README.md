# ArduinoJson_V6_toV7_Compat

# ArduinoJson V6 → V7 Compatibility Wrapper

이 라이브러리는 [ArduinoJson v7](https://arduinojson.org/v7/) 에서 **v6 스타일 API**를 그대로 사용할 수 있도록 해주는 **호환성 래퍼(Wrapper)** 입니다.  
기존 프로젝트의 소스를 거의 수정하지 않고도 ArduinoJson v7 로 마이그레이션할 수 있습니다.

---

## ✨ 주요 특징

- `DynamicJsonDocument`, `StaticJsonDocument`, `BasicJsonDocument` 그대로 사용 가능  
- `createNestedObject()`, `createNestedArray()` → v7 방식으로 자동 변환  
- `containsKey()` → `isNull()` 기반 대체 (호환 유지)  
- `serializeJson(..., String&)` → v6처럼 **append 동작** 복원  
- `capacity()`, `memoryUsage()`, `garbageCollect()`, `shallowCopy()` → 더 이상 의미 없으나 **스텁 제공**  
- `overflowed()` → v7의 권장 API를 그대로 노출  
- `jsonArraySize()`, `jsonObjectSize()` 헬퍼 제공 → v6 스타일 코드 호환  

---

## 📂 설치 방법

1. 이 저장소를 다운로드/클론합니다.
2. Arduino IDE 또는 PlatformIO 프로젝트의 `libraries` 폴더에 넣습니다.
3. 코드에서 `<ArduinoJson.h>` 대신 **`<ArduinoJson_V6toV7_Compat.h>`** 를 include 합니다.

---

## 🚀 사용 예제

### 1. 기본 사용 (v6 스타일 그대로)
```cpp
#include <Arduino.h>
#include <ArduinoJson_V6toV7_Compat.h>

DynamicJsonDocument doc(512);

void setup() {
  Serial.begin(115200);

  JsonObject root = doc.to<JsonObject>();
  root["device"] = "ESP32";

  JsonArray arr = root.createNestedArray("values");
  arr.add(42);
  arr.add(100);

  serializeJsonPretty(doc, Serial);
}

void loop() {}
