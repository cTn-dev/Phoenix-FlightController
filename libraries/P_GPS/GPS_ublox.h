/*  Ublox protocol (UBX) implementation in C++
    
    Currently i am running neo 6m module on Serial bus 2.
    readGPS function is called every 10ms (100Hz), baud rate should be adjusted accordingly.
    By "default" Serial2 uses 64 byte long RX buffer, using faster baud rate at the same "reading/sampling"
    rate will result in buffer overflow and data being lost/damaged.
    
    If you are using faster bus please adjust reading speed accordingly (or increase RX buffer size) to prevent
    buffer overflow.
    
    Big thanks to kha from #aeroquad for helping me get this up and running.
*/

struct gpsData {
    int32_t  lat,lon;  // position as degrees (*10E7)
    int32_t  course;   // degrees (*10E5)
    uint32_t speed;    // cm/s
    int32_t  height;   // mm (from ellipsoid)
    uint32_t accuracy; // mm
    uint32_t fixage;   // fix 
    uint32_t fixtime;  // fix 
    uint32_t sentences; // sentences/packets processed from gps (just statistics)
    uint8_t  state;    // gps state
    uint8_t  sats;     // number of satellites active
} gpsData;    

class UBLOX {
    public:
        // Constructor
        UBLOX() {
        };
        
        // reads "RAW" packet from serial buffer
        void read_packet() {
            while (Serial2.available()) {
                data = Serial2.read(); // store single byte from serial buffer into data variable
                
                switch (UBX_step) {
                    case 0:
                        if (data == 0xB5) { // UBX sync char 1
                            UBX_step++;
                        }
                    break;
                    case 1:
                        if (data == 0x62) { // UBX sync char 2
                            UBX_step++;
                        } else {
                            UBX_step = 0; // Restart and try again
                        }
                    break;
                    case 2: // CLASS
                        UBX_class = data; // byte containing UBX class
                        
                        UBX_CK_A = data; // initial value (also resets the value in new packet)
                        UBX_CK_B = data; // initial value (also resets the value in new packet)
                        
                        UBX_step++;
                    break;
                    case 3: // ID
                        UBX_id = data; // byte containing UBX id
                        
                        UBX_CK_A += data;
                        UBX_CK_B += UBX_CK_A;
                        
                        UBX_step++;
                    break;
                    case 4: // PAYLOAD LENGTH LOW byte
                        UBX_expected_length = data; // containing LOW byte of payload length
                        
                        UBX_CK_A += data;
                        UBX_CK_B += UBX_CK_A;                
                        
                        UBX_step++;
                    break;
                    case 5: // PAYLOAD LENGTH HIGH byte
                        UBX_expected_length += data << 8; // containing HIGH byte of payload length
                        
                        UBX_CK_A += data;
                        UBX_CK_B += UBX_CK_A;
                        
                        if (UBX_expected_length <= sizeof(ubloxMessage)) {
                            UBX_step++;
                        } else {
                            // discard overlong messages
                            UBX_step = 0;
                        }
                    break;
                    case 6: // PAYLOAD
                        UBX_CK_A += data;
                        UBX_CK_B += UBX_CK_A;
                        
                        ubloxMessage.raw[UBX_data_length] = data;
                        UBX_data_length++;
                        
                        if (UBX_data_length >= UBX_expected_length) {
                            UBX_data_length = 0;
                            UBX_step++;
                        }
                    break;
                    case 7: // FIRST CHECKSUM
                        if (UBX_CK_A != data) {
                            // checksum failed
                            UBX_step = 0;
                        } else {
                            UBX_step++;
                        }    
                    break;
                    case 8: // SECOND CHECKSUM
                        if (UBX_CK_B != data) {
                            // checksum failed
                        } else {
                            // checksum OK - process data
                            process_data();
                        }

                        UBX_step = 0; // jump back to start
                    break;
                }                
            }        
        };
        
