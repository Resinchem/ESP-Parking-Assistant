//v0.61
// Literal strings
const char *html_footer = R"literal(
  <footer>
    <br>&copy;2026 by Resinchem Tech. All rights reserved. Documentation, source and license available on <a href="https://github.com/Resinchem/ESP-Parking-Assistant" target="_blank">Github</a>.
  </footer>
  </body>
  </html>
)literal";

const char *onboard = R"literal(
  <!-- ========================
        ONBOARDING/MOBILE PAGE 
       ======================== -->   
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*VAR_APP_NAME* Onboarding</title>
      <style>
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
      </style>
  </head>
  <body>
    <h1>*VAR_APP_NAME* Onboarding</h1>
    Version: *VAR_CURRENT_VER* <br><br>
    Please enter your WiFi information below. These are CASE-SENSITIVE and limited to 64 characters each.<br><br>
    <form method="post" enctype="application/x-www-form-urlencoded" action="/onboard">
      <table>
        <tr>
        <td><label for="ssid">SSID:</label></td>
        <td><input type="text" name="ssid" maxlength="64" value="*SSID*" required></td>
        </tr>
        <tr>
        <td><label for="wifipw">Password:</label></td>
        <td><input type="password" name="wifipw" maxlength="64" value="*WIFIPW*"></td>
        </tr>
      </table><br>
      <b>Device Name: </b>Please give this device a unique name from all other devices on your network, including other installs of *VAR_APP_NAME*. 
      This will be used to set the WiFi and OTA hostnames.<br><br>
      A maximum of 16 alphanumeric (a-z, A-Z, 0-9, -) or hyphen characters, no spaces:
      <table>
        <tr>
        <td><label for="devicename">Device Name:</label></td>
        <td><input type="text" name="devicename" maxlength="16" value="*DEVICENAME*" required></td>
        </tr>
      </table>  
      <br><br>
      <input type="submit" value="Submit">
    </form>
  </body></html>
)literal";

const char *onboardpost = R"literal(
  <!-- ===================
        Post Onboard Page 
       =================== -->   
