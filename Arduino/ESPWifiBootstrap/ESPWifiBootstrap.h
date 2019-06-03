
#ifndef __ESPWifiBootstrap_H__

    #define __ESPWifiBootstrap_H__

    #if (ARDUINO >= 100)
      #include <Arduino.h>
    #else
      #include <WProgram.h>
      #include <pins_arduino.h>
    #endif

    class ESPWifiBootstrap
    {
      private:
        String _ssid = "";
        String _password = "";
        String _filename = "/wifi.set";
        bool _ipAddressSetupSaved = false;
        bool _isConnected = false;
        
        bool LoadSettings();
        bool SaveSettings();
        
        void GetHandler();
        void PostHandler();
        
      public:
        bool Connect();
        void CreateAP();   
        void StartWebServer();
        String GetIpAddress();
        String GetMacAddress();
    };

#endif