#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);
Adafruit_MPU6050 mpu;

// Accelerometer
float ax = 0;
float ay = 0;
float az = 0;

// Gyroscope
float gx = 0;
float gy = 0;
float gz = 0;

// Temperature
float tempC = 0;

void handleRoot()
{
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 MPU6050 Graph</title>

<style>
body{
  font-family:Arial,sans-serif;
  text-align:center;
}

canvas{
  border:1px solid black;
  width:95%;
  max-width:900px;
}

.info{
  font-size:20px;
  margin:10px;
}
</style>
</head>

<body>

<h2>ESP32 MPU6050 Live Plot</h2>

<div class="info" id="ax">AX:</div>
<div class="info" id="ay">AY:</div>
<div class="info" id="az">AZ:</div>

<div class="info" id="gx">GX:</div>
<div class="info" id="gy">GY:</div>
<div class="info" id="gz">GZ:</div>

<div class="info" id="temp">TEMP:</div>

<canvas id="graph" width="900" height="400"></canvas>

<script>

const canvas = document.getElementById("graph");
const ctx = canvas.getContext("2d");

const MAX_POINTS = 200;

let axData = [];
let ayData = [];
let azData = [];

function pushData(arr,val)
{
    arr.push(val);

    if(arr.length > MAX_POINTS)
        arr.shift();
}

function drawLine(data,color)
{
    ctx.beginPath();
    ctx.strokeStyle = color;

    for(let i=0;i<data.length;i++)
    {
        let x = i * (canvas.width / MAX_POINTS);

        let y = canvas.height/2 - data[i]*15;

        if(i===0)
            ctx.moveTo(x,y);
        else
            ctx.lineTo(x,y);
    }

    ctx.stroke();
}

function drawGraph()
{
    ctx.clearRect(0,0,canvas.width,canvas.height);

    ctx.strokeStyle = "#888";

    ctx.beginPath();
    ctx.moveTo(0,canvas.height/2);
    ctx.lineTo(canvas.width,canvas.height/2);
    ctx.stroke();

    drawLine(axData,"red");
    drawLine(ayData,"green");
    drawLine(azData,"blue");
}

async function updateData()
{
    try
    {
        let response = await fetch('/data');
        let d = await response.json();

        document.getElementById("ax").innerHTML =
            "AX: " + d.ax.toFixed(2);

        document.getElementById("ay").innerHTML =
            "AY: " + d.ay.toFixed(2);

        document.getElementById("az").innerHTML =
            "AZ: " + d.az.toFixed(2);

        document.getElementById("gx").innerHTML =
            "GX: " + d.gx.toFixed(2);

        document.getElementById("gy").innerHTML =
            "GY: " + d.gy.toFixed(2);

        document.getElementById("gz").innerHTML =
            "GZ: " + d.gz.toFixed(2);

        document.getElementById("temp").innerHTML =
            "TEMP: " + d.temp.toFixed(2) + " °C";

        pushData(axData,d.ax);
        pushData(ayData,d.ay);
        pushData(azData,d.az);

        drawGraph();
    }
    catch(err)
    {
        console.log(err);
    }
}

setInterval(updateData,100);

</script>

<p>
Red = AX<br>
Green = AY<br>
Blue = AZ
</p>

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
  json += "\"temp\":" + String(tempC,2);
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

    while(1)
    {
      delay(100);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 Found");

  WiFi.begin(ssid,password);

  Serial.print("Connecting");

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();

  Serial.println("Server Started");
}

void loop()
{
  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);

  // Accelerometer (m/s²)
  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  // Gyroscope (rad/s)
  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;

  // Temperature
  tempC = temp.temperature;

  server.handleClient();

  delay(20);
}