        // use union to read the binary message
        void process_data() {
            if (UBX_class == 0x01) { // NAV
                if (UBX_id == 0x02) { // NAV:POSLLH
                    gpsData.lat = ubloxMessage.nav_posllh.lat;
                    gpsData.lon = ubloxMessage.nav_posllh.lon;
                    gpsData.height = ubloxMessage.nav_posllh.height;
                    gpsData.accuracy = ubloxMessage.nav_posllh.hAcc;
                    gpsData.fixtime = ubloxMessage.nav_posllh.iTow;
                } else if (UBX_id == 0x03) { // NAV:STATUS
                    switch (ubloxMessage.nav_status.gpsFix) {
                        case 2: // 2D FIX
                            gpsData.state = 2;
                        break;
                        case 3: // 3D FIX
                            gpsData.state = 3;
                        break;
                        default: // NO FIX
                            gpsData.state = 1;
                        break;
                    }
                } else if (UBX_id == 0x06) { // NAV:SOL
                    gpsData.sats = ubloxMessage.nav_sol.numSV;
                } else if (UBX_id == 0x12) { // NAV:VELNED
                    gpsData.course = ubloxMessage.nav_velned.heading / 100; // 10E-5 to millidegrees
                    gpsData.speed = ubloxMessage.nav_velned.gSpeed;
                } else {
                    // Used for debugging
                    // Serial.println(UBX_id, HEX);
                }
            }
        };
        
    private:
        // UBLOX binary message definitions (done by kha)
        struct ublox_NAV_STATUS { // 0x01 0x03 (16 bytes)
            uint32_t iTow;
            uint8_t  gpsFix;
            uint8_t  flags;
            uint8_t  fixStat;
            uint8_t  flags2;
            uint32_t ttfx;
            uint32_t msss;
        };

        struct ublox_NAV_POSLLH { // 0x01 0x02 (28 bytes)
            uint32_t iTow;
            int32_t lon; // 1e-7 degrees
            int32_t lat; // 1e-7 degrees
            int32_t height; // mm
            int32_t hMSL; // mm
            uint32_t hAcc; //mm
            uint32_t vAcc; //mm
        };

        struct ublox_NAV_SOL { // 0x01 0x6 (52 bytes)
            uint32_t iTow;
            int32_t  fTow;
            int16_t  week;
            uint8_t  gspFix;
            uint8_t  flags;
            int32_t  ecefX;
            int32_t  ecefY;
            int32_t  ecefZ;
            int32_t  pAcc;
            int32_t  ecefVX;
            int32_t  ecefVY;
            int32_t  ecefVZ;
            int32_t  sAcc;
            uint16_t pDOP;
            uint8_t  res1;
            uint8_t  numSV;
            uint32_t res2;
        };

        struct ublox_NAV_VELNED { // 0x01 0x12h (36 bytes)
            uint32_t iTow;
            int32_t  velN; // cm/s
            int32_t  velE; // cm/s
            int32_t  velD; // cm/s
            uint32_t  speed; // cm/s
            uint32_t  gSpeed; // cm/s
            int32_t  heading; // dev 1e-5
            uint32_t sAcc; // cm/s
            uint32_t cAcc; // deg 1e-5
        };

        union ublox_message {
            struct ublox_NAV_STATUS nav_status;
            struct ublox_NAV_POSLLH nav_posllh;
            struct ublox_NAV_VELNED nav_velned;
            struct ublox_NAV_SOL nav_sol;
            unsigned char raw[52];
        } ubloxMessage;  
        
        
        // Private variables used inside the class/loop
        char data; // variable used to store a single byte from serial
        
        uint8_t UBX_step = 0; // used to select a correct step (data packet is read by state machine)

        int8_t UBX_class;
        int8_t UBX_id;
        uint16_t UBX_expected_length;
        uint16_t UBX_data_length;
        uint8_t UBX_CK_A;
        uint8_t UBX_CK_B;        
};

UBLOX ublox;

void SensorArray::readGPS() {
    ublox.read_packet();
}