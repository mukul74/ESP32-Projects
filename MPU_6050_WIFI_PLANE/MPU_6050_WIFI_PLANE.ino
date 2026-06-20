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

// Accelerometer
float ax = 0, ay = 0, az = 0;

// Gyroscope
float gx = 0, gy = 0, gz = 0;

// Temperature
float tempC = 0;

// Accelerometer attitude
float roll = 0;
float pitch = 0;

// Gyro attitude
float gyroRoll = 0;
float gyroPitch = 0;

float gyroRollDeg = 0;
float gyroPitchDeg = 0;

unsigned long lastTime = 0;

void handleRoot()
{
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Aircraft Attitude Indicator</title>
<style>
body{
    font-family: Arial,sans-serif;
    text-align:center;
    background:#f5f5f5;
}
.info{
    font-size:20px;
    margin:5px;
}
canvas{
    border:2px solid black;
    background:white;
}
</style>
</head>
<body>

<h2>ESP32 MPU6050 Aircraft Attitude Indicator</h2>

<div class="info" id="ax">AX:</div>
<div class="info" id="ay">AY:</div>
<div class="info" id="az">AZ:</div>

<div class="info" id="gx">GX:</div>
<div class="info" id="gy">GY:</div>
<div class="info" id="gz">GZ:</div>

<div class="info" id="temp">TEMP:</div>

<div class="info" id="roll">ROLL:</div>
<div class="info" id="pitch">PITCH:</div>

<div class="info" id="gyroRoll">GYRO ROLL:</div>
<div class="info" id="gyroPitch">GYRO PITCH:</div>

<br>
<canvas id="attitude" width="500" height="500"></canvas>

<script>

const canvas = document.getElementById("attitude");
const ctx = canvas.getContext("2d");

let roll = 0;
let pitch = 0;

function drawAircraft()
{
    ctx.strokeStyle = "yellow";
    ctx.lineWidth = 4;

    ctx.beginPath();
    ctx.moveTo(180,250);
    ctx.lineTo(320,250);
    ctx.stroke();

    ctx.beginPath();
    ctx.moveTo(250,230);
    ctx.lineTo(250,270);
    ctx.stroke();
}

function drawAttitude()
{
    const w = canvas.width;
    const h = canvas.height;

    ctx.clearRect(0,0,w,h);

    ctx.save();

    ctx.translate(w/2,h/2);
    ctx.rotate(-roll * Math.PI / 180);

    let pitchOffset = pitch * 3;

    ctx.fillStyle = "#4da6ff";
    ctx.fillRect(-1000,-1000 + pitchOffset,2000,1000);

    ctx.fillStyle = "#8B4513";
    ctx.fillRect(-1000,pitchOffset,2000,1000);

    ctx.strokeStyle = "white";
    ctx.lineWidth = 4;
    ctx.beginPath();
    ctx.moveTo(-1000,pitchOffset);
    ctx.lineTo(1000,pitchOffset);
    ctx.stroke();

    ctx.restore();

    ctx.beginPath();
    ctx.lineWidth = 3;
    ctx.strokeStyle = "black";
    ctx.arc(250,250,240,0,2*Math.PI);
    ctx.stroke();

    drawAircraft();
}

async function updateData()
{
    try
    {
        let response = await fetch('/data');
        let d = await response.json();

        document.getElementById("ax").innerHTML = "AX: " + d.ax.toFixed(2);
        document.getElementById("ay").innerHTML = "AY: " + d.ay.toFixed(2);
        document.getElementById("az").innerHTML = "AZ: " + d.az.toFixed(2);

        document.getElementById("gx").innerHTML = "GX: " + d.gx.toFixed(2);
        document.getElementById("gy").innerHTML = "GY: " + d.gy.toFixed(2);
        document.getElementById("gz").innerHTML = "GZ: " + d.gz.toFixed(2);

        document.getElementById("temp").innerHTML =
            "TEMP: " + d.temp.toFixed(2) + " °C";

        document.getElementById("roll").innerHTML =
            "ROLL: " + d.roll.toFixed(2) + " °";

        document.getElementById("pitch").innerHTML =
            "PITCH: " + d.pitch.toFixed(2) + " °";

        document.getElementById("gyroRoll").innerHTML =
            "GYRO ROLL: " + d.gyroRoll.toFixed(2) + " °";

        document.getElementById("gyroPitch").innerHTML =
            "GYRO PITCH: " + d.gyroPitch.toFixed(2) + " °";

        roll = d.roll;
        pitch = d.pitch;

        drawAttitude();
    }
    catch(err)
    {
        console.log(err);
    }
}

updateData();
setInterval(updateData,100);

</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void handleData()
{
  String json = "{";

  json += "\"ax\":" + String(ax,2) + ",";
  json += "\"ay\":" + String(ay,2) + ",";
  json += "\"az\":" + String(az,2) + ",";

  json += "\"gx\":" + String(gx,2) + ",";
  json += "\"gy\":" + String(gy,2) + ",";
  json += "\"gz\":" + String(gz,2) + ",";

  json += "\"temp\":" + String(tempC,2) + ",";

  json += "\"roll\":" + String(roll,2) + ",";
  json += "\"pitch\":" + String(pitch,2) + ",";

  json += "\"gyroRoll\":" + String(gyroRollDeg,2) + ",";
  json += "\"gyroPitch\":" + String(gyroPitchDeg,2);

  json += "}";

  server.send(200, "application/json", json);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  if(!mpu.begin())
  {
    Serial.println("MPU6050 not found!");
    while(1) delay(100);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  lastTime = millis();
}

void loop()
{
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;

  tempC = temp.temperature;

  roll  = atan2(ay, az) * 180.0 / PI;
  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  gyroRoll += gx * dt;
  gyroPitch += gy * dt;

  gyroRollDeg = gyroRoll * 180.0 / PI;
  gyroPitchDeg = gyroPitch * 180.0 / PI;

  server.handleClient();

  delay(20);
}
