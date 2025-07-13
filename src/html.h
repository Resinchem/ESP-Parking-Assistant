// Literal string
const char *updateHtml = R"literal(
  <!DOCTYPE html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #0000ff; }
    </style>
  </head>
  <body>
    <h1>VAR_APP_NAME Firmware Update</h1><br>
    <b>IMPORTANT:</b> Please read the release notes for the firmware before upgrading!<br>
    Some upgrades may require onboarding and configuring your device again.<br><br>
    <button type="button" id="btnback" style="font-size: 16px; border-radius: 8px; width: 100px; height: 30px;" onclick="location.href = './'"><< Back</button>
    <br><br>
    <b>Current Firmware Version:</b> VAR_CURRENT_VER <br><br><br>
    <form method='POST' enctype='multipart/form-data' id='upload-form'>
      <table style="border: 1px solid black;"><tr>
      <td><input type='file' id='file' name='update'></td>
      <td><input type='submit' value='Update'></td>
      </tr></table>
    </form>
    <br>
    <table style="width:25%">
    <tr><td>
    <div id='prg' style='width:0;color:#cccccc;text-align:center'>0%</div>
    </td></tr></table>
    <br>
    If the upload is successful, the controller will automatically reboot. Once complete, you can use the link below to return to the main page.<br>
    Check the version on the main page to assure a successful update.<br<br>
    <a href="./">Return to settings</a><br><br>
    <h3>Basic Troubleshooting</h3>
    Here are a few items of note:<br>
    <ul>
    <li>Depending on the issue, an error message may be displayed that will point to the issue</li>
    <li>If version number does not change, the update failed and firmware rolled back to prior version.</li>
      <ul>
      <li>Try flashing the firmware again</li>
      <li>Verify you are using a valid firmware file for your ESP board</li>
      <li>Check the firmware version's release notes.
      </ul>
    <li>The controller appeared to reboot, but you can no longer access the web app</li>
      <ul>
      <li>Check to see if the VAR_APP_NAME hotspot is broadcasting (you may need to onboard again)</li>
      <li>If possible, check for the blue LED on the ESP board. If not lit, the controller could not join WiFi</li>
      <li>Assure a new IP address wasn't assigned by your router.  If possible, assign a static/reserved IP for the controller</li>
      </ul>
    <li>If the update continues to fail over-the-air, try flashing via USB if possible (may require onboarding again)</li>
    </ul>
  </body>
  <script>
    var prg = document.getElementById('prg');
    var form = document.getElementById('upload-form');
    form.addEventListener('submit', el=>{
      prg.style.backgroundColor = 'blue';
      el.preventDefault();
      var data = new FormData(form);
      var req = new XMLHttpRequest();
      var fsize = document.getElementById('file').files[0].size;
      req.open('POST', '/update?size=' + fsize);
      req.upload.addEventListener('progress', p=>{
        let w = Math.round(p.loaded/p.total*100) + '%';
          if(p.lengthComputable){
             prg.innerHTML = w;
             prg.style.width = w;
          }
          if(w == '100%') prg.style.backgroundColor = 'black';
      });
      req.send(data);
     });
  </script>
)literal";

const char *otaHtml = R"literal(
  <!DOCTYPE html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #0000ff; }
    </style>
  </head>
  <body>
  <H1>Ready for OTA Upload...</H1>
  Firmware Version: VAR_CURRENT_VER<br><br>
  <H2>Start upload from Arduino IDE now</H2>
  <ul>
    <li>LEDs (if connected) should show alternating red and green color.</li>
    <li>If code not is received after approx. 20 seconds, OTA mode will exit and system will return to normal operation.</li>
    <li>If upload is successful, the controller will automatically restart.</li>
    <li>This page will not refresh on its own.</li>
  </ul>
  <br>
  After upload/reboot is complete, you may <a href='./'>Return to Main Settings</a>
  </body></html>
)literal";

const char *calibrateSensors = R"literal(
  <!DOCTYPE html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #0000ff; }
      table, td, th { border: 1px solid black; text-align: center; font-size:150%; }
    </style>
  </head>
  <body>
  <H1>Sensor Calibration</H1>
  Use this page for determining optimal sensor distances.  Note that during calibration:
  <ul>
    <li> Sensor distance(s) will be updated once per second</li>
    <li> Values of <b>9999/999</b> means that measurement is out-of-range (no object detected)</li>
    <li> <b>Err</b> means that no reading was received</li>
    <li> <b>N/A</b> for the side sensor means it is not available or enabled</li>
    <li> No values or settings are saved during calibration</li>
    <li> <i>LED Display may be delayed/unreliable during calibration</i></li>    
  </ul>
  <H3><i>Close this page when finished to return system to normal operation!</i></H3>
  <H2>Front Sensor Distance</H2>
  <table border='0'>
    <tr>
    <td>Millimeters</td><td>&nbsp;&nbsp;</td><td>Inches</td>
    </tr><tr>
    <td><p id='frontmm'>0</p></td>
    <td>&nbsp;</td>
    <td><p id='frontin'>0</p></td>
    </tr></table>
  <br>
  <H2>Side Sensor Distance</H2>
  <table border='0'>
    <tr>
    <td>Millimeters</td><td>&nbsp;&nbsp;</td><td>Inches</td>
    </tr><tr>
    <td><p id='sidemm'>0</p></td>
    <td>&nbsp;</td>
    <td><p id='sidein'>0</p></td>
    </tr></table>
  <br>
  <a href='./'>Return to Main Settings</a>
  <script>
    function updateSensorValues() {
      fetch('./data')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json(); 
        })
        .then(data => {
            // Update the content of each sensor value element
            document.getElementById('frontmm').innerText = data.frontmm;
            document.getElementById('frontin').innerText = data.frontin;
            document.getElementById('sidemm').innerText = data.sidemm;
            document.getElementById('sidein').innerText = data.sidein;

        })
        .catch(error => {
            console.error('Error fetching sensor data:', error);
            document.getElementById('frontmm').innerText = 'Err';
            document.getElementById('frontin').innerText = 'Err';
            document.getElementById('sidemm').innerText = 'Err';
            document.getElementById('sidein').innerText = 'Err';
        });
    }
    document.addEventListener('DOMContentLoaded', updateSensorValues);
    setInterval(updateSensorValues, 1000);
  </script>  
  </body></html>
)literal";
