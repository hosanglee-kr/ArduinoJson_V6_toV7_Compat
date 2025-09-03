

// AdvancedExample.ino
//
// 이 예제는 ArduinoJson v6 스타일 코드와
// v7 스타일 코드의 차이를 나란히 보여줍니다.
//
// - v6 스타일: Compat 헤더를 통해 여전히 빌드 가능
// - v7 스타일: 공식 가이드에 맞게 변경된 코드
//
// 참고: https://arduinojson.org/v7/how-to/upgrade-from-v6/

#include <Arduino.h>
#include <ArduinoJson_V6toV7_Compat_v002.h>  // v6 호환성 래퍼

// v6 스타일: 용량(capacity) 인자 사용 (호환 래퍼에서 무시됨)
StaticJsonDocument<256> staticDoc_v6;
DynamicJsonDocument dynamicDoc_v6(512);

// v7 스타일: JsonDocument 하나만 사용, capacity 제거
JsonDocument doc_v7;

void setup() {
  Serial.begin(115200);
  while (!Serial) {;}

  Serial.println("=== ArduinoJson v6 → v7 구문 변경 예제 ===");

  // =======================================================
  // Case 1: Nested Object & Array Creation
  // =======================================================
  Serial.println("\n--- Case 1: 중첩 객체/배열 생성 ---");

  // v6 스타일
  {
    JsonObject root = dynamicDoc_v6.to<JsonObject>();
    JsonObject device = root.createNestedObject("device");
    device["name"] = "ESP32_Sensor";
    device["location"] = "LivingRoom";

    JsonArray sensors = device.createNestedArray("sensors");
    JsonObject tempSensor = sensors.createNestedObject();
    tempSensor["type"] = "temperature";
    tempSensor["unit"] = "C";
    tempSensor["value"] = 24.5;

    JsonObject humSensor = sensors.createNestedObject();
    humSensor["type"] = "humidity";
    humSensor["unit"] = "%";
    humSensor["value"] = 62.1;

    Serial.println("v6 스타일 JSON:");
    serializeJsonPretty(dynamicDoc_v6, Serial);
    Serial.println();
  }

  // v7 스타일
  {
    JsonObject root = doc_v7.to<JsonObject>();
    JsonObject device = root["device"].to<JsonObject>();
    device["name"] = "ESP32_Sensor";
    device["location"] = "LivingRoom";

    JsonArray sensors = device["sensors"].to<JsonArray>();
    JsonObject tempSensor = sensors.add<JsonObject>();
    tempSensor["type"] = "temperature";
    tempSensor["unit"] = "C";
    tempSensor["value"] = 24.5;

    JsonObject humSensor = sensors.add<JsonObject>();
    humSensor["type"] = "humidity";
    humSensor["unit"] = "%";
    humSensor["value"] = 62.1;

    Serial.println("v7 스타일 JSON:");
    serializeJsonPretty(doc_v7, Serial);
    Serial.println();
  }

  // =======================================================
  // Case 2: containsKey() vs is<T>()/isNull()
  // =======================================================
  Serial.println("\n--- Case 2: 키 존재 확인 ---");
  const char* json_str = "{\"id\":101,\"active\":true,\"data\":\"Test\"}";

  // v6 스타일
  {
    DeserializationError err = deserializeJson(staticDoc_v6, json_str);
    if (!err) {
      JsonObject obj = staticDoc_v6.as<JsonObject>();
      if (obj.containsKey("id")) {
        Serial.print("v6 ID: ");
        Serial.println(obj["id"].as<int>());
      }
    }
  }

  // v7 스타일
  {
    DeserializationError err = deserializeJson(doc_v7, json_str);
    if (!err) {
      JsonObject obj = doc_v7.as<JsonObject>();
      // v7 권장: 존재+타입 검사 동시에
      if (obj["id"].is<int>()) {
        Serial.print("v7 ID: ");
        Serial.println(obj["id"].as<int>());
      }
    }
  }

  // =======================================================
  // Case 3: String Append vs Overwrite
  // =======================================================
  Serial.println("\n--- Case 3: String 직렬화 ---");

  // v6 스타일: append (Compat 헤더에서 복원됨)
  {
    String s = "prefix:";
    serializeJson(staticDoc_v6, s);
    Serial.print("v6 Append 결과: ");
    Serial.println(s);
  }

  // v7 스타일: overwrite
  {
    String s = "prefix:";
    serializeJson(doc_v7, s);
    Serial.print("v7 Overwrite 결과: ");
    Serial.println(s);
  }

  // =======================================================
  // Case 4: capacity() vs overflowed()
  // =======================================================
  Serial.println("\n--- Case 4: 메모리 확인 ---");

  // v6: capacity() / memoryUsage() (의미 있었음)
  Serial.print("v6 capacity(): ");
  Serial.println(staticDoc_v6.capacity());
  Serial.print("v6 memoryUsage(): ");
  Serial.println(staticDoc_v6.memoryUsage());

  // v7: overflowed() (용량 개념 사라짐)
  if (doc_v7.overflowed()) {
    Serial.println("v7: 메모리 부족 발생!");
  } else {
    Serial.println("v7: 문서 정상 (용량 개념 없음)");
  }

  // =======================================================
  // Case 5: shallowCopy()
  // =======================================================
  Serial.println("\n--- Case 5: shallowCopy vs deep copy ---");

  // v6: shallowCopy() 지원
  {
    DynamicJsonDocument doc1(256);
    doc1["msg"] = "hello";
    DynamicJsonDocument doc2(256);
    doc2.shallowCopy(doc1);  // v6 스타일 (Compat 헤더에서는 noop)

    Serial.print("v6 shallowCopy 결과: ");
    serializeJson(doc2, Serial);
    Serial.println();
  }

  // v7: 얕은 복사 지원 안 함 → deep copy만 가능
  {
    JsonDocument doc1;
    doc1["msg"] = "hello";
    JsonDocument doc2;
    doc2 = doc1;  // 항상 deep copy
    Serial.print("v7 deep copy 결과: ");
    serializeJson(doc2, Serial);
    Serial.println();
  }
}

void loop() {
}
