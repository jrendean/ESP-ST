

#include "..\JRECommon\Diagnostics.h"
#include "ESPWifiBootstrap.h"
#include "FS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>



ESP8266WebServer webServer(23969);
Diagnostics diagnostics;

bool ESPWifiBootstrap::Connect()
{
    diagnostics.DebugPrintLine("Connect");
    
    /*
    SPIFFS.begin();
    File f1 = SPIFFS.open(this->_filename, "w");
    if (!f1)
    {
        this->DebugPrintLine("File open failed");
        return false;
    }
    
    f1.println("JUPITER");
    f1.println("badcabdead");
    
    f1.close();
    SPIFFS.end();
    */
    
    
    
    /*
    SPIFFS.begin();
    SPIFFS.format();
    SPIFFS.end();
    */
    
    
    if (this->LoadSettings())
    {
        diagnostics.DebugPrintLine("Connecting to: '%s'", this->_ssid.c_str());

        WiFi.softAPdisconnect();
        WiFi.disconnect();
        WiFi.begin(this->_ssid.c_str(), this->_password.c_str());
        
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            diagnostics.DebugPrint(".");
            //diagnostics.DebugPrintLine("%d", WiFi.status());
        }
        
        diagnostics.DebugPrintLine("Connected, IP address: %s", WiFi.localIP().toString().c_str());
        
        this->_isConnected = true;
        
        return true;
    }
    
    return false;
}

void ESPWifiBootstrap::CreateAP()
{
    diagnostics.DebugPrintLine("CreateAP");
    
    char apName[16];
    sprintf(apName, "ESP8266-%08X", ESP.getChipId());
    
    diagnostics.DebugPrintLine("SoftAP Name:  %s", apName);
    diagnostics.DebugPrintLine("SoftAP State: %s", WiFi.softAP(apName) ? "Ready" : "Failed");
    diagnostics.DebugPrintLine("SoftAP IP:    %s", WiFi.softAPIP().toString().c_str());
    diagnostics.DebugPrintLine("SoftAP MAC:   %s", WiFi.softAPmacAddress().c_str());
        
    this->StartWebServer();
}

void ESPWifiBootstrap::StartWebServer()
{
    // TODO: dns name as well?
    //char apName[16];
    //sprintf(apName, "ESP8266-%08X", ESP.getChipId());
    
    //if (MDNS.begin(apName))
    if (MDNS.begin("ESP8266"))
    {
		diagnostics.DebugPrintLine("MDNS responder started");
	}
    
    webServer.on("/", HTTP_GET, std::bind(&ESPWifiBootstrap::GetHandler, this));
    webServer.on("/", HTTP_POST, std::bind(&ESPWifiBootstrap::PostHandler, this));
    webServer.begin();
    
    while (!this->_ipAddressSetupSaved)
    {
        webServer.handleClient();
        //delay(100);
    }
}

String ESPWifiBootstrap::GetIpAddress()
{
    return this->_isConnected ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}

String ESPWifiBootstrap::GetMacAddress()
{
    return this->_isConnected ? WiFi.macAddress() : WiFi.softAPmacAddress();
}



bool ESPWifiBootstrap::LoadSettings()
{
    diagnostics.DebugPrintLine("LoadSettings");
    
    SPIFFS.begin();
    if (!SPIFFS.exists(this->_filename))
    {
        diagnostics.DebugPrintLine("Cannot find file: %s", this->_filename.c_str());
        return false;
    }
    
    File settingsFile = SPIFFS.open(this->_filename, "r");
    if (!settingsFile)
    {
        diagnostics.DebugPrintLine("File open failed");
        return false;
    }

    int lineCounter = 0;

    while (settingsFile.available())
    {
        String temp = settingsFile.readStringUntil('\n');
        lineCounter++;
        switch (lineCounter)
        {
            case 1:
                // need to remove \n on the end
                this->_ssid = temp.substring(0, temp.length() - 1);
                break;
                
            case 2:
                // need to remove \n on the end
                this->_password = temp.substring(0, temp.length() - 1);
                break;
                
            default:
                diagnostics.DebugPrintLine("too many lines with line number %d", lineCounter);
                break;
        }
    }

    settingsFile.close();
    SPIFFS.end();
    
    return true;
}

bool ESPWifiBootstrap::SaveSettings()
{
    diagnostics.DebugPrintLine("SaveSettings");
    
    SPIFFS.begin();
    File settingsFile = SPIFFS.open(this->_filename, "w");
    
    if (!settingsFile)
    {
        diagnostics.DebugPrintLine("File open failed");
        return false;
    }
    
    settingsFile.println(this->_ssid);
    settingsFile.println(this->_password);
    
    settingsFile.close();
    SPIFFS.end();
    
    return true;
}

void ESPWifiBootstrap::GetHandler()
{
    diagnostics.DebugPrintLine("GetHandler");
    
    webServer.sendHeader("Connection", "close");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");

    if (this->_ssid == NULL && this->_password == NULL)
    {
        if (!this->LoadSettings())
        {
            diagnostics.DebugPrintLine("Cannot load settings for GetHandler");
        }
    }
    
    String index = "<html><body><form method='post'>";
    index.concat("<span>IP: ");
    index.concat(this->GetIpAddress());
    index.concat("</span><span>MAC: ");
    index.concat(this->GetMacAddress());
    index.concat("</span><br/>");
    index.concat("ssid: <input type='text' name='ssid' value='");
    index.concat(this->_ssid);
    index.concat("'/><br/>password: <input type='password' name='password' value='");
    index.concat(this->_password);
    index.concat("'/><br/><button>Save</button></form></body></html>");
    
    webServer.send(200, "text/html", index);
}

void ESPWifiBootstrap::PostHandler()
{
    diagnostics.DebugPrintLine("PostHandler");
    
    webServer.sendHeader("Connection", "close");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");

    this->_ssid = webServer.arg("ssid");
    this->_password = webServer.arg("password");
    
    diagnostics.DebugPrintLine("ssid: %s", this->_ssid.c_str());
    diagnostics.DebugPrintLine("password: %s", this->_password.c_str());
    
    this->_ipAddressSetupSaved = SaveSettings();

    webServer.send(200, "text/plain", this->_ipAddressSetupSaved ?  "Save successful" : "Save failed");

    delay(1000);
    
    if (this->_ipAddressSetupSaved)
    {
        WiFi.softAPdisconnect();
        ESP.restart();
    }
}