<!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"> 
    <title>*VAR_APP_NAME* Connection Status</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; color: #000000; padding: 20px; line-height: 1.4; }
      .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); max-width: 500px; margin: auto; text-align: center; }
      .status { font-weight: bold; padding: 4px 8px; border-radius: 4px; display: inline-block; margin: 10px 0; }
      .pending { background-color: #ffeeba; color: #856404; }
      .success { background-color: #d4edda; color: #155724; }
      .failed { background-color: #f8d7da; color: #721c24; }
      .btn { text-align: center; background-color:#ADC6C7; font-size: 14px; font-weight: bold; border-radius: 8px; padding: 12px 24px; border: none; cursor: pointer; margin-top: 20px; text-decoration: none; display: inline-block; color: black; }
      .warning-box { margin-top: 15px; padding: 12px; border: 1px dashed #666; background-color: #f9f9f9; border-radius: 6px; font-size: 13px; text-align: left; }
      .info-list { text-align: left; margin: 15px auto; max-width: 320px; font-size: 14px; background: #f5f5f5; padding: 10px; border-radius: 4px; display: none; }
    </style>
  </head>
  <body>
    <div class="card">
      <h3 id="main-title">Onboarding Progress</h3>
      <p id="main-desc">The controller is currently attempting to authenticate and join your local Wi-Fi network. This process may take up to <b>2 minutes</b>! Please wait...</p>
      <div>
        Connection Status: <span id="conn-status" class="status pending">PENDING</span>
      </div>
      <div id="info-display" class="info-list">
        Network SSID: <strong><span id="res-ssid">-</span></strong><br>
        Assigned IP: <strong><span id="res-ip">0.0.0.0</span></strong>
      </div>
      <div id="msg-box" class="warning-box">
        <strong>Notice:</strong> Do not close this browser window or reset your device. Your phone may briefly disconnect from the configuration hotspot while the radio negotiates its connection channel.
      </div>
       <div id="action-area"></div>
    </div>
    <script>
      var pollCount = 0;
      var targetDevice = '*DEVICE_NAME*';
       function runStatusPoll() {
        pollCount++;
        fetch('/onboard-status')
          .then(response => response.json())
          .then(data => handleResponse(data))
          .catch(err => {
            fetch('http://' + targetDevice + '.local/onboard-status')
              .then(res => res.json())
              .then(data => handleResponse(data))
              .catch(e => {
                if (pollCount < 30) {
                  setTimeout(runStatusPoll, 2000);
                } else {
                  document.getElementById('conn-status').innerText = 'TIMEOUT / DISCONNECTED';
                  document.getElementById('conn-status').className = 'status failed';
                  document.getElementById('main-desc').innerText = 'Lost link tracking to the setup host. Please re-verify your network configurations.';
                  document.getElementById('msg-box').innerHTML = '<strong>Connection Timeout:</strong> The controller timed out trying to connect. The setup hotspot has been restarted automatically.';
                  document.getElementById('action-area').innerHTML = '<input type="button" class="btn" value="Try Again" onclick="location.href=\'/\';">';
                }
              });
          });
      }
      function handleResponse(data) {
        if (data.status === 'success') {
          document.getElementById('main-title').innerText = 'Successfully Connected!';
          document.getElementById('main-desc').innerText = 'Onboarding complete! The settings have been processed successfully.';
          document.getElementById('conn-status').innerText = 'SUCCESSFUL';
          document.getElementById('conn-status').className = 'status success';
          document.getElementById('res-ssid').innerText = data.ssid || '*SSID*';
          document.getElementById('res-ip').innerText = data.ip;
          document.getElementById('info-display').style.display = 'block';
          document.getElementById('msg-box').innerHTML = '<strong>Onboarding Complete:</strong> The device profile is locked in and the controller is rebooting. Once your phone reconnects to normal wifi, you can visit the web interface by using the button below or entering the IP address in any browser on the same WiFi network.';
          document.getElementById('action-area').innerHTML = '<a class="btn" href="http://' + data.ip + '">Visit Device</a>';
        } 
        else if (data.status === 'failed') {
          document.getElementById('main-title').innerText = 'Connection Failed';
          document.getElementById('main-desc').innerText = 'The controller was unable to connect to your access point router.';
          document.getElementById('conn-status').innerText = 'FAILED';
          document.getElementById('conn-status').className = 'status failed';
          document.getElementById('msg-box').innerHTML = '<strong>Authentication Failure:</strong> Verify that your SSID and password entries match exactly. Both variables are strictly case-sensitive.';
          document.getElementById('action-area').innerHTML = '<input type="button" class="btn" value="Try Again" onclick="location.href=\'/\';">';
        } 
        else {
          if (pollCount < 30) setTimeout(runStatusPoll, 2000);
        }
      }
      setTimeout(runStatusPoll, 2000);
    </script>
  </body>
  </html>
)literal";

const char *mainpage = R"literal(
  <!-- ================
        MAIN APP PAGE 
       ================ -->   
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Main Page</title>
      <style>
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
      </style>
  </head>
    <body>
    <H1>*VAR_APP_NAME* Main Page</H1>
    Firmware Version: *VAR_CURRENT_VER*<br><br>
    <table border="1">
      <tr><td style="padding-left: 5px; padding-right: 5px;">Device Name:</td>
          <td style="padding-left: 5px; padding-right: 5px; color:#016200;">*DEVICENAME*</td></tr>
      <tr><td style="padding-left: 5px; padding-right: 5px;">WiFi Network:</td>
          <td id="ssidname" style="padding-left: 5px; padding-right: 5px; color:#016200;">*SSID*</td></tr>
      <tr><td style="padding-left: 5px; padding-right: 5px;">MAC Address:</td>
          <td style="padding-left: 5px; padding-right: 5px; color:#016200;">*MACADDR*</td></tr>
      <tr><td style="padding-left: 5px; padding-right: 5px;">IP Address:</td>
          <td style="padding-left: 5px; padding-right: 5px; color:#016200;">*IPADDR*</td></tr>
    </table>
    <!-- Hidden Fields -->
    <input type="hidden" id="whichdata" name="whichdata" value="all">
    <h2>System Configuration</h2>
    <table border=1 cellpadding="5px">
      <tr>
        <td><button type="button" id="btnsystem" name="btnsystem" 
            style="text-align: center; font-size: 14px; border-radius: 8px; width: 140px; height: 30px; background-color:#ADC6C7;" 
            onclick="location.href = './system';">Hardware Settings</button></td>
        <td>GPIO pins, Sensor type(s), Number of LEDs, etc.</td>
      </tr><tr>
        <td><button type="button" id="btnintegrate" name="btnintegrate" 
            style="text-align: center; font-size: 14px; border-radius: 8px; width: 140px; height: 30px; background-color:#ADC6C7;" 
            onclick="location.href = './integrations';">Integrations</button></td>
        <td>MQTT Setup, Home Assistant Discovery (<i>Optional</i>)</td>
      </tr>
    </table>
    <h2>Active Settings</h2>
    <table style="border: 1px solid black; color: #000088;">
      <tr>
        <td><b>Load Defaults:&emsp;</b></td>
        <td>Will load, <i>but not apply</i>, the saved default values.</td>
      </tr><tr>
        <td><b>Apply (or Test):&emsp;</b></td>
        <td>Apply the current values as active. Remains in effect until controller restarted.</td>
      </tr><tr>
        <td><b>Set as Defaults:&emsp;</b></td>
        <td>Will save the current values as the new defaults. <b><font color="red">Controller will reboot</font></b>.</td>
      </tr>
    </table><br>
    <table>
      <tr>
        <td><h3>LED Colors, Effects &amp; Brightness</h3></td>
        <td>&emsp;<a href="/">Refresh &amp; Sync</a></td>
      </tr>
    <table>
    <form id="frmcolor" action="/mainapply" method="post">
      <table border=0>
        <tr>
          <td>Wake Color:</td>
          <td><input type="color" id="colorwake" name="colorwake" style="width: 120px;" title="Set Wake Zone color" value="#00ff00"/></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Active Color:</td>
          <td><input type="color" id="coloractive" name="coloractive" style="width: 120px;" title="Set Active Zone color" value="#ffff00"/></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Parked Color:</td>
          <td><input type="color" id="colorpark" name="colorpark" style="width: 120px;" title="Set Parked Zone color" value="#ff0000"/></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Backup Color:</td>
          <td><input type="color" id="colorbackup" name="colorbackup" style="width: 120px;" title="Set Backup Zone color" value="#ff0000"/></td>
          <td><i>(flashing)</i></td>
        </tr><tr>
          <td>Standby Color:</td>
          <td><input type="color" id="colorstandby" name="colorstandby" style="width: 120px;" title="Set Standby color" value="#0000ff"/></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Effect:</td>
          <td><select id="effect" name="effect"></select></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Running Brightness:</td>
          <td><input type="range" id="activebright" name="activebright" min="10" max="255" step="1" title="Set brightness level when system is active" value="64" 
              style="width: 100%;" oninput="document.getElementById('activebrightval').textContent = this.value"></td>
          <td>&nbsp;<span id="activebrightval" style="color: indigo; font-weight: bold;">64</span></td>
        </tr><tr>
          <td>Standby Brightness:</td>
          <td><input type="range" id="stndbybright" name="stndbybright" min="0" max="255" step="1" title="Set brightness level when system is in standby mode" value="10" 
              style="width: 100%;" oninput="document.getElementById('stndbybrightval').textContent = this.value"></td>
          <td>&nbsp;<span id="stndbybrightval" style="color: indigo; font-weight: bold;">10</span></td>
        </tr>
      </table><br>
      <button type="submit" id="colordfltbtn" name="colordfltbtn" value="colordfltbtn" title="Load last default/boot settings"
              style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 35px; background-color:#a4f086;" 
              formnovalidate>Load Defaults</button>&ensp;
      <button type="submit" id="colorapplybtn" name="colorapplybtn" value="colorapplybtn" title="Make current settings Active"
              style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 35px; background-color:#a4f086;" 
              >Apply</button>&ensp;
      <button type="submit" id="colorsavebtn" name="colorsavetbtn" value="colorsavebtn" title="Save current settings as new defaults"
              style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 35px; background-color:#faadb7;"
              >Set as Defaults</button><br><br>
      <span id="coloractivemsg" style="color: crimson;">&nbsp;</span>
      <br>
    </form>
    <h3 style="margin-bottom: 0; margin-top: 10px;">Sensor Distances</h3>
    Current Units: <span id="uom" style="color: indigo; font-weight: bold;">millimeters</span> (change via Hardware Settings)<br><br>
    <label style="font-family: Arial, sans-serif; display: inline-flex; align-items: center; gap: 10px; cursor: pointer;">
      <span>Sensor Override</span>
      <input type="checkbox" id="sensorOverride"
        style="appearance: none; -webkit-appearance: none; width: 40px; height: 20px; background: radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), #a1a1a1; border-radius: 20px; cursor: pointer; outline: none; transition: background 0.15s ease-in-out; vertical-align: middle; margin: 0;">
    </label>&emsp;<span id="overrideval" style="color: red" font-weight: bold;">&nbsp</span><br><br>   
    <form id="frmdistance" action="/distapply" method="post">
      <input type="hidden" id="usesidesensor" name"usesidesensor" value="0">
      <input type="hidden" id="sidesensorpos" name"sidesensorpos" value="0">
      <b><u>Zone Distances</u></b>&emsp;<span id="uomrange" style="color: #016200">305-4980 mm</span><br><br>
      <table border=0>
        <tr>
          <td>Wake Zone:</td>
          <td><input type="number" id="wakedist" name="wakedist" min="305" max="4980" step="1" style="width: 50px;" title="Wake zone beginning distance" value="4000"></td>
          <td><span id="wakemsg" style="color: red;">&nbsp;</span></td>
        </tr><tr>
          <td>Active Zone:</td>
          <td><input type="number" id="activedist" name="activedist" min="305" max="4980" step="1" style="width: 50px;" title="Active zone beginning distance" value="3000"></td>
          <td><span id="activemsg" style="color: red;">&nbsp;</span></td>
        </tr><tr>
          <td>Parked Zone:</td>
          <td><input type="number" id="parkdist" name="parkdist" min="305" max="4980" step="1" style="width: 50px;" title="Parked zone beginning distance" value="500"></td>
          <td><span id="parkmsg" style="color: red;">&nbsp;</span></td>
        </tr><tr>  
          <td>Backup Zone:</td>
          <td><input type="number" id="backupdist" name="backupdist" min="305" max="4980" step="1" style="width: 50px;" title="Backup zone beginning distance" value="450"></td>
          <td><span id="backupmsg" style="color: red;">&nbsp;</span></td>
        </tr>
      </table><br>
      <b><u>Lateral (side sensor) Distances</u></b>&emsp;<span id="uomrangeside" style="color: #016200">50-1220 mm</span><br>
      <table id="tblsidedist" border=0>
        <tr>
          <td>Left Distance:</td>
          <td><input type="number" id="leftdist" name="leftdist" min="50" max="1220" step="1" style="width: 50px;" title="Side Sensor left distance" value="250"></td>
          <td><span id="leftmsg" style="color: red;">&nbsp;</span></td>
        </tr><tr>
          <td>Right Distance:</td>
          <td><input type="number" id="rightdist" name="rightdist" min="50" max="1220" step="1" style="width: 50px;" title="Side Sensor right distance" value="300"></td>
          <td><span id="rightmsg" style="color: red;">&nbsp;</span></td>
        </tr>
      </table>
      <span id="sideoff" style="color: crimson; font-style: italic;">Side sensor disabled - enable via Hardware Settings<br></span>
      <br>
      <button type="submit" id="distdfltbtn" name="distdfltbtn" value="distdfltbtn" title="Load last default/boot distances"
              style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 35px; background-color:#a4f086;" 
              formnovalidate>Load Defaults</button>&ensp;
      <button type="submit" id="distapplybtn" name="distapplybtn" value="distapplybtn" title="Make current distance Active"
              style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 35px; background-color:#a4f086;" 
              >Apply</button>&ensp;
      <button type="submit" id="distsavebtn" name="distsavetbtn" value="distsavebtn" title="Save current distances as new defaults"
              style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 35px; background-color:#faadb7;"
              >Set as Defaults</button><br><br>
      <span id="distactivemsg" style="color: crimson;">&nbsp;</span><br><br>
    </form>
    <h2 style="margin-top: -10px;">Controller Commands</h2>
    <table border="1" cellpadding="5px">
      <tr>
        <td><button type="button" id="btnrestart" style="text-align: center; background-color:#e0df80; font-size: 14px; border-radius: 8px; width: 140px; height: 28px;" 
              onclick="confirmRestart()">Restart</button></td>
        <td>This will reboot the <i><b>*DEVICENAME*</b></i> controller and reload default boot values.</td>
      </tr><tr>
        <td><button type="button" id="btnupdate" style="text-align: center; background-color:#e0df80; font-size: 14px; border-radius: 8px; width: 140px; height: 28px;"
              onclick="location.href = './firmwareupdate';">Firmware Upgrade</button></td>
        <td>Upload and apply new firmware from a compiled .bin file.&nbsp;<i>(beta feature)</i></td>
      </tr><tr>
        <td><button type="button" id="btnotamode" style="text-align: center; background-color:#e0df80; font-size: 14px; border-radius: 8px; width: 140px; height: 28px;"
              onclick="location.href = './otaupdate';">Arudino OTA</button></td>
        <td><span id="otabtnmsg">Put system in Arduino OTA mode for approx. 20 seconds to flash modified firmware from IDE.</span></td>
      </tr><tr>
        <td><button type="button" id="btnconfigdump" style="text-align: center; background-color:#e0df80; font-size: 14px; border-radius: 8px; width: 140px; height: 28px;"
              onclick="location.href = './configdump';">Config Dump</button></td>
        <td>See contents of the saved configuration file (config.json).</td>
      </tr><tr>
        <td><button type="button" id="btninfo" style="text-align: center; background-color:#e0df80; font-size: 14px; border-radius: 8px; width: 140px; height: 28px;"
              onclick="location.href = './info';">System Info</button></td>
        <td>Plain text output of system settings, default and active variables.</td>
      </tr><tr>
        <td><button type="button" id="btnreset" style="text-align: center; background-color: #fa0f2e; color: #ffffff; font-size: 14px; border-radius: 8px; width: 140px; height: 28px;" 
              onclick="confirmReset()">RESET ALL</button></td>
        <td><b><font color=red>WARNING</font></b>: This will clear all settings, including WiFi, on the <i><b>*DEVICENAME*</b></i> controller.</td>
      </tr>
    </table>
    <script>
      function populateForm(data) {
        const whichData = data.which_data
        const testColorMode = data.test_flag_color
        const testDistMode = data.test_flag_dist;
        const useSideSensors = data.use_side_sensor;
        const uomDistance = Number(data.uom_distance);
        const sensorOverride = Number(data.sensor_override);
        const effectSelect = document.getElementById("effect");
        const colorActiveMsg = document.getElementById("coloractivemsg");
        const distActiveMsg = document.getElementById("distactivemsg");
        const chkOverride = document.getElementById("sensorOverride");
        const sensorMsg = document.getElementById("overrideval");
        const apMode = (data.ap_mode == 1);
        const otaBtnMsg = document.getElementById("otabtnmsg");
        document.getElementById("whichdata").value = whichData;
        if ((whichData == "all") || (whichData == "color")) {
          //Color form
          document.getElementById("colorwake").value = data.color_wake;
          document.getElementById("coloractive").value = data.color_active;
          document.getElementById("colorpark").value = data.color_parked;
          document.getElementById("colorbackup").value = data.color_backup;
          document.getElementById("colorstandby").value = data.color_standby;
          document.getElementById("coloractivemsg").textContent = data.page_color_msg;
          document.getElementById("activebright").value = data.brightness;
          document.getElementById("activebrightval").textContent = data.brightness;
          document.getElementById("stndbybright").value = data.brightness_standby;
          document.getElementById("stndbybrightval").textContent = data.brightness_standby;
          // Handle message delivery and 5-second auto-clear
          if (data.page_color_msg && data.page_color_msg.trim() !== "") {
            colorActiveMsg.innerHTML = data.page_color_msg;
            setTimeout(() => {
              colorActiveMsg.innerHTML = "&nbsp;"; // Use non-breaking space to preserve layout height
            }, 5000);
          }
          if (!testColorMode) {
            // Clear existing dropdown options to prevent creating duplicates
            effectSelect.innerHTML = "";
            if (data.effects) {
              data.effects.forEach(effect => {
                const option = document.createElement("option");
                option.value = effect.effectname;
                option.textContent = effect.effectname;
                effectSelect.appendChild(option);
              });
            }
            effectSelect.value = data.led_effect;
          }
        }  
        if ((whichData == "all") || (whichData == "dist")) {
          const sideTable = document.getElementById("tblsidedist");
          const sideOff = document.getElementById("sideoff");
          const uomRange = document.getElementById("uomrange");
          const uomRangeSide = document.getElementById("uomrangeside");
          const wakeDist = document.getElementById("wakedist");
          const activeDist = document.getElementById("activedist");
          const parkDist = document.getElementById("parkdist");
          const backDist = document.getElementById("backupdist");
          const leftDist = document.getElementById("leftdist");
          const rightDist = document.getElementById("rightdist");
          if (data.page_dist_msg && data.page_dist_msg.trim() !== "") {
            distActiveMsg.innerHTML = data.page_dist_msg;
            setTimeout(() => {
              distActiveMsg.innerHTML = "&nbsp;"; // Use non-breaking space to preserve layout height
            }, 5000);
          }
          if (uomDistance == 1) {
            uom.textContent = "millimeters";
            uomRange.textContent = "(305-4980 mm)";
            uomRangeSide.textContent = "(50-1220 mm)";
            wakeDist.min = "305";
            wakeDist.max = "4980";
            wakeDist.step = "1";
            activeDist.min = "305";
            activeDist.max = "4980";
            activeDist.step = "1";
            parkDist.min = "305";
            parkDist.max = "4980";
            parkDist.step = "1";
            backDist.min = "305";
            backDist.max = "4980";
            backDist.step = "1";
            leftDist.min = "50";
            leftDist.max = "1220";
            leftDist.step = "1";
            rightDist.min = "50";
            rightDist.max = "1220";
            rightDist.step = "1";
            wakeDist.value = data.dist_wake;
            activeDist.value = data.dist_active;
            parkDist.value = data.dist_parked;
            backDist.value = data.dist_backup;
            leftDist.value = data.dist_left;
            rightDist.value = data.dist_right;
          } else {
            uom.textContent = "inches";
            uomRange.textContent = "(12-192 in)";
            uomRangeSide.textContent = "(2-48 in)";
            wakeDist.min = "12";
            wakeDist.max = "192";
            wakeDist.step = "0.1";
            activeDist.min = "12";
            activeDist.max = "192";
            activeDist.step = "0.1";
            parkDist.min = "12";
            parkDist.max = "192";
            parkDist.step = "0.1";
            backDist.min = "12";
            backDist.max = "192";
            backDist.step = "0.1";
            leftDist.min = "2";
            leftDist.max = "48";
            leftDist.step = "0.1";
            rightDist.min = "2";
            rightDist.max = "48";
            rightDist.step = "0.1";
            wakeDist.value = Number(data.dist_wake).toFixed(1);
            activeDist.value = Number(data.dist_active).toFixed(1);
            parkDist.value = Number(data.dist_parked).toFixed(1);
            backDist.value = Number(data.dist_backup).toFixed(1);
            leftDist.value = Number(data.dist_left).toFixed(1);
            rightDist.value = Number(data.dist_right).toFixed(1);
          }
          document.getElementById("wakemsg").textContent = "";
          document.getElementById("activemsg").textContent = "";
          document.getElementById("parkmsg").textContent = "";
          document.getElementById("backupmsg").textContent = "";
          chkOverride.checked = (sensorOverride == 1);
          if (useSideSensors) {
            sideTable.style.display = "";
            sideOff.style.display = "none"; 
            uomRangeSide.style.display = "inline";
            document.getElementById("usesidesensor").value = "1";
            document.getElementById("sidesensorpos").value = Number(data.side_sensor_pos);
          } else {
            sideTable.style.display = "none";
            sideOff.style.display = "inline"; 
            uomRangeSide.style.display = "none";
            leftDist.min = "0";
            rightDist.min = "0";
            leftDist.value = 0;
            rightDist.value = 0;
            document.getElementById("usesidesensor").value = "0";
            document.getElementById("sidesensorpos").value = "0";
          }
          //Set override toggle and control states
          updateToggleVisual(chkOverride);
          if (sensorOverride == 1) {
            sensorMsg.textContent = "Sensors bypassed!  LEDs must be manually controlled.";
          } else {
            sensorMsg.textContent = " ";
          }
          setControlState(chkOverride, !sensorOverride, "distance");
          //Disable Arduino OTA if running in AP mode
          if (apMode) {
            document.getElementById("btnotamode").disabled = true;
            otaBtnMsg.style.color = "#660d70";
            otaBtnMsg.innerHTML = "<i>Requires WiFi/mDNS.  Unavailable when using AP Mode.</i>"; 
            document.getElementById("ssidname").style.color = "#ff0000";
          } else {
            document.getElementById("btnotamode").disabled = false;
          }
        }
      }
      function fetchData() {
        fetch("/mainjson")
          .then(response => response.json())
          .then(data => populateForm(data))
          .catch(error => console.error("Error fetching data: ", error));
      } 
      function toggleSensors(ctrl) {
        const enableOverride = ((ctrl.checked) ? 1 : 0);
        const sensorMsg = document.getElementById("overrideval");
        fetch('/api?sensoroverride=' + enableOverride)
          .then(response => {
            if (response.ok) {
               if (enableOverride == 1) {
                sensorMsg.textContent = "Sensors bypassed!  LEDs must be manually controlled.";
               } else {
                sensorMsg.textContent = " ";
               }
               setControlState(ctrl, !enableOverride, "distance");
            }
          })
      }
      function updateToggleVisual(ctrl) {
        // Swaps the radial gradient background depending on the .checked state
        ctrl.style.background = ctrl.checked 
          ? 'radial-gradient(circle at 30px 10px, #ffffff 6px, transparent 7px), #2196F3' 
          : 'radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), #a1a1a1';
      }
      function setControlState(ctrl, enable, whichSet) {
        let selector = "";
        if (whichSet == "color") {
          selector = "#frmcolor input, #frmcolor select, #frmcolor button";
        } else if (whichSet == "distance") {
          selector = "#frmdistance input, #frmdistance button";
        } else {
          selector = "#frmcolor input, #frmcolor select, #frmcolor button, #frmdistance input, #frmdistance button";
        }
        const controls = document.querySelectorAll(selector);
        controls.forEach(control => {
          if (ctrl && control == ctrl) {
            return;
          }
          control.disabled = !enable;
        });
      }
      function validDistances() {
        let retVal = true;
        const wakeVal = Number(document.getElementById("wakedist").value);
        const activeVal = Number(document.getElementById("activedist").value);
        const parkVal = Number(document.getElementById("parkdist").value);
        const backupVal = Number(document.getElementById("backupdist").value);
        const wakeMsg = document.getElementById("wakemsg");
        const activeMsg = document.getElementById("activemsg");
        const parkMsg = document.getElementById("parkmsg");
        const backupMsg = document.getElementById("backupmsg");
        const useSideSensor = Number(document.getElementById("usesidesensor").value);
        const sideSensorPos = Number(document.getElementById("sidesensorpos").value);
        const leftSideMsg = document.getElementById("leftmsg");
        const rightSideMsg = document.getElementById("rightmsg");
        if (activeVal >= wakeVal) {
          retVal = false;
          activeMsg.textContent = "Active zone must be LESS than Wake zone";
        } else {
          activeMsg.textContent = "";
        }
        if (parkVal >= activeVal) {
          retVal = false;
          parkMsg.textContent = "Parked zone must be LESS than Active zone";
        } else {
          parkMsg.textContent = "";
        }
        if (backupVal >= parkVal) {
          retVal = false;
          backupMsg.textContent = "Backup zone must be LESS than Parked zone";
        } else {
          backupMsg.textContent = "";
        }
        if (useSideSensor) {
          const leftDist = Number(document.getElementById("leftdist").value);
          const rightDist = Number(document.getElementById("rightdist").value);
          if (sideSensorPos == 1) {  
            //right mount
            if (leftDist <= rightDist) {
              leftSideMsg.textContent = "Right-Mounted Sensor:";
              rightSideMsg.textContent = "Left distance must be greater than right distance.";
              retVal = false;
            } else {
              leftSideMsg.textContent = "";
              rightSideMsg.textContent = "";
            }
          } else if (sideSensorPos == 2) {
            if (rightDist <= leftDist) {
              leftSideMsg.textContent = "Left-Mounted Sensor:";
              rightSideMsg.textContent = "Right distance must be greater than left distance.";
              retVal = false;
            } else {
              leftSideMsg.textContent = "";
              rightSideMsg.textContent = "";
            }
          }
        }
        return retVal;
      }
      function confirmRestart() {
        var userConfirmed = confirm("Are you sure you want to restart the display controller?");
        if (userConfirmed) {
          window.location.href = "./restart";
        }
      }
      function confirmReset() {
        var userConfirmed = confirm("Are you SURE you wish to reset the Display Controller?  You will need to onboard again!");
        if (userConfirmed) {
          window.location.href = "./reset";
        }
      }
      document.addEventListener("DOMContentLoaded", fetchData);
      document.addEventListener("DOMContentLoaded", function() {
        const overrideToggle = document.getElementById("sensorOverride");
        if (overrideToggle) {
          overrideToggle.addEventListener("change", function() {
            // Update knob background immediately when user clicks
            updateToggleVisual(this);
            toggleSensors(this);
          });
          }
      });
      document.getElementById("frmcolor").addEventListener("submit", async function (event) {
        event.preventDefault();
        const frm = document.getElementById("frmcolor");
        const btnVal = event.submitter.value;
        const data = {
          page_mode: btnVal,
          color_wake: document.getElementById("colorwake").value,
          color_active: document.getElementById("coloractive").value,
          color_parked: document.getElementById("colorpark").value,
          color_backup: document.getElementById("colorbackup").value,
          color_standby: document.getElementById("colorstandby").value,
          brightness: Number(document.getElementById("activebright").value),
          brightness_standby: Number(document.getElementById("stndbybright").value),
          led_effect: document.getElementById("effect").value
        };
        if ((btnVal == "colordfltbtn") || (btnVal == "colorapplybtn")) {
          try {
            const response = await fetch("/mainapply", { 
              method: "POST", 
              headers: { "Content-Type": "application/json" },
              body: JSON.stringify(data),
            });
            if (response.ok) {
              const resJson = await response.json();
              populateForm(resJson);
            } else {
              throw new Error("Server returned an error status");
            }
          } catch (error) {
            console.error("Submission error:", error);
          }
        } else if (btnVal == "colorsavebtn") {
          var userConfirmed = confirm("Save color settings as new DEFAULTS and reboot?");
          if (userConfirmed) {
            fetch("/mainapply", { 
              method: "POST", 
              headers: { "Content-Type": "application/json" },
              body: JSON.stringify(data),
            })
            .then(response => {
              if (response.ok) {
                return response.text(); 
              }
              throw new Error("Server error during save execution.");
            })
            .then(htmlPageContent => {
              document.open();
              document.write(htmlPageContent);
              document.close();
            })
            .catch(error => {
              console.error("Submission failed:", error);
              alert("Failed to communicate configuration changes to the hardware controller.");
            });
          }
        }
      });
      document.getElementById("frmdistance").addEventListener("submit", async function (event) {
        event.preventDefault();
        const frm = document.getElementById("frmdistance");
        const btnVal = event.submitter.value;
        const data = {
          page_mode: btnVal,
          dist_wake: Number(document.getElementById("wakedist").value),
          dist_active: Number(document.getElementById("activedist").value),
          dist_parked: Number(document.getElementById("parkdist").value),
          dist_backup: Number(document.getElementById("backupdist").value),
          dist_left: Number(document.getElementById("leftdist").value),
          dist_right: Number(document.getElementById("rightdist").value)
        };
        if ((btnVal == "distdfltbtn") || ((btnVal == "distapplybtn") && (validDistances()))) {
          try {
            const response = await fetch("/mainapply", { 
              method: "POST", 
              headers: { "Content-Type": "application/json" },
              body: JSON.stringify(data),
            });
            if (response.ok) {
              const resJson = await response.json();
              populateForm(resJson);
            } else {
              throw new Error("Server returned an error status");
            }
          } catch (error) {
            console.error("Submission error:", error);
          }
        } else if ((btnVal == "distsavebtn") && (validDistances())) {
          var userConfirmed = confirm("Save distance settings as new DEFAULTS and reboot?");
          if (userConfirmed) {
            fetch("/mainapply", { 
              method: "POST", 
              headers: { "Content-Type": "application/json" },
              body: JSON.stringify(data),
            })
            .then(response => {
              if (response.ok) {
                return response.text(); 
              }
              throw new Error("Server error during save execution.");
            })
            .then(htmlPageContent => {
              document.open();
              document.write(htmlPageContent);
              document.close();
            })
            .catch(error => {
              console.error("Submission failed:", error);
              alert("Failed to communicate configuration changes to the hardware controller.");
            });
          }
        }      
      });
    </script>
)literal";

const char *mainsave = R"literal(
  <!-- ------------------------ 
         Main Page Post Page
       ------------------------ -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME* - Defaults Saved</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <h1 id="topheading" style="color: #1e5c14;">*VAR_APP_NAME* Default Settings Updated</h1>
    Firmware Version: *VAR_CURRENT_VER*<br>
    Device Name: *DEVICENAME*<br><br>
    <h2 id="subheading" style="color: #0d4d08;">New defaults saved to configuration file.</h2><br>
    The device will automatically reboot.<br><br> 
    After the boot(s) completes, you may:<br><br>
    <button type="button" id="btnmain" style="text-align: center; font-size: 16px; border-radius: 8px; width: 160px; height: 40px; background-color:#bbbbbb;"
               onclick="document.location='http://*IPADDR*'">Return to Main Page</button><br><br>
    <i>Do not use the browser back button as the page may not reflect most recent data.</i><br><br>       
)literal";

const char *systempage = R"literal(
  <!-- ========================
        HARDWARE SETUP PAGE
       ======================== -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Hardware Setup</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <H1>*VAR_APP_NAME* Hardware Setup</H1>
    Firmware Version: *VAR_CURRENT_VER*<br><br>
    <button type="button" id="btnmain" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 150px; height: 35px;"
             onclick="location.href = '/';"><< Return to Main</button><br><br>       
    <table style="border: 1px solid black; color: #000088;">
    <tr><td>&nbsp;&bull; Verify that GPIO pin selections are appropriate for input/output types.</td></tr>
    <tr><td>&nbsp;&bull; It is recommended to avoid use of strapping pins.</td></tr>
    <tr><td>&nbsp;&bull; <b>GPIO 0 cannot be used for any pins</b></td></tr>
    <tr><td>&nbsp;&bull; No changes are made to the system until you click 'Save & Reboot'</td></tr>
    </table>
    <form id="frmsystem" action="/systemapply" method="post">
      <h2 style="text-decoration: underline;">System Indicators</h2>
      <table border="0">
        <tr>
          <td>ESP32 Onboard LED:</td>
          <td><input type="checkbox" id="onboardled" name="onboardled" 
                style="appearance: none; -webkit-appearance: none; width: 40px; height: 20px; background: radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), 
                      #a1a1a1; border-radius: 20px; cursor: pointer; outline: none; transition: background 0.15s ease-in-out; vertical-align: middle; margin: 0;"></td>
          <td><span id="onboardledsmsg" style="color: #000088;">&emsp;- Use onboard LED to show successful WiFi Connection</span></td>
        </tr><tr id="gpiorow">
          <td>Onboard GPIO Pin:</td>
          <td><input type="number" id="onboardpin" name="onboardpin" min="0" max="39" step="1" style="width: 30px;" value="0"></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>LED Boot Test:</td>
          <td><input type="checkbox" id="bootleds" name="bootleds" 
                style="appearance: none; -webkit-appearance: none; width: 40px; height: 20px; background: radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), 
                      #a1a1a1; border-radius: 20px; cursor: pointer; outline: none; transition: background 0.15s ease-in-out; vertical-align: middle; margin: 0;"></td>
          <td><span id="bootledsmsg" style="color: #000088;">&emsp;- Use LED strip to show boot succcess</span></td>
        </tr>
      </table>
      <h2 style="text-decoration: underline;">LED Strip Configuration</h2>
      <table border="0">
        <tr>
          <td>Number of LEDs:</td>
          <td><input type="number" id="numleds" name="numleds" min="5" max="600" step="1" style="width: 40px;" value="0"></td>
          <td><span id="numledsmsg" style="color: #ff0000;">&nbsp;</span></td>
        </tr><tr>
          <td>Wired On:</td>
          <td><input type="radio" id="wiredleft" name="ledwire" value="0">Left/Bottom</td>
          <td><input type="radio" id="wiredright" name="ledwire" value="1">Right/Top</td>
        </tr><tr>
          <td>Data GPIO Pin:</td>
          <td><input type="number" id="ledpin" name="ledpin" min="0" max="39" step="1" style="width: 30px;" value="0"></td>
          <td><span id="ledgpiomsg" style="color: #ff0000;">&nbsp;</td>
        </tr><tr>
          <td>Max. Amp Draw:</td>
          <td><input type="number" id="amps" name="amps" min="1" max="30" step="0.1" style="width: 45px;" value="0.0">&nbsp;amps</td>
          <td><span id="ampmsg" style="color: #045e04;"><i>(recommend about 80% of power supply rating)</i></span></td>
        </tr>
      </table>
      <H2 style="text-decoration: underline;">Sensor Configuration</H2>
      <table border="0">
        <tr>
          <td style="padding-bottom: 10px;">System Units:</td>
          <td style="padding-bottom: 10px;"><input type="radio" id="uominch" name="uom" value="0">Inches</td>
          <td style="padding-bottom: 10px;"><input type="radio" id="uommm" name="uom" value="1">Millimeters</td>
        </tr>
      </table>
      <button type="button" id="btnCalibrate" style="background-color:#04AA6D; color:white; border-radius: 12px; font-size: 16px; height: 30px;" 
         onclick="confirmCalibrate()">Calibration Mode</button> - See real time sensor data 
      <h3><i>Front Sensor (TFMini)</i></h3>
      <table border="0">
        <tr>
          <td>RX GPIO Pin:</td>
          <td><input type="number" id="frontrxpin" name="frontrxpin" min="1" max="39" step="1" style="width: 30px;" value="16"></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>TX GPIO Pin:</td>
          <td><input type="number" id="fronttxpin" name="fronttxpin" min="1" max="39" step="1" style="width: 30px;" value="17"></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>No Car Debounce:</td>
          <td><input type="number" id="debounce" name="debounce" min="0" max="25" style="width: 35px;" value="0"></td>
          <td><span id="debouncemsg">&emsp;(0-25 cycles)</td>
        </tr><tr>
          <td>Active Park Time:</td>
          <td><input type="number" id="parktime" name="parktime" min="5" max="300" style="width: 40px;" value="60"</td>
          <td>&emsp;(5-300 seconds)</td>
        </tr><tr>
          <td>Active Exit Time:</td>
          <td><input type="number" id="exittime" name="exittime" min="5" max="300" style="width: 40px;" value="5"</td>
          <td>&emsp;(5-300 seconds)</td>
        </tr>
      </table>
      <h3><i>Optional Side Sensor (VL53L0X)</i></h3>
      <table border="0">
        <tr>
          <td>Enable Side Sensor:</td>
          <td><input type="checkbox" id="usesidesensor" name="usesidesensor" 
                style="appearance: none; -webkit-appearance: none; width: 40px; height: 20px; background: radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), 
                      #a1a1a1; border-radius: 20px; cursor: pointer; outline: none; transition: background 0.15s ease-in-out; vertical-align: middle; margin: 0;"></td>
        </tr>
      </table>
      <table id="tblsidesensor" border="0">
        <tr>
          <td>Sensor Data Pin:</td>
          <td><input type="number" id="tofdatpin" name="tofdatpin" min="1" max="39" step="1" style="width: 30px;" value="21"></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Sensor Clock Pin:</td>
          <td><input type="number" id="tofclkpin" name="tofclkpin" min="1" max="39" step="1" style="width: 30px;" value="22"></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td>Mount Location:</td>
          <td><input type="radio" id="sideleft" name="sidepos" value="2">Left Side</td>
          <td><input type="radio" id="sideright" name="sidepos" value="1">Right Side</td>
        </tr>
      </table><br>
      <H2 style="text-decoration: underline;">Access Point Mode</H2>
      <table border="0">
        <tr>
          <td>Enable AP Mode (no WiFI):</td>
          <td><input type="checkbox" id="nowifimode" name="nowifimode" 
                style="appearance: none; -webkit-appearance: none; width: 40px; height: 20px; background: radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), 
                      #a1a1a1; border-radius: 20px; cursor: pointer; outline: none; transition: background 0.15s ease-in-out; vertical-align: middle; margin: 0;"></td>
          <td><span id="apmodemsg" style="color: #045e04; font-style: italic;">&emsp;Enabling will also <b>disable</b> WiFi</span></td>
        </tr>
      </table>
      <table id="tblapmode" border="0">
        <tr>
          <td>Hotspot Name:</td>
          <td><input type="text" id="manualapname" name="manualapname" maxlength="32" style="width: 150px;" value=""></td>
          <td><span id="apnamemsg" style="color: #000088;">&emsp;Up to 32 chars, spaces and symbols (except backslash[\] and quote["])</td>
        </tr><tr>
          <td>Hotspot Password:</td>
          <td><input type="password" id="manualappwd" name="manualappwd" maxlength="64" style="width: 150px;" autocomplete="off" value=""></td>
          <td><span id="appwdmsg" style="color: #000088;">&emsp;(8-64 chars, or leave blank for Open)</span></td>
        </tr>
      </table><br>
      <!-- hidden fields for checkboxes -->
        <input type="hidden" id="bootledsval" name="bootledsval" value="0">
        <input type="hidden" id="onboardledval" name="onboardledval" value="0">
        <input type="hidden" id="usesidesensorval" name="usesidesensorval" value="0">
        <input type="hidden" id="nowifimodeval" name="nowifimodeval" value="0">
      <table>
        <tr>
          <td><button type="button" id="btnreset" name="btnreset" title="Reset all values to current defaults" 
               style="text-align: center; font-size: 16px; border-radius: 8px; width: 140px; height: 40px; background-color:#e0df80;"
               onclick="document.location='http://*IPADDR*/system'">Reset</button>&nbsp&nbsp;</td>
          <td><button type="submit" id="btnsubmit" title="Save current changes to config and reboot controller" 
               style="text-align: center; font-size: 16px; border-radius: 8px; width: 140px; height: 40px; background-color:#FAADB7;">
               Save & Reboot</button>&nbsp&nbsp;</td>
        </tr>
      </table>
    </form>    
    <script>
      let mqttEnabledState = 0;
      function fetchData() {
        fetch("/systemjson")
          .then(response => response.json())
          .then(data => {
            const useOnboard = Number(data.use_onboard_led);
            const useBootLEDs = Number(data.use_boot_leds);
            const useSideSensor = Number(data.use_side_sensor);
            const useNoWiFi = Number(data.no_wifi_mode); 
            mqttEnabledState = Number(data.mqtt_enabled); //Track MQTT active state
            document.getElementById("onboardled").checked = (useOnboard == 1);
            document.getElementById("onboardpin").value = data.onboard_led_pin;
            document.getElementById("bootleds").checked = (data.use_boot_leds == 1);
            document.getElementById("usesidesensor").checked = (useSideSensor == 1);
            document.getElementById("numleds").value = data.led_count;
            if (Number(data.right_led_wiring) == 1) {
              document.getElementById("wiredright").checked = true;
            } else {
              document.getElementById("wiredleft").checked = true;
            }
            document.getElementById("ledpin").value = data.led_data_pin;
            document.getElementById("amps").value = data.amps;
            if (Number(data.uom_distance) == 1) {
              document.getElementById("uommm").checked = true;
            } else {
              document.getElementById("uominch").checked = true;
            }
            document.getElementById("frontrxpin").value = data.tfmini_rx_pin;
            document.getElementById("fronttxpin").value = data.tfmini_tx_pin;
            document.getElementById("debounce").value = data.no_car_debounce;
            document.getElementById("parktime").value = data.led_park_time;
            document.getElementById("exittime").value = data.led_exit_time;
            document.getElementById("tofdatpin").value = data.tof_dat_pin;
            document.getElementById("tofclkpin").value = data.tof_clk_pin;
            if (Number(data.side_sensor_pos) == 1) {
              document.getElementById("sideright").checked = true;
            } else {
              document.getElementById("sideleft").checked = true;
            }
            document.getElementById("nowifimode").checked = (useNoWiFi == 1);
            document.getElementById("manualapname").value = data.manual_ap_name || "";
            document.getElementById("manualappwd").value = data.manual_ap_pwd || "";

            //Hidden checkbox fields
            document.getElementById("bootledsval").value = useBootLEDs;
            document.getElementById("onboardledval").value = useOnboard;
            document.getElementById("usesidesensorval").value = useSideSensor;
            document.getElementById("nowifimodeval").value = useNoWiFi;
            updateToggleVisual(document.getElementById("bootleds"));
            updateToggleVisual(document.getElementById("onboardled"));
            updateToggleVisual(document.getElementById("usesidesensor"));
            updateToggleVisual(document.getElementById("nowifimode"));
          })
          .catch(error => console.error("Error fetching data:", error));
      }
      function updateToggleVisual(ctrl) {
        // Swaps the radial gradient background depending on the .checked state
        ctrl.style.background = ctrl.checked 
          ? 'radial-gradient(circle at 30px 10px, #ffffff 6px, transparent 7px), #2196F3' 
          : 'radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), #a1a1a1';
        if (ctrl.id === "bootleds") {
           document.getElementById("bootledsval").value = (ctrl.checked ? 1 : 0);
        } else if (ctrl.id === "onboardled") {
          const gpioRow = document.getElementById("gpiorow");
          const gpioPin = document.getElementById("onboardpin");
          if (gpioRow) {
            if (ctrl.checked) {
              gpioPin.value = "2";
              gpioPin.min = "1"
              gpioRow.style.display = "";
            } else {
              gpioPin.min = "0"
              gpioPin.value = "0";
              gpioRow.style.display = "none";
            }
          }
          document.getElementById("onboardledval").value = (ctrl.checked ? 1 : 0);
        } else if (ctrl.id === "usesidesensor") {
          const tblside = document.getElementById("tblsidesensor");
          const isChecked = ctrl.checked;
          if (tblside) {
            tblside.style.display = isChecked ? "" : "none";
            document.getElementById("tofdatpin").disabled = !isChecked;
            document.getElementById("tofclkpin").disabled = !isChecked;
          }
          document.getElementById("usesidesensorval").value = (ctrl.checked ? 1 : 0);
        } else if (ctrl.id === "nowifimode") {
          if (ctrl.checked && mqttEnabledState == 1) {
            alert("MQTT integration is currently active!\n\nYou must first disable MQTT on the Integrations page before enabling Access Point Mode.");
            ctrl.checked = false;
            ctrl.style.background = 'radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), #a1a1a1';
            document.getElementById("nowifimodeval").value = 0;
            document.getElementById("tblapmode").style.display = "none";
            return;
          }
          const tblap = document.getElementById("tblapmode");
          const isChecked = ctrl.checked;
          if (tblap) {
            tblap.style.display = isChecked ? "" : "none";
            document.getElementById("manualapname").disabled = !isChecked;
            document.getElementById("manualappwd").disabled = !isChecked;
          }
          document.getElementById("nowifimodeval").value = (ctrl.checked ? 1 : 0);
        }
      }
      function confirmCalibrate() {
        var userConfirmed = confirm("Unsaved changes will be lost!  Continue to Calibration Page?");
        if (userConfirmed) {
          window.location.href = "./calibrate";
        }
      }
      document.addEventListener("DOMContentLoaded", fetchData);
      document.addEventListener("DOMContentLoaded", function() {
        const toggleIds = ["bootleds", "onboardled", "usesidesensor", "nowifimode"];
        toggleIds.forEach(id => {
          const toggle = document.getElementById(id);
          if (toggle) {
            updateToggleVisual(toggle);
            toggle.addEventListener("change", function() {
              updateToggleVisual(this);
            });
          }
        });
      });
      document.getElementById("frmsystem").addEventListener("submit", async function (event) {
        event.preventDefault();
        const useAP = document.getElementById("nowifimode").checked;
        if (useAP) {
          const apName = document.getElementById("manualapname").value.trim();
          const apPwd = document.getElementById("manualappwd").value;

          if (apName === "") {
            alert("Hotspot Name cannot be blank when Access Point Mode is enabled.");
            return;
          }
          if (/["\\]/.test(apName)) {
            alert("Hotspot Name cannot contain quotes (\") or backslashes (\\).");
            return;
          }
          if (apPwd.length > 0 && apPwd.length < 8) {
            alert("Hotspot Password must be either left blank (for an Open network) or at least 8 characters long.");
            return;
          }
        }
        var okToSave = confirm("Save current settings and reboot controller?");
        if (okToSave) {
          this.action = "/systemapply";
          this.method = "POST";
          this.submit();
          return;
        }
      });
    </script>
)literal";

const char *postsystem = R"literal(
  <!-- ------------------------
        HARDWARE UPDATED PAGE
       ------------------------ -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Hardware Settings Submitted</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <h1 id="topheading" style="color: #1e5c14;">*VAR_APP_NAME* Hardware Settings Updated</h1>
    Firmware Version: *VAR_CURRENT_VER*<br>
    Device Name: *DEVICENAME*<br><br>
    <h2 id="subheading" style="color: #0d4d08;">System changes saved to configuration file.</h2><br>
    The device will automatically reboot.<br><br> 
    After the boot(s) completes, you may:<br><br>
    <button type="button" id="btnmain" style="text-align: center; font-size: 16px; border-radius: 8px; width: 160px; height: 40px; background-color:#bbbbbb;"
               onclick="document.location='http://*IPADDR*'">Return to Main Page</button><br><br>
    <i>Do not use the browser back button as the page may not reflect most recent data.</i><br>
)literal";

const char *calibratepage = R"literal(
  <!-- ========================
        SENSOR CALIBRATION PAGE
       ======================== -->  
  <!DOCTYPE html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME* - Sensor Calibration</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
      table, td, th { border: 1px solid black; text-align: center; font-size:150%; }
    </style>
  </head>
  <body>
  <H1>*VAR_APP_NAME* Sensor Calibration</H1>
  Firmware Version: *VAR_CURRENT_VER*<br>
  Device Name: *DEVICENAME*<br><br>
  <button type="button" id="btnmain" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 150px; height: 35px;"
           onclick="location.href = '/system';"><< Return to Settings</button><br><br>       
  Use this page for determining optimal sensor distances.  Note that during calibration:
  <ul style="color: #000088;">
    <li> Sensor distance(s) will be updated once per second</li>
    <li> Values of <b>9999/999</b> means that measurement is out-of-range (no object detected)</li>
    <li> <b>Err</b> means that no reading was received</li>
    <li> <b>N/A</b> for the side sensor means it is not available or enabled</li>
    <li> No values or settings are saved during calibration</li>
    <li> <i>LEDs will not display and MQTT/API updates are disabled</i></li>    
  </ul>
  <H3><font color="red"><i>Do not close this page!  Use the button above to return system to normal operation!</i></font></H3>
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

const char *integratepage = R"literal(
  <!-- ========================
        INTEGRATIONS PAGE
       ======================== -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: System Integrations</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <H1>*VAR_APP_NAME* Optional System Integrations</H1>
    Firmware Version: *VAR_CURRENT_VER*<br>
    Device Name: *DEVICENAME*<br><br>
    <button type="button" id="btnreturn" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 140px; height: 35px;"
             onclick="location.href = '/';"><< Return to Main</button><br>       
    <form id="frmsystem" action="/integrateapply" method="post">
      <H2 style="text-decoration: underline;">MQTT Integration</H2>
      <ul style="color: #000088;">
        <li>WiFi is required for MQTT Integration</li>
        <li>ONLY enter this information if you already have an MQTT broker configured.</li>
        <li><i>When disabled, the IP address will be set to 0.0.0.0</i></li>
        <li><font color=red>Setting the IP address to "0.0.0.0" will also disable MQTT.</font></li>
      </ul>
      <table border="0">
        <tr>
          <td>Enable MQTT:</td>
          <td><input type="checkbox" id="enablemqtt" name="enablemqtt" style="appearance: none; -webkit-appearance: none; width: 40px; height: 20px; background: radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px),
           #a1a1a1; border-radius: 20px; cursor: pointer; outline: none; transition: background 0.15s ease-in-out; vertical-align: middle; margin: 0;"></td>
          <td>&emsp;<span id="nowifimsg" style="color: #660d70; font-size: 16px; font-weight: bold; font-style: italic;">&nbsp;</span></td>
        </tr>
      </table><br>
      <input type="hidden" id="enablemqttval" name="enablemqttval" value="0">
      <table>
        <tr>
          <td><label for="mqttaddr1">Broker IP Address:</label></td>
          <td><input type="number" min="0" max="255" step="1" id="mqttaddr1" name="mqttaddr1" 
               style="width: 40px;" value="0" disabled>.
              <input type="number" min="0" max="255" step="1" id="mqttaddr2" name="mqttaddr2" style="width: 40px;" value="0" disabled>. 
              <input type="number" min="0" max="255" step="1" id="mqttaddr3" name="mqttaddr3" style="width: 40px;" value="0" disabled>.
              <input type="number" min="0" max="255" step="1" id="mqttaddr4" name="mqttaddr4" style="width: 40px;" value="0" disabled></td>
          <td><input type="text" id="mqttdisabled" name="mqttdisabled" style="color: #660d70; font-size: 16px; font-weight: bold; font-style: italic; border:none; background-color: #cccccc; 
                     user-select: none; pointer-events: none" tabindex="-1" value="Loading. Please wait..." readonly></td>
        </tr><tr>
          <td><label for="mqttport">MQTT Broker Port:</label></td>
          <td><input type="number" min="0" max="65535" step="1" id="mqttport" name="mqttport" style="width: 50px;" title="MQTT Port" value="1883" disabled></td>
          <td><span id="errmqttport" style="color: red;">&nbsp;</span></td>
        </tr><tr>
          <td><label for="mqttuser">MQTT User Name:</label></td>
          <td><input type="text" id="mqttuser" name="mqttuser" maxlength="64" autocomplete="username" title="User Name for MQTT" value="" disabled></td>
          <td><span id="errmqttuser" style="color: red;">&nbsp;</span></td>
        </tr><tr>
          <td><label for="mqttpw">MQTT Password:</label></td>
          <td><input type="password" id="mqttpw" name="mqttpw" maxlength="64" autocomplete="off" title="Password for MQTT" value="" disabled></td>
          <td>&nbsp;</td>
        </tr><tr>
          <td><label for="mqtttopicsub">MQTT Subscribe Topic:&nbsp;cmnd/</label></td>
          <td><input type="text" id="mqtttopicsub" name="mqtttopicsub" maxlength="16" value="" title="The system will subscribe to this topic to receive commands" disabled></td>
          <td><span id="errmqttsub">(16 alphanumeric chars max - no spaces, no symbols)</span></td>
        </tr><tr>
          <td><label for="mqtttopicpub">MQTT Publish Topic:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;stat/</label></td>
          <td><input type="text" id="mqtttopicpub" name="mqtttopicpub" maxlength="16" value="" title="The system will publish state changes to this topic" disabled></td>
          <td><span id="errmqttpub">(16 alphanumeric chars max - no spaces, no symbols)</span></td>
        </tr><tr id="telemetry">
          <td><label for="mqttperiod">Idle Telemetry Period:</label></td>
          <td><input type="number" min="60" max="600" step="1" id="mqttperiod" name="mqttperiod" 
               style="width: 40px;" value="" title="How often to refresh states when idle. Actual state changes will always be published immediately">
               &nbsp;seconds</td>
          <td><span id="errtele">(60 min, 600 max)</td>
        </tr>
      </table><br>
      <table>
        <tr>
          <td><button type="button" id="btnreset" name="btnreset" title="Reset to previously saved values" 
               style="text-align: center; font-size: 16px; border-radius: 8px; width: 140px; height: 40px; background-color:#e0df80;"
               onclick="fetchData('mqtt')">Reset</button>&nbsp&nbsp;</td>
          <td><button type="submit" id="btnsubmit" title="Save current MQTT settings and reboot controller" 
               style="text-align: center; font-size: 16px; border-radius: 8px; width: 140px; height: 40px; background-color:#FAADB7;">
               Save & Reboot</button>&nbsp&nbsp;</td>
        </tr>
      </table>
    </form><br>
    <form id="frmdiscover" action="/savediscovery" method="post">
      <H2 style="text-decoration: underline;">Home Assistant Discovery</H2>
      <div id="discblock">
        This will automatically create a device and desired entities in Home Assistant.<br>
        &emsp;- <i>Certain prerequisites must be met</i> (See documentation for full details).<br>
        &emsp;- <font color="red">You should configure all hardware settings (number of LEDs, sensor config, etc.) BEFORE enabling.</font><br><br>
        The device name can be up to <i>32 alphanumeric characters and spaces</i>:
        <ul style="margin-top: 5px;">
          <li>Can be the same or different than the system Device Name</li>
          <li>It will become the device name in Home Assistant</li>
          <li>It will also be prepended to all entity names</li>
        </ul>
        <input type="hidden" id="discexists" name="discexists" value="0">
        <table border="0">
          <tr>
            <td>Device Name:</td>
            <td><input type="text" id="devicename" name="devicename" maxlength="32" pattern="[a-zA-Z0-9\s]+" title="Up to 32 Alphanumeric characters and spaces"></td>
          </tr>          
        </table><br>
        <font style="color: #126e22">Select the entities you wish to include in Home Assistant.  Omitted entities can still be used via YAML/MQTT.</font><br><br>
        <table border="0">
          <tr>
            <td style="vertical-align: top;"><input type="checkbox" id="controls" name="controls" value="controls" style="vertical-align: top;"></td>
            <td style="vertical-align: top;"><label for="controls"><u>Controls</u></label><br>
                &ensp;- LED Strip (state, color, brightness)&emsp;<br>
                &ensp;- Sensor Override<br>&nbsp;
            </td>  
            <td style="vertical-align: top;"><input type="checkbox" id="sensors" name="sensors" value="sensors"></td>
            <td style="vertical-align: top;"><label for="Sensors"><u>Sensors</u></label><br>
                &ensp;- Front Sensor Distance<br>
                &ensp;- Side Sensor Distance (if enabled)<br>
                &ensp;- Current Occupied Zone<br>
                &ensp;- Car Presence
            </td>
          </tr><tr>
            <td style="vertical-align: top;"><input type="checkbox" id="config" name="config" value="config"></td>
            <td style="vertical-align: top;"><label for="config"><u>Configuration</u></label><br>
                &ensp;- Zone Colors<br>
                &ensp;- Zone Distances<br>
                &ensp;- Active Zone Effect<br>
                &ensp;- Sleep Brightness
            </td>
            <td style="vertical-align: top;"><input type="checkbox" id="diagnostics" name="diagnostics" value="diagnostics"></td>
            <td style="vertical-align: top;"><label for="Diagnostic"><u>Diagnostics</u></label><br>
                &ensp;- Controller IP address<br>
                &ensp;- Controller MAC address<br>
                &ensp;- Reboot Button<br>
            </td>
          </tr>
        </table><br>
        <input type="hidden" id="discaction" name="discaction" value="">
        <button type="submit" id="btneanble" name="btnenable" value="save" title="Enable or update Discovery"
          style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 45px; background-color:#99f2a9;" 
          >Enable Discovery
        </button>&ensp;
        <button type="submit" id="btndisable" name="btndisable" value="delete" title="Disable Discovery and remove device"
          style="text-align: center; font-size: 16px; border-radius: 10px; width: 128px; height: 45px; background-color:#f2a2a5;" 
          >Disable Discovery
        </button>&ensp;
      </div>
      <span id="discdisabled" style="color: #660d70; font-size: 16px; font-weight: bold; font-style: italic;"></span> 
    </form>
  <script>
    let noWiFiModeState = 0;
    function fetchData(whichData) {
      fetch("/integratejson")
        .then(response => response.json())
        .then(data => {
          noWiFiModeState = Number(data.no_wifi_mode);
          if ((whichData == "all") || (whichData == "mqtt")) {
            const mqttOn = Number(data.mqtt_enabled);
            const mqttMsg = document.getElementById("mqttdisabled");
            if (noWiFiModeState == 1) {
              document.getElementById("enablemqtt").checked = false;
              document.getElementById("enablemqtt").disabled = true;
              document.getElementById("enablemqttval").value = "0";
              document.getElementById("nowifimsg").textContent = "Unavailable in Access Point Mode (WiFi Required)";
              mqttMsg.value = "";
              setFieldAccess(false);
            } else {
              document.getElementById("enablemqtt").disabled = false;
              document.getElementById("enablemqtt").checked = (mqttOn == 1);
              const mqttIP1 = document.getElementById("mqttaddr1");
              const mqttIP2 = document.getElementById("mqttaddr2");
              const mqttIP3 = document.getElementById("mqttaddr3");
              const mqttIP4 = document.getElementById("mqttaddr4");
              const mqttPort = document.getElementById("mqttport");
              const mqttUser = document.getElementById("mqttuser");
              const mqttPW = document.getElementById("mqttpw");
              const mqttTopicSub = document.getElementById("mqtttopicsub");
              const mqttTopicPub = document.getElementById("mqtttopicpub");
              const mqttPeriod = document.getElementById("mqttperiod");
              //MQTT
              mqttIP1.value = data.mqtt_addr_1;
              mqttIP2.value = data.mqtt_addr_2;
              mqttIP3.value = data.mqtt_addr_3;
              mqttIP4.value = data.mqtt_addr_4;
              mqttPort.value = data.mqtt_port;
              mqttUser.value = data.mqtt_user;
              mqttPW.value = data.mqtt_pw;
              mqttTopicSub.value = data.mqtt_topic_sub;
              mqttTopicPub.value = data.mqtt_topic_pub;
              mqttPeriod.value = data.mqtt_tele_period;
              if (data.mqtt_enabled) {
                if (data.mqtt_connected) {
                  mqttMsg.value = "Enabled & Connected";
                  mqttMsg.style.color = "#1e5c14";
                } else {
                  mqttMsg.value = "Enabled - Not Connected!";
                  mqttMsg.style.color = "#a67307";
                }
              } else {
                mqttMsg.value = "";
              }
            }
            updateToggleVisual(document.getElementById("enablemqtt"));
          } 
          if ((whichData == "all") || (whichData == "discovery")) {
            document.getElementById("devicename").value = data.disc_devname;   
            document.getElementById("controls").checked = data.disc_controls;
            document.getElementById("sensors").checked = data.disc_sensors;
            document.getElementById("config").checked = data.disc_config;
            document.getElementById("diagnostics").checked = data.disc_diag;
            if (data.disc_exists) {
              document.getElementById("devicename").disabled = true;
              document.getElementById("btneanble").innerHTML = "Update Discovery";
              document.getElementById("btndisable").innerHTML = "Remove Discovery";
              document.getElementById("btndisable").hidden = false;
              document.getElementById("discexists").value = "1";
            } else {
              document.getElementById("discexists").value = "0";
              document.getElementById("btndisable").hidden = true;
            }
          }
          //Show or hide Discovery based on MQTT status
          if ((data.mqtt_enabled) && (noWiFiModeState == 0)) {
            document.getElementById("discblock").style.display = "block";
            document.getElementById("discdisabled").textContent = "";
          } else if (noWiFiModeState == 1) {
            document.getElementById("discblock").style.display = "none";
            document.getElementById("discdisabled").textContent = "MQTT and Discovery are unavailable when running in Access Point Mode!";            
          } else {
            document.getElementById("discblock").style.display = "none";
            document.getElementById("discdisabled").textContent = "MQTT must be enabled and connected before Discovery can be configured!";
          }
        })
        .catch(error => console.error("Error fetching data:", error));
    }
    function updateToggleVisual(ctrl) {
      // Swaps the radial gradient background depending on the .checked state
      ctrl.style.background = ctrl.checked
        ? 'radial-gradient(circle at 30px 10px, #ffffff 6px, transparent 7px), #2196F3' 
        : 'radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), #a1a1a1';
      if (ctrl.id === "enablemqtt") {
        if (ctrl.checked && noWiFiModeState == 1) {
          alert("Access Point Mode is currently active. MQTT requires WiFi!\n\nYou must first disable Access Point Mode on the Hardware Setup page before MQTT can be enabled.");
          ctrl.checked = false;
          ctrl.style.background = 'radial-gradient(circle at 10px 10px, #ffffff 6px, transparent 7px), #a1a1a1';
          document.getElementById("enablemqttval").value = "0";
          setFieldAccess(false);
          return;
        }        
        document.getElementById("enablemqttval").value = (ctrl.checked ? 1 : 0);
        setFieldAccess(ctrl.checked);
      }
    }
    function setFieldAccess(enable) {
      const controls = document.querySelectorAll("#frmsystem input");
      const enableToggle = document.getElementById("enablemqtt");
      controls.forEach(control => {
        if (control.id !== "enablemqtt") {
          control.disabled = !enable;
        }
      });
    }
    function validData(event) {
      let dataOK = true;
      let ctrlName = "";
      //Guard check against submitting MQTT when in AP Mode
      if (noWiFiModeState == 1) {
        alert("MQTT integration cannot be saved while Access Point Mode is active.");
        if (event && event.preventDefault) event.preventDefault();
        return;
      }
      //MQTT
      const mqttOn = Number(document.getElementById("enablemqttval").value);
      const discExistsElem = document.getElementById("discexists");
      const discExists = discExistsElem ? (discExistsElem.value === "1") : false;
      let mqttIP1 = Number(document.getElementById("mqttaddr1").value);
      let mqttIP2 = Number(document.getElementById("mqttaddr2").value);
      let mqttIP3 = Number(document.getElementById("mqttaddr3").value);
      let mqttIP4 = Number(document.getElementById("mqttaddr4").value);
      if (!mqttOn && discExists) {
        alert("Home Assistant Discovery is currently active!\n\nYou must first remove the Discovered device before MQTT can be disabled.");
        dataOK = false;
      } else if ((mqttIP1 > 0) || (mqttIP2 > 0) || (mqttIP3 > 0) || (mqttIP4 > 0)) {
        const mqttPort = document.getElementById("mqttport");
        const mqttSub = document.getElementById("mqtttopicsub"); 
        const mqttPub = document.getElementById("mqtttopicpub");
        if (mqttPort.value.trim() === "") {
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqttport";
          mqttPort.style = "background-color: #f5f39d;";
          document.getElementById("errmqttport").textContent = "Port is required for MQTT integration!";
        }
        if (mqttSub.value.trim() === "") {
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqtttopicsub";
          mqttSub.style = "background-color: #f5f39d;";
          document.getElementById("errmqttsub").textContent = "Subscribe topic is required for MQTT!";
        } else if ((mqttSub.value.trim().indexOf(" ")) > 0) {
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqtttopicsub";
          mqttSub.style = "background-color: #f5f39d;";
          document.getElementById("errmqttsub").textContent = "Topic cannot contain spaces!";
        } else if ((mqttSub.value.trim().indexOf("/")) == 0) {
          const errSub = document.getElementById("errmqttsub");
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqtttopicsub";
          mqttSub.style = "background-color: #f5f39d;";
          errSub.textContent = "Topic cannot begin with '/'";
          errSub.style ="color: red;"
        }
        if (mqttPub.value.trim() == "") {
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqtttopicpub";
          mqttPub.style = "background-color: #f5f39d;";
          document.getElementById("errmqttpub").textContent = "Publish topic is required for MQTT!";
        } else if ((mqttPub.value.trim().indexOf(" ")) > 0) {
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqtttopicpub";
          mqttPub.style = "background-color: #f5f39d;";
          document.getElementById("errmqttpub").textContent = "Topic cannot contain spaces!";
        } else if ((mqttPub.value.trim().indexOf("/")) == 0) {
          errPub = document.getElementById("errmqttpub");
          dataOK = false;
          if (ctrlName == "") ctrlName = "mqtttopicpub";
          mqttPub.style = "background-color: #f5f39d;";
          errPub.textContent = "Topic cannot begin with '/'";
          errPub.style = "color: red;";
        }
        if (dataOK) {
          var okToReboot = confirm("Changes will be saved and controller will reboot.\n\n Continue?");
          dataOK = okToReboot;
        }
      } else {
        //IP is 0.0.0.0
        if (discExists) {
          alert("Home Assistant Discovery is currently active!\n\nYou must first remove the Discovered device before MQTT can be disabled.");
          dataOK = false;
          if (ctrlName === "") ctrlName = "mqttaddr1";
        } else if (mqttOn) {
          var okToSave = confirm("An IP address of \'0.0.0.0\' will DISABLE MQTT.\n\n Continue anyway?");
          dataOK = okToSave;
        }
      }
      if (!dataOK) {
        if (event && event.preventDefault) event.preventDefault(); // Prevent form submission
        if (ctrlName != "") {
          const targetCtrl = document.getElementById(ctrlName);
          if (targetCtrl) targetCtrl.focus();
        }
      }
    }
    document.addEventListener("DOMContentLoaded", function() { fetchData("all"); });
    document.addEventListener("DOMContentLoaded", function() {
      const toggleIds = ["enablemqtt"];
      toggleIds.forEach(id => {
        const toggle = document.getElementById(id);
        if (toggle) {
          updateToggleVisual(toggle);
          toggle.addEventListener("change", function() {
            if (this.id === "enablemqtt" && !this.checked) {
              const discExistsElem = document.getElementById("discexists");
              const discExists = discExistsElem ? (discExistsElem.value === "1") : false;
              if (discExists) {
                alert("Home Assistant Discovery is currently active!\n\nYou must first remove or disable Discovery before MQTT can be disabled.");
                this.checked = true; // Instantly force checkbox back to checked
                updateToggleVisual(this); // Refresh gradient, enablemqttval, and setFieldAccess()
                return; // Abort further execution
              }
            }
            updateToggleVisual(this);
          });
        }
      });
    });
    document.getElementById("frmsystem").addEventListener("submit", validData);
    document.getElementById("frmdiscover").addEventListener("submit", function(e) {
      const act = e.submitter.value;
      document.getElementById("discaction").value = act;
      if (act === "save") {
        const controls = document.getElementById("controls").checked;
        const sensors = document.getElementById("sensors").checked;
        const config = document.getElementById("config").checked;
        const diagnostics = document.getElementById("diagnostics").checked;
        const discExists = (document.getElementById("discexists").value == "1");
        if (!controls && !sensors && !config && !diagnostics) {
          if (discExists) {
            alert("At least one entity group must be selected to update Discovery.\n\nTo completely remove the device from Home Assistant, click 'Remove Discovery' instead.");
          } else {
            alert("At least one entity group must be selected to enable Discovery.");
          }
          e.preventDefault();
          return;
        }
      }
      const msg = (act === "save") ? "This will IMMEDIATELY add or update the device in Home Assistant. Continue?" : "This will IMMEDIATELY remove this device from Home Assistant. Continue?";
      if (!confirm(msg)) {
        e.preventDefault();
        return;
      }
      if (act === "save") {
        document.getElementById("devicename").disabled = false;
      }
    });
  </script>
)literal";

const char *postIntegrations = R"literal(
  <!-- ----------------------------
        INTEGRATIONS UPDATED PAGE 
       ---------------------------- -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Integrations Submitted</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <h1 id="topheading" style="color: #1e5c14;">*VAR_APP_NAME* Integrations Updated</h1>
    Firmware Version: *VAR_CURRENT_VER*<br>
    Device Name: *DEVICENAME*<br><br>
    <h2 id="subheading" style="color: #0d4d08;">Integration changes saved to configuration file.</h2><br>
    The device will now reboot and load your changes.<br><br> 
    After the boot(s) completes, you may:<br><br>
    <button type="button" id="btnmain" style="text-align: center; font-size: 16px; border-radius: 8px; width: 160px; height: 40px; background-color:#bbbbbb;"
               onclick="document.location='http://*IPADDR*'">Return to Main Page</button><br><br>
    <button type="button" id="btnintegrate" style="text-align: center; font-size: 16px; border-radius: 8px; width: 160px; height: 40px; background-color:#bbbbbb;"
               onclick="document.location='./integrations'">Return to Integrations</button><br><br>
    <i>Do not use the browser back button as the page may not reflect most recent data.</i><br>
)literal";

const char *postDiscoveryAdd = R"literal(
  <!-- -------------------------
        DISCOVERY ENABLED PAGE 
       ------------------------- -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME* - Discovery Settings Submitted</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <h1 id="topheading" style="color: #1e5c14;">*VAR_APP_NAME* Discovery Enabled</h1>
    Firmware Version: *VAR_CURRENT_VER*<br>
    Device Name: *DEVICENAME*<br><br>
    <button type="button" id="btnmain" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 180px; height: 35px;"
      onclick="location.href = '/';"><< Return to Main</button><br><br>       
    <button type="button" id="btnadv" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 180px; height: 35px;"
      onclick="location.href = '/integrations';"><< Return to Integrations</button><br>
    <h2>Discovery Messages sent to Home Assistant!</h2><br>
    If this is your initial discovery, the device should be found in Home Assistant at:<br>
    Settings&xrarr;Devices & Services&xrarr;Integrations&xrarr;MQTT<br>
)literal";  

const char *postDiscoveryRemove = R"literal(
  <!-- -------------------------
        DISCOVERY DISABLED PAGE 
       ------------------------- -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME* - Discovery Removal Submitted</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <h1 id="topheading" style="color: #1e5c14;">*VAR_APP_NAME* Discovery Disabled</h1>
    Firmware Version: *VAR_CURRENT_VER*<br>
    Device Name: *DEVICENAME*<br><br>
    <button type="button" id="btnmain" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 180px; height: 35px;"
      onclick="location.href = '/';"><< Return to Main</button><br><br>       
    <button type="button" id="btnadv" style="text-align: center; background-color:#ADC6C7; font-size: 14px; border-radius: 8px; width: 180px; height: 35px;"
      onclick="location.href = '/integrations';"><< Return to Integrations</button><br>
    <h2>Discovery REMOVAL Messages sent to Home Assistant!</h2><br>
    The device and all entites should have been removed from your Home Assistant<br><br>
    <i><font color="red">Note: If you renamed any entities in Home Assistant after the last Discovery, these may not have been automatically removed.&nbsp;
    You may need to locate and manually remove these using the Home Assistant entities page.</font></i><br>
)literal";  

