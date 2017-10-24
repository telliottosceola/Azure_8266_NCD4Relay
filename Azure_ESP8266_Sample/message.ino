#include <ArduinoJson.h>

//#include <NCD4Relay.h>

int previousStatus = 300;

void initSensor()
{
  setAddress(0,0,0);
}

bool readMessage(int messageId, char *payload)
{
  int currentStatus = readAllInputs();
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    if(previousStatus == 300){
      //Device just booted
      root["deviceBoot"] = true;
    }else{
      root["deviceBoot"] = false;
    }
    bool inputAlert = false;

    // NAN is not the valid json, change it to NULL
    if (std::isnan(currentStatus))
    {
        root["Input"] = NULL;
    }
    else
    {
        root["Input"] = currentStatus;
        if (currentStatus != previousStatus || previousStatus == 300)
        {
            inputAlert = true;
        }
        previousStatus = currentStatus;
    }
    root.printTo(payload, MESSAGE_MAX_LEN);
    return inputAlert;
}

void parseTwinMessage(char *message)
{
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(message);
    if (!root.success())
    {
        Serial.printf("Parse %s failed.\r\n", message);
        return;
    }

    if (root["desired"]["interval"].success())
    {
        interval = root["desired"]["interval"];
    }
    else if (root.containsKey("interval"))
    {
        interval = root["interval"];
    }
}
