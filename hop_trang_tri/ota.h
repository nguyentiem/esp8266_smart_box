#pragma one


const char* ssid = "Tiem";
const char* password = "11111111";

const char* host = "boot";
const char* updatePath = "/update";
const char* updateUsername = "";     //password
const char* updatePassword = "";      // password

//-----------------------------------------//
const char MainPage[] PROGMEM = R"=====(
  <!DOCTYPE html> 
  <html>
   <head> 
       <title>OTA-Nguyen Van Tiem</title> 
       <style> 
          body{
            text-align: center;
          }
       </style>
       <meta name="viewport" content="width=device-width,user-scalable=0" charset="UTF-8">
   </head>
   <body> 
      <div>
        <img src='https://dienthongminhesmart.000webhostapp.com/firmware_ota.jpg' height='200px' width='330px'>
      </div>
      <div>
        <button onclick="window.location.href='/update'">UPLOAD FIRMWARE</button><br><br>

      </div>
      <script>
      </script>
   </body> 
  </html>
)=====";