const char *restart = R"literal(
  <!-- =============================
        CONTROLLER REBOOT PAGE
       ============================= -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Reboot</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <H1>*VAR_APP_NAME* Controller restarting...</H1><br>
      <H3>Please wait</H3><br>
      After the controller completes the boot process, you may use the following to return to the main page:<br><br>
      <a href="http://*IPADDR*">Return to settings</a><br><br>
)literal";

const char *configpage = R"literal(
  <!-- ========================
        CONFIG FILE DUMP PAGE
       ======================== -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Config File Dump</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <H1>*VAR_APP_NAME* Configuration File</H1>
    The following is the raw JSON configuration file loaded <b><u>at last boot</u></b>.<br>
    It does <i>not</i> list current settings or any default settings changed but not yet saved.<br><br>
    <button type="button" id="btncancel" style="text-align: center; font-size: 16px; border-radius: 8px; width: 100px; height: 30px; background-color:#bbbbbb;"
               onclick="document.location='http://*IPADDR*'">&lt;&lt; Back</button><br><br>
    Config File:<br>
    <textarea id="filedump" rows="46" cols="50" readonly>*CONFIGJSON*</textarea><br><br>
    Discovery File (only present if Discovery enabled):<br>
    <textarea id="discdump" rows="10" cols="50">*DISCJSON*</textarea>
  </body>
  </html>
)literal";

