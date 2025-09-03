// BasicExample.ino

#include <Arduino.h>
#include <ArduinoJsonV6Compat.h> // v6 호환성 래퍼 라이브러리 include

// v6 스타일의 DynamicJsonDocument를 사용합니다.
DynamicJsonDocument doc(256);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // 시리얼 포트가 열릴 때까지 대기
  }
  Serial.println("ArduinoJson v6 호환성 라이브러리 예제");

  // 1. v6 스타일로 JSON 객체 생성
  // v7에서는 to<>()를 사용해야 하지만, 래퍼 덕분에 이 문법이 가능합니다.
  JsonObject data = doc.to<JsonObject>();

  // 2. 중첩된 객체와 배열 생성
  // v6의 createNestedObject()와 createNestedArray() 함수를 사용합니다.
  JsonObject nestedObj = data.createNestedObject("sensor_data");
  JsonArray readings = nestedObj.createNestedArray("readings");

  // 3. 값 추가
  nestedObj["temperature"] = 25.5;
  nestedObj["humidity"] = 75.0;
  readings.add(101);
  readings.add(102);

  data["status"] = "OK";

  // 4. JSON 문서 직렬화 및 출력 (v6 스타일)
  // serializeJson() 함수를 사용하며, v7의 String 오버로드와 동작이 다릅니다.
  Serial.println("\nJSON 문서 생성 및 출력:");
  serializeJson(doc, Serial);
  Serial.println();

  // 5. JSON 문서 역직렬화 (Deserialization)
  const char* json_string = "{\"node_id\":\"sensor_01\",\"value\":99}";
  DeserializationError error = deserializeJson(doc, json_string);

  if (error) {
    Serial.print("\n역직렬화 실패: ");
    Serial.println(error.c_str());
    return;
  }

  // 6. 역직렬화된 데이터 접근 (v6 스타일)
  // containsKey() 함수로 키 존재 여부를 확인합니다.
  if (data.containsKey("node_id")) {
    Serial.print("\n역직렬화된 데이터: node_id = ");
    Serial.println(data["node_id"].as<String>());
  }
  
  Serial.print("역직렬화된 데이터: value = ");
  Serial.println(data["value"].as<int>());
}

void loop() {
  // 별도 동작 없음
}

