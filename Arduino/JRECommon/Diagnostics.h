
#ifndef __JRECommonDiagnostics_H__

    #define __JRECommonDiagnostics_H__

    #if (ARDUINO >= 100)
      #include <Arduino.h>
    #else
      #include <WProgram.h>
      #include <pins_arduino.h>
    #endif
    #include "Print.h"

    class Diagnostics
    {
      private:
        HardwareSerial& _serial;
        char _debugBuffer[128];
        
      public:
        Diagnostics(HardwareSerial& serial = Serial, int baud = 115200) : _serial(serial)
        {
            _serial.begin(baud);
        }
        
        void DeviceDump()
        {
            #if defined(ESP8266)
                DebugPrintLine("Reset reason:    %s", ESP.getResetReason().c_str());
                DebugPrintLine("ESP Chip ID:     %08X", ESP.getChipId());

                uint32_t realSize = ESP.getFlashChipRealSize();
                DebugPrintLine("Flash real id:   %08X", ESP.getFlashChipId());
                DebugPrintLine("Flash real size: %u", realSize);

                uint32_t ideSize = ESP.getFlashChipSize();
                FlashMode_t ideMode = ESP.getFlashChipMode();
                DebugPrintLine("Flash ide size:  %u", ideSize);
                DebugPrintLine("Flash ide speed: %u", ESP.getFlashChipSpeed());
                DebugPrintLine("Flash ide mode:  %s", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
            #endif
            
            DebugPrintLine("");
        }
      
        void DebugPrint(const char *format, ...)
        {
            va_list args;
            va_start(args, format);
            vsnprintf(_debugBuffer, 128, format, args);
            va_end(args);
            
            _serial.print(_debugBuffer);
        }

        void DebugPrintLine(const char *format, ...)
        {
            va_list args;
            va_start(args, format);
            vsnprintf(_debugBuffer, 128, format, args);
            va_end(args);
            
            _serial.println(_debugBuffer);
        }
    };

#endif