const char *infodump = R"literal(
  <!-- =============================
        PLAIN TEXT INFO DUMP (API)
       ============================= -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Information</title>
    <style>
        .ascii-table {
            font-family: "Courier New", Courier, monospace;
            background-color: #f4f4f4;
            color: #333;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            line-height: 1.2;
        }
    </style>
</head>
<body>
<pre class="ascii-table">
Device Name: *DEVICENAME*
IP Address:  *IPADDR*
MAC Address: *MACADDR*
WiFi SSID:   *SSID*
</pre>
<pre class="ascii-table" id="api-info-display">Loading system info...</pre>
</body>
<script>
  function fetchData() {
    fetch("/infojson")
      .then(response => response.json())
      .then (data => {
        let asciiOut = "Hardware & Integration Settings:\n\n";
        asciiOut += "| Setting          | Value      |\n";
        asciiOut += "|------------------+------------|\n";
        for (const [key, value] of Object.entries(data.system_settings)) {
          // Convert to string and pad directly (no quotes)
          const paddedKey = String(key).padEnd(16, ' ');
          const paddedVal = String(value).padEnd(10, ' ');
          asciiOut += `| ${paddedKey} | ${paddedVal} |\n`;
        }
        asciiOut += "|------------------+------------|\n\n";
        asciiOut += "Configuration Variables:\n\n";
        asciiOut += "| Variable            | Default Value       | Active Value        |\n";
        asciiOut += "|---------------------+---------------------+---------------------|\n";   
        data.config_vars.forEach(item => {
          // Pad directly based on the exact width of the column headers
          const paddedVar = String(item.name).padEnd(19, ' ');
          const paddedDef = String(item.default).padEnd(19, ' ');
          const paddedAct = String(item.active).padEnd(19, ' ');

          asciiOut += `| ${paddedVar} | ${paddedDef} | ${paddedAct} |\n`;
        });    
    asciiOut += "|---------------------+---------------------+---------------------|\n";
    // Inject the final string into the <pre> block
    document.getElementById('api-info-display').textContent = asciiOut;
    })
    .catch(error => {
      document.getElementById('api-info-display').textContent = "Error loading info.";
      console.error('Fetch error:', error);    
    });
  }
  document.addEventListener("DOMContentLoaded", fetchData);
