

// AdvancedExample.ino
// v6 스타일과 v7 스타일을 나란히 보여주는 예제

// 전역 using 충돌 방지
#define ARDUINOJSON_V6COMPAT_NO_GLOBAL_USING 1
#include <Arduino.h>
#include <ArduinoJson_V6toV7_Compat.h>  // 위 헤더

// 필요 타입만 로컬 별칭으로 가져오기
using AJv6::JsonObjectCompat;
using AJv6::JsonArrayCompat;
using AJv6::DynamicJsonDocument;
template <size_t N> using StaticJsonDocument = AJv6::StaticJsonDocument<N>;

// v6 스타일 문법(호환 래퍼로 동작)
StaticJsonDocument<256> staticDoc_v6;
DynamicJsonDocument     dynamicDoc_v6(512);

// v7 스타일 문법
JsonDocument doc_v7;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("=== ArduinoJson v6 → v7 구문 변경 예제 ===");

  // Case 1: 중첩 객체/배열 생성
  Serial.println("\n--- Case 1: Nested Object & Array Creation ---");
  {
    JsonObjectCompat root = dynamicDoc_v6.to<JsonObjectCompat>();
    JsonObjectCompat device = root.createNestedObject("device");
    device["name"] = "ESP32_Sensor";
    device["location"] = "LivingRoom";

    JsonArrayCompat sensors = device.createNestedArray("sensors");
    JsonObjectCompat tempSensor = sensors.createNestedObject();
    tempSensor["type"] = "temperature";
    tempSensor["unit"] = "C";
    tempSensor["value"] = 24.5;

    JsonObjectCompat humSensor = sensors.createNestedObject();
    humSensor["type"] = "humidity";
    humSensor["unit"] = "%";
    humSensor["value"] = 62.1;

    Serial.println("v6 스타일 JSON:");
    serializeJsonPretty(dynamicDoc_v6, Serial);
    Serial.println();
  }
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

  // Case 2: containsKey() vs is<T>()
  Serial.println("\n--- Case 2: Key Check ---");
  const char* json_str = "{\"id\":101,\"active\":true,\"data\":\"Test\"}";

  {
    DeserializationError err = deserializeJson(staticDoc_v6, json_str);
    if (!err) {
      JsonObjectCompat obj = staticDoc_v6.as<JsonObjectCompat>();
      if (obj.containsKey("id")) {
        Serial.print("v6 ID: ");
        Serial.println(obj.impl["id"].as<int>());
      }
    }
  }
  {
    DeserializationError err = deserializeJson(doc_v7, json_str);
    if (!err) {
      JsonObject obj = doc_v7.as<JsonObject>();
      if (obj["id"].is<int>()) {
        Serial.print("v7 ID: ");
        Serial.println(obj["id"].as<int>());
      }
    }
  }

  // Case 3: String append vs overwrite
  Serial.println("\n--- Case 3: String Serialize ---");
  {
    String s = "prefix:";
    serializeJson(staticDoc_v6, s);  // append 복원
    Serial.print("v6 Append: ");
    Serial.println(s);
  }
  {
    String s = "prefix:";
    serializeJson(doc_v7, s);        // v7 기본 overwrite
    Serial.print("v7 Overwrite: ");
    Serial.println(s);
  }

  // Case 4: capacity/memoryUsage vs overflowed
  Serial.println("\n--- Case 4: Memory Check ---");
  Serial.print("v6 capacity(): ");
  Serial.println(staticDoc_v6.capacity());
  Serial.print("v6 memoryUsage(): ");
  Serial.println(staticDoc_v6.memoryUsage());
  if (doc_v7.overflowed()) {
    Serial.println("v7 overflowed!");
  } else {
    Serial.println("v7 ok!");
  }

  // Case 5: shallowCopy vs deep copy
  Serial.println("\n--- Case 5: Copy ---");
  {
    DynamicJsonDocument d1(256);
    d1["msg"] = "hello";
    DynamicJsonDocument d2(256);
    d2.shallowCopy(d1); // noop
    Serial.print("v6 shallowCopy: ");
    serializeJson(d2, Serial);
    Serial.println();
  }
  {
    JsonDocument d1;
    d1["msg"] = "hello";
    JsonDocument d2;
    d2 = d1; // deep copy
    Serial.print("v7 deep copy: ");
    serializeJson(d2, Serial);
    Serial.println();
  }
}

void loop() {}
