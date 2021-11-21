#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

#include "MLX90641_API.h"
#include "MLX9064X_I2C_Driver.h"

#include "detection/detect.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define debug Serial

// mlx90641相关函数变量
#define TA_SHIFT 8

const byte MLX90641_address = 0x33;
uint16_t eeMLX90641[832];
float MLX90641To[192];
int newMlx[192];
uint16_t MLX90641Frame[242];
paramsMLX90641 MLX90641;
int errorno = 0;

boolean isConnected();
void setupSensor();
void frameRefresh();
void initNewMLX();
void sendJson();

// detect
// Vector<Vector<location>> lib = { {} };
int count = 0;
int empty_count = 0;
boolean getLocation(int *mlx, int count, location_frame &lf);
location pre(0, 0);

// wifi
const char *ssid = "TP-LINK_6E13";
const char *password = "1412freedom";
void setup_wifi();

// mqtt
// int count = 0;
const char *mqtt_server = "192.168.1.120";
WiFiClient wificlient;
PubSubClient client(wificlient);
void reconnect();

// ardiono

void setup()
{
    setupSensor();
    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void loop()
{
    count += 1;
    frameRefresh();
    location_frame lf;
    initNewMLX();
    getLocation(newMlx, count, lf);

    // location l1 = lf.left;

    Serial.print(lf.left.x);
    Serial.print(" ");
    Serial.print(pre.x);

    float dis = distance(lf.left, pre);
    Serial.print("dis: ");
    Serial.print(dis);

    pre = lf.left;

    if (!client.connected())
    {
        reconnect();
    }
    char msg[1024];
    snprintf(msg, sizeof(msg), "location:%lf,%f\n dis:%lf", lf.left.x, lf.left.y,dis);
    // Serial.print(lf.left.x);
    client.publish("location", msg);
}

// wifi setup function
void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// i2c function
boolean isConnected()
{
    if (Wire.endTransmission() != 0)
    {
        return (false); //Sensor did not ACK
    }
    return (true);
}

// mlx90641 init function
void setupSensor()
{
    Wire.begin();
    Wire.setClock(400000); //Increase I2C clock speed to 400kHz

    debug.begin(9600); //Fast debug as possible

    while (!debug)
        ; //Wait for user to open terminal
    //debug.println("MLX90640 IR Array Example");

    if (isConnected() == false)
    {
        debug.println("MLX90641 not detected at default I2C address. Please check wiring. Freezing.");
        while (1)
            ;
    }
    //Get device parameters - We only have to do this once
    int status;
    status = MLX90641_DumpEE(MLX90641_address, eeMLX90641);
    errorno = status; //MLX90641_CheckEEPROMValid(eeMLX90641);//eeMLX90641[10] & 0x0040;//

    if (status != 0)
    {
        debug.println("Failed to load system parameters");
        while (1)
            ;
    }

    status = MLX90641_ExtractParameters(eeMLX90641, &MLX90641);
    //errorno = status;
    if (status != 0)
    {
        debug.println("Parameter extraction failed");
        while (1)
            ;
    }

    //Once params are extracted, we can release eeMLX90641 array

    //MLX90641_SetRefreshRate(MLX90641_address, 0x02); //Set rate to 2Hz
    MLX90641_SetRefreshRate(MLX90641_address, 0x05); //Set rate to 4Hz
    //MLX90641_SetRefreshRate(MLX90641_address, 0x07); //Set rate to 64Hz
}

// refresh data of mlx90641 in MLX90641To[192]
void frameRefresh()
{
    // long startTime = millis();

    for (byte x = 0; x < 2; x++)
    {
        MLX90641_GetFrameData(MLX90641_address, MLX90641Frame);
        MLX90641_GetVdd(MLX90641Frame, &MLX90641);
        float Ta = MLX90641_GetTa(MLX90641Frame, &MLX90641);

        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
        float emissivity = 0.95;

        MLX90641_CalculateTo(MLX90641Frame, &MLX90641, emissivity, tr, MLX90641To);
    }

    // long stopTime = millis();
    // Serial.print("read time: ");
    // Serial.print(stopTime - startTime);
    // Serial.println("ms");
}

// mqtt reconnect function
void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("arduinoClient"))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("outTopic", "hello world");
            // ... and resubscribe
            client.subscribe("inTopic");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