</script>
</html>
)literal";

const char *updateFirmware = R"literal(
  <!-- =================
        FIRMWARE UPGRADE 
       ================= --> 
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Firmware Update</title>
    <style>
      body { background-color:#cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <h1>*VAR_APP_NAME* Firmware Update</h1><br>
    <b>IMPORTANT:</b> Please read the release notes for the firmware before upgrading!<br>
    Some upgrades may require onboarding and configuring your device again.<br><br>
    <button type="button" id="btnback" style="font-size: 16px; border-radius: 8px; width: 100px; height: 30px;" onclick="location.href = './'"><< Back</button>
    <br><br>
    <b>Current Firmware Version:</b> *VAR_CURRENT_VER* <br><br>
    <span style="color: red; font-weight: bold;">Verify that file selected is firmware for the <u>*DEVICENAME*</u> controller</span><br><br>
    <form method='POST' enctype='multipart/form-data' id='upload-form'>
      <table style="border: 1px solid black;"><tr>
      <td><input type='file' id='file' name='update'></td>
      <td><input type='submit' value='Update'></td>
      </tr></table>
    </form>
    <br>
    <table style="width:25%">
    <tr><td>
    <div id='prg' style='width:0;color:#d0d1a9;text-align:center'>0%</div>
    </td></tr></table>
    <br>
    If the upload is successful, the controller will automatically reboot. Once complete, the system will reload the main page or you can use the link below.<br>
    Check the version on the main page to assure a successful update.<br<br>
    <a href="./">Return to settings</a><br><br>
    <h3>Basic Troubleshooting</h3>
    Here are a few items of note:<br>
    <ul>
    <li>Depending on the issue, an error message may be displayed that will point to the issue</li>
    <li>If version number does not change, the update failed and the firmware rolled back to prior version.</li>
      <ul>
      <li>Try flashing the firmware again</li>
      <li>Verify you are using a valid .bin firmware file for your ESP board</li>
      <li>Check the firmware version's release notes.
      </ul>
    <li>The controller appeared to reboot, but you can no longer access the web app</li>
      <ul>
      <li>Check to see if the *VAR_APP_NAME*_AP hotspot is broadcasting (you may need to onboard again)</li>
      <li>Assure a new IP address wasn't assigned by your router.  If possible, assign a static/reserved IP for the controller</li>
      </ul>
    <li>If the update continues to fail over-the-air, try flashing via USB if possible (may require onboarding again)</li>
    </ul>
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
            if(w == '100%') {
              prg.innerHTML = "Verifying firmware integrity...";
              prg.style.backgroundColor = 'black';
            }
        });
        req.addEventListener('load', ()=>{
          if (req.status == 200 && req.responseText.trim() === "OK") {
            prg.innerHTML = "Success! Rebooting...";
            prg.style.backgroundColor = 'green';
            prg.style.width = '100%';
            // Redirects browser back to home page after 10 seconds
            setTimeout(()=>{ location.href = './'; }, 10000); 
          } else {
            prg.innerHTML = "Update Failed! Aborted by controller.";
            prg.style.backgroundColor = 'red';
            prg.style.width = '100%';
          }
        });
        req.addEventListener('error', ()=>{
          prg.innerHTML = "Network connection lost during transfer!";
          prg.style.backgroundColor = 'red';
          prg.style.width = '100%';
        });
        req.send(data);
      });
    </script>
)literal";

