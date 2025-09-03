// AdvancedExample.ino

#include <Arduino.h>
#include <ArduinoJson_V6toV7_Compat_v002.h>

// v6 스타일의 DynamicJsonDocument와 StaticJsonDocument를 사용합니다.
// v6에서 메모리 용량을 계산하던 것과 달리, v7 래퍼는 이 용량 인자를 무시합니다.
StaticJsonDocument<256> staticDoc; // v6 문법, 내부적으로는 v7 StaticJsonDocument를 사용
DynamicJsonDocument dynamicDoc(512); // v6 문법, 내부적으로는 v7 ElasticDocument를 사용

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // 시리얼 포트가 열릴 때까지 대기
  }
  Serial.println("ArduinoJson v6 호환성 라이브러리 고급 예제");

  // =======================================================
  // Case 1: 중첩된 객체와 배열 생성 (v6 스타일)
  // =======================================================
  Serial.println("\n--- Case 1: Nested Object & Array Creation ---");
  // 이 문법은 v7에서 to<>()로 대체되었지만, 래퍼 덕분에 그대로 사용 가능합니다.
  JsonObject root = dynamicDoc.to<JsonObject>();

  // v6의 createNestedObject()와 createNestedArray() 함수 사용
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

  Serial.println("생성된 JSON:");
  serializeJsonPretty(dynamicDoc, Serial);
  Serial.println();

  // =======================================================
  // Case 2: containsKey()와 직렬화 (v6 스타일)
  // =======================================================
  Serial.println("\n--- Case 2: containsKey() and Serialization ---");
  const char* json_str = "{\"id\":101,\"active\":true,\"data\":\"Test\"}";
  DeserializationError err = deserializeJson(staticDoc, json_str);

  if (err) {
    Serial.print("역직렬화 오류: ");
    Serial.println(err.c_str());
  } else {
    JsonObject obj = staticDoc.as<JsonObject>();
    
    // v6의 containsKey() 함수 사용
    if (obj.containsKey("id")) {
      Serial.print("ID: ");
      Serial.println(obj["id"].as<int>());
    } else {
      Serial.println("ID 키를 찾을 수 없습니다.");
    }
    
    // v6 스타일의 String 직렬화. serializeJson()은 내용을 덮어쓰지 않고 추가합니다.
    String output = "JSON Data: ";
    size_t len = serializeJson(staticDoc, output); // v6 동작: String에 내용 추가
    Serial.print("직렬화된 문자열 (길이 ");
    Serial.print(len);
    Serial.print("): ");
    Serial.println(output);
  }

  // =======================================================
  // Case 3: 오타 흡수 매크로 테스트 (v6 스타일)
  // =======================================================
  Serial.println("\n--- Case 3: Typo Macro Test ---");
  // 흔한 오타인 'createnestarray'와 'createneastedobject'를 사용
  DynamicJsonDocument doc2(128);
  JsonObject root2 = doc2.to<JsonObject>();
  JsonObject device2 = root2.createneastedobject("device");
  JsonArray values = device2.createnestarray("values");
  values.add(1);
  values.add(2);

  Serial.println("오타 매크로를 사용해 생성된 JSON:");
  serializeJson(doc2, Serial);
  Serial.println();
}

void loop() {
  // 별도 동작 없음
}


