#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);
Adafruit_MPU6050 mpu;

// Raw Sensor Data
float ax, ay, az;
float gx, gy, gz;

// Accelerometer Angles
float accelRollDeg = 0;
float accelPitchDeg = 0;

// Gyroscope Angles
float gyroRoll = 0;
float gyroPitch = 0;

float gyroRollDeg = 0;
float gyroPitchDeg = 0;

// Complementary Filter Angles
float fusedRoll = 0;
float fusedPitch = 0;

float fusedRollDeg = 0;
float fusedPitchDeg = 0;

bool firstRun = true;

unsigned long lastTime = 0;

void handleData()
{
    String json = "{";

    json += "\"accelRoll\":" + String(accelRollDeg, 2) + ",";
    json += "\"accelPitch\":" + String(accelPitchDeg, 2) + ",";

    json += "\"gyroRoll\":" + String(gyroRollDeg, 2) + ",";
    json += "\"gyroPitch\":" + String(gyroPitchDeg, 2) + ",";

    json += "\"fusedRoll\":" + String(fusedRollDeg, 2) + ",";
    json += "\"fusedPitch\":" + String(fusedPitchDeg, 2);

    json += "}";

    server.send(200, "application/json", json);
}
void handleRoot()
{
    String page = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>MPU6050 Filter Demo</title>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>
body{
    font-family:Arial;
    text-align:center;
    background:#f0f0f0;
}

.chart-container{
    width:95%;
    max-width:1000px;
    margin:auto;
    background:white;
    padding:15px;
    margin-top:20px;
    border-radius:10px;
}
</style>
</head>

<body>

<h2>MPU6050 Sensor Fusion Demo</h2>

<h3>Roll Comparison</h3>

<div class="chart-container">
<canvas id="rollChart"></canvas>
</div>

<h3>Pitch Comparison</h3>

<div class="chart-container">
<canvas id="pitchChart"></canvas>
</div>

<script>

const MAX_POINTS = 100;

let labels = [];

let accelRollData = [];
let gyroRollData  = [];
let fusedRollData = [];

let accelPitchData = [];
let gyroPitchData  = [];
let fusedPitchData = [];

const rollChart =
new Chart(
document.getElementById('rollChart'),
{
type:'line',
data:
{
labels:labels,
datasets:
[
{
label:'Accel Roll',
data:accelRollData,
borderColor:'blue',
fill:false
},
{
label:'Gyro Roll',
data:gyroRollData,
borderColor:'red',
fill:false
},
{
label:'Fused Roll',
data:fusedRollData,
borderColor:'green',
fill:false
}
]
},
options:
{
animation:false,
responsive:true
}
}
);

const pitchChart =
new Chart(
document.getElementById('pitchChart'),
{
type:'line',
data:
{
labels:labels,
datasets:
[
{
label:'Accel Pitch',
data:accelPitchData,
borderColor:'blue',
fill:false
},
{
label:'Gyro Pitch',
data:gyroPitchData,
borderColor:'red',
fill:false
},
{
label:'Fused Pitch',
data:fusedPitchData,
borderColor:'green',
fill:false
}
]
},
options:
{
animation:false,
responsive:true
}
}
);

function trimArray(arr)
{
    while(arr.length > MAX_POINTS)
    {
        arr.shift();
    }
}

async function updateData()
{
    try
    {
        const response =
            await fetch('/data');

        const d =
            await response.json();

        labels.push("");

        accelRollData.push(d.accelRoll);
        gyroRollData.push(d.gyroRoll);
        fusedRollData.push(d.fusedRoll);

        accelPitchData.push(d.accelPitch);
        gyroPitchData.push(d.gyroPitch);
        fusedPitchData.push(d.fusedPitch);

        trimArray(labels);

        trimArray(accelRollData);
        trimArray(gyroRollData);
        trimArray(fusedRollData);

        trimArray(accelPitchData);
        trimArray(gyroPitchData);
        trimArray(fusedPitchData);

        rollChart.update();
        pitchChart.update();
    }
    catch(err)
    {
        console.log(err);
    }
}

setInterval(updateData,100);

</script>

</body>
</html>
)rawliteral";

    server.send(200,"text/html",page);
}
void setup()
{
    server.on("/", handleRoot);
    Serial.begin(115200);

    Wire.begin();

    if (!mpu.begin())
    {
        Serial.println("MPU6050 not found");

        while (1)
        {
            delay(100);
        }
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    WiFi.begin(ssid, password);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/data", handleData);

    server.begin();

    lastTime = millis();
}

void loop()
{
    unsigned long currentTime = millis();

    float dt = (currentTime - lastTime) / 1000.0f;

    if (dt > 0.1f)
        dt = 0.1f;

    lastTime = currentTime;

    sensors_event_t a, g, temp;

    mpu.getEvent(&a, &g, &temp);

    ax = a.acceleration.x;
    ay = a.acceleration.y;
    az = a.acceleration.z;

    gx = g.gyro.x;
    gy = g.gyro.y;
    gz = g.gyro.z;

    float accelRoll =
        atan2(ay, az);

    float denom =
        sqrt(ay * ay + az * az);

    if (denom < 0.001f)
        denom = 0.001f;

    float accelPitch =
        atan2(-ax, denom);

    if (firstRun)
    {
        fusedRoll = accelRoll;
        fusedPitch = accelPitch;

        firstRun = false;
    }

    gyroRoll += gx * dt;
    gyroPitch += gy * dt;

    fusedRoll =
        0.98f * (fusedRoll + gx * dt) +
        0.02f * accelRoll;

    fusedPitch =
        0.98f * (fusedPitch + gy * dt) +
        0.02f * accelPitch;

    accelRollDeg =
        accelRoll * 180.0f / PI;

    accelPitchDeg =
        accelPitch * 180.0f / PI;

    gyroRollDeg =
        gyroRoll * 180.0f / PI;

    gyroPitchDeg =
        gyroPitch * 180.0f / PI;

    fusedRollDeg =
        fusedRoll * 180.0f / PI;

    fusedPitchDeg =
        fusedPitch * 180.0f / PI;

    server.handleClient();

    delay(20);
}