const char *otapage = R"literal(
  <!-- =============================
        ARDUINO IDE OTA UPDATE PAGE
       ============================= -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: OTA Update</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
    <H1>*VAR_APP_NAME* Arduino OTA Upload...</H1>
    <table border=1 cellpadding="5">
    <tr><td><b>Device Name:</b></td><td>*DEVICENAME*</td></tr>
    <tr><td><b>Firmware Ver:</b></td><td>*VAR_CURRENT_VER*</td></tr>
    </table><br>
    <H2>Start upload from Arduino IDE now</H2>
    <ul>
      <li>LEDs (if connected) should show as alternating red & green.</li>
      <li>If code not is received after approx. 20 seconds, OTA mode will exit and system will return to normal operation.</li>
      <li>If upload is successful, the controller will automatically restart.</li>
      <li>This page will not refresh on its own.</li>
    </ul>
    <br>
    After upload/reboot is complete, you may <a href='./'>Return to Main Settings</a><br>
)literal";

const char *reset = R"literal(
  <!-- =============================
        CONTROLLER FULL RESET PAGE
       ============================= -->
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>*DEVICENAME*: Full Reset</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }
    </style>
  </head>
  <body>
      <H1>*VAR_APP_NAME* Controller Resetting...</H1><br>
      <H3>After this process is complete, you <b>must</b> setup your display controller again:</H3>
      <ul>
        <li>Connect a device to the controller's local access point: *VAR_APP_NAME*_AP</li>
        <li>Open a browser and go to: 192.168.4.1</li>
        <li>Enter your WiFi information and set other default settings values</li>
        <li>Click Save. The controller will reboot and join your WiFi</li>
      </ul><br>
      Once the above process is complete, you can return to the main settings page by rejoining your WiFi and entering the IP address assigned by your router in a browser.<br>
      You will need to reenter all of your settings for the system as all values will be reset to original defaults<br><br>
      <b>This page will NOT automatically reload or refresh</b><br>
)literal";