// get the location of one frame
boolean getLocation(int *mlx, int count, location_frame &lf)
{
    lf.count = count;
    int result[192] = {0};
    // 算子遍历后数组结果
    for (int i = 2; i <= 9; i++)
    {
        for (int j = 2; j <= 13; j++)
        {
            int ret = (-2) * mlx[(i - 2) * 16 + j - 2] + (-2) * mlx[(i - 2) * 16 + j + 2] + (-2) * mlx[(i + 2) * 16 + j - 2] + (-2) * mlx[(i - 2) * 16 + j + 2]

                      + (-1) * mlx[(i - 2) * 16 + j - 1] + (-1) * mlx[(i - 2) * 16 + j] + (-1) * mlx[(i - 2) * 16 + j + 1]

                      + (-1) * mlx[(i - 1) * 16 + j - 2] + (-1) * mlx[(i)*16 + j - 2] + (-1) * mlx[(i + 1) * 16 + j - 2]

                      + (-1) * mlx[(i - 1) * 16 + j + 2] + (-1) * mlx[(i)*16 + j + 2] + (-1) * mlx[(i + 1) * 16 + j + 2]

                      + (-1) * mlx[(i + 2) * 16 + j - 1] + (-1) * mlx[(i + 2) * 16 + j] + (-1) * mlx[(i + 2) * 16 + j + 1]

                      + (2) * mlx[(i - 1) * 16 + j] + (2) * mlx[(i - 1) * 16 + j + 1]

                      + (2) * mlx[(i + 1) * 16 + j]

                      + (2) * mlx[(i - 1) * 16 + j - 1] + (2) * mlx[(i)*16 + j - 1] + (2) * mlx[(i + 1) * 16 + j - 1]

                      + (2) * mlx[(i)*16 + j + 1] + (2) * mlx[(i + 1) * 16 + j + 1]

                      + (4) * mlx[(i)*16 + j];

            result[i * 16 + j] = ret;
        }
    }
    // 提取位置
    int sum = 0;

    for (int i = 0; i < 192; i++)
    {
        sum += mlx[i];
    }

    float threhold = sum / 192.0;
    Serial.print("thread: ");
    Serial.println(threhold);
    int ret = false;
    // 根据算子结果找目标坐标
    int point_number = 0;

    for (int j = 2; j <= 13; j++)
    {
        for (int i = 2; i <= 9; i++)
        {
            if (result[16 * i + j] > threhold)
            {
                // lf.left.x = j;
                // lf.left.y = i;
                int count = 0;
                count += (result[16 * (i - 1) + j - 1] > threhold) ? 1 : 0;
                count += (result[16 * (i - 1) + j] > threhold) ? 1 : 0;
                count += (result[16 * (i - 1) + j + 1] > threhold) ? 1 : 0;

                count += (result[16 * (i + 0) + j - 1] > threhold) ? 1 : 0;
                count += (result[16 * (i + 0) + j + 1] > threhold) ? 1 : 0;

                count += (result[16 * (i + 1) + j - 1] > threhold) ? 1 : 0;
                count += (result[16 * (i + 1) + j] > threhold) ? 1 : 0;
                count += (result[16 * (i + 1) + j + 1] > threhold) ? 1 : 0;
                // Serial.println(count);
                if (count >= 2)
                {
                    ret = true;
                    point_number += 1;
                    lf.left.x = j;
                    lf.left.y = i;
                }
            }
        }
    }

    for (int j = 13; j >= 2; j--)
    {
        for (int i = 2; i <= 9; i++)
        {
            if (result[16 * i + j] > threhold)
            {
                int count = 0;
                // double locX = 0, locY = 0;
                count = count + (result[16 * (i - 1) + j - 1] > threhold) ? 1 : 0;
                // locX += (result[16 * (i - 1) + j - 1] > threhold) ? result[16 * (i - 1) + j - 1] * (j - 1) : 0;
                // locY += (result[16 * (i - 1) + j - 1] > threhold) ? result[16 * (i - 1) + j - 1] * (i - 1) : 0;
                count = count + (result[16 * (i - 1) + j] > threhold) ? 1 : 0;
                // locX += (result[16 * (i - 1) + j] > threhold) ? j : 0;
                // locX += (result[16 * (i - 1) + j] > threhold) ? j : 0;
                count = count + (result[16 * (i - 1) + j + 1] > threhold) ? 1 : 0;

                count = count + (result[16 * (i + 0) + j - 1] > threhold) ? 1 : 0;
                count = count + (result[16 * (i + 0) + j + 1] > threhold) ? 1 : 0;

                count = count + (result[16 * (i + 1) + j - 1] > threhold) ? 1 : 0;
                count = count + (result[16 * (i + 1) + j] > threhold) ? 1 : 0;
                count = count + (result[16 * (i + 1) + j + 1] > threhold) ? 1 : 0;
                if (count >= 2)
                {
                    ret = true;
                    point_number += 1;
                    lf.right.x = j;
                    lf.right.y = i;
                }
            }
        }
    }
    lf.number = point_number;
    return ret;
}

// 发送红外数据jsOn格式
void sendJson()
{

    long t1 = millis();

    DynamicJsonDocument doc(4000);
    long json_time = millis();
    doc["sensor"] = "mlx90641";
    doc["time"] = json_time;
    doc["count"] = count++;
    JsonArray data = doc.createNestedArray("data");
    for (int i = 0; i < 192; i++)
    {
        data.add(int(MLX90641To[i] * 10));
    }

    Serial.print("json size:");
    Serial.println(measureJson(doc));

    String json_str;
    serializeJson(doc, json_str);

    long t2 = millis();

    Serial.print("json parse time: ");
    Serial.print(t2 - t1);
    Serial.println("ms");

    int cut = 256;
    int json_str_len = json_str.length();

    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
    if (json_str_len > cut)
    {
        //开始发送长文件参数分别为  主题，长度，是否持续
        client.beginPublish("mlx", json_str_len, true);
        int count = json_str_len / cut;
        for (int i = 0; i < (count - 1); i++)
        {
            client.print(json_str.substring(i * cut, (i * cut + cut)));
        }
        client.print(json_str.substring(cut * (count - 1)));
        //结束发送文本
        client.endPublish();
    }
    else
    {
        client.publish("mlx", json_str.c_str());
    }

    long t3 = millis();

    Serial.print("send time: ");
    Serial.print(t3 - t2);
    Serial.println("ms");
}

// 将MLX90641To转换成三位有效数字的int
void initNewMLX()
{
    for (int i = 0; i < 192; i++)
    {
        newMlx[i] = int(MLX90641To[i] * 10);
    }
}
