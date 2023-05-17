const char _index[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="styles.css">
    <title>ESP_QWiFi</title>
</head>
<body>
    <div class="container">
        <div class="title">
            <p>Control Panel</p>
        </div>
        <div class="err_m" id="ID_ERR_BG">
            <p id="ID_ERR_MSG"></p>
        </div>
        <div class="content">
            <div class="card">
                <p class="card-title">STA Credentials</p>
                <div class="card-row">
                    <input type="text" class="card-textbox" id="ID_STA_SSID" placeholder="Enter SSID">
                    <input type="password" class="card-textbox" id="ID_STA_PASS" placeholder="Enter Password">
                </div>
                <div class="card-row">
                    <button id="ID_STA_SUBMIT" class="btn">Submit</button>
                </div>
            </div>
            <div class="card">
                <p class="card-title">AP Credentials</p>
                <div class="card-row">
                    <input type="text" class="card-textbox" id="ID_AP_SSID" placeholder="Enter SSID">
                    <input type="password" class="card-textbox" id="ID_AP_PASS" placeholder="Enter Password">
                </div>
                <div class="card-row">
                    <button id="ID_AP_SUBMIT" class="btn">Submit</button>
                </div>
            </div>
            <div class="card">
                <p class="card-title">Saved Networks</p>
                <div class="card-row" id="ID_SAVED_NW_WRAPPER">
                    
                </div>
            </div>
            <div class="card">
                <p class="card-title">Settings</p>
                <div class="card-row">
                    <button id="ID_AP_RESET" class="btn">Reset AP Credentials</button>
                    <button id="ID_ALL_RESET" class="btn">Factory Reset</button>
                </div>
            </div>
        </div>
        
    </div>
    <script src="script.js"></script>
</body>
</html>
)=====";