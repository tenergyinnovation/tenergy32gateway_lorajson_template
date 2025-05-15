/***********************************************************************
 * Project      :     tenergy32gateway_lorajson_template
 * Description  :     Test program for Tenergy32 Gateway board
 * Hardware     :     Tenergy32GateWay
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     27/04/2025
 ***********************************************************************/
#include <Arduino.h>
#include <tenergy32gateway.h>
#include <esp_task_wdt.h>
#include <ArduinoJson.h>

Tenergy32GateWay mcu;

void header_print(void)
{
    Serial.printf("\r\n***********************************************************************\r\n");
    Serial.printf("* Project      :     tenergy32gateway_lorajson_template\r\n");
    Serial.printf("* Description  :     Template coding for Tenergy32GateWay on PlatformIO\r\n");
    Serial.printf("* Hardware     :     Tenergy32GateWay\r\n");
    Serial.printf("* Author       :     Tenergy Innovation Co., Ltd.\r\n");
    Serial.printf("* Date         :     27/04/2025\r\n");
    Serial.printf("* Revision     :     %s\r\n", mcu._version.c_str());
    Serial.printf("* website      :     http://www.tenergyinnovation.co.th\r\n");
    Serial.printf("* Email        :     uten.boonliam@tenergyinnovation.co.th\r\n");
    Serial.printf("* TEL          :     +66 89-140-7205\r\n");
    Serial.printf("***********************************************************************/\r\n");
}

const char* lora_id_1 = "tenergy32hub-4AC628";
const char* lora_id_2 = "tenergy32hub-4AC629";
const char* lora_id_3 = "tenergy32hub-4AC62A"; // เพิ่ม id ได้ตามต้องการ

// ฟังก์ชันตรวจสอบ id
bool isAllowedId(const char* id) {
    return (strcmp(id, lora_id_1) == 0) ||
           (strcmp(id, lora_id_2) == 0) ||
           (strcmp(id, lora_id_3) == 0);
}

void setup()
{
    Serial.begin(115200);
    header_print();

    // Initialize watchdog timer
    esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);

    if (!mcu.begin())
    {
        Serial.println("Board initialization failed!");
        while (1)
            ;
    }

    // Delay to view initial info
    delay(1000);
}


void loop()
{
    // --- รับข้อมูลจาก LoRa ---
    uint8_t buffer[256]; // ขยาย buffer ให้ใหญ่พอสำหรับ JSON
    int received = 0;
    if (mcu.receiveLoRa(buffer, sizeof(buffer), received) && received > 0)
    {
        int rssi = LoRa.packetRssi();

        // แปลง buffer เป็น string
        buffer[received] = '\0'; // ป้องกัน string overflow
        String jsonStr = (char*)buffer;

        // แกะ JSON
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, jsonStr);
        if (!error) {
            Serial.println("=== LoRa JSON Received ===");
            Serial.printf("id: %s\n", doc["id"] | "");
            Serial.printf("fw: %s\n", doc["fw"] | "");
            Serial.printf("rssi: %s\n", doc["rssi"] | "");
            Serial.printf("topic: %s\n", doc["topic"] | "");
            Serial.printf("counter: %d\n", doc["counter"] | 0);
            for (int i = 1; i <= 10; ++i) {
                String key = "param_" + String(i);
                Serial.printf("%s: %.2f\n", key.c_str(), doc[key] | 0.0);
            }
            Serial.printf("RSSI: %d dBm\n", rssi);

            const char* id = doc["id"] | "";
            if (isAllowedId(id)) {
                Serial.printf("Data size: %d bytes\n", received);

                char oledLine1[32], oledLine2[32], oledLine3[32], oledLine4[32];
                snprintf(oledLine1, sizeof(oledLine1), "id:%s", id);
                snprintf(oledLine2, sizeof(oledLine3), " ");
                snprintf(oledLine3, sizeof(oledLine3), "cnt:%d rssi:%d", doc["counter"] | 0, rssi);
                snprintf(oledLine4, sizeof(oledLine4), "Size: %d bytes", received);
                mcu.displayOLEDLines(oledLine1, oledLine2, oledLine3, oledLine4);
            }
            // ถ้า id ไม่ตรง จะไม่แสดงบน OLED
        } else {
            Serial.println("JSON parse failed!");
            mcu.displayOLED("JSON parse failed!");
        }
    }

    esp_task_wdt_reset();
    delay(100);
}
