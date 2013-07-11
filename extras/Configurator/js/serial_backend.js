var PSP = {
    PSP_SYNC1:              0xB5,
    PSP_SYNC2:              0x62,
    
    PSP_REQ_CONFIGURATION:  1,
    PSP_REQ_GYRO_ACC:       2,
    PSP_REQ_MAG:            3,
    PSP_REQ_BARO:           4,
    PSP_REQ_GPS:            5,
    PSP_REQ_RC:             6,
    PSP_REQ_KINEMATICS:     7,
    PSP_REQ_MOTORS_OUTPUT:  8,
    PSP_REQ_MOTORS_COUNT:   9, 
    PSP_REQ_SENSORS_ALIVE:  10,
    PSP_REQ_AUX_TRIGGERED:  11,
    
    PSP_SET_CONFIGURATION:     101,
    PSP_SET_EEPROM_REINIT:     102,
    PSP_SET_ACCEL_CALIBRATION: 103,
    PSP_SET_MAG_CALIBRATION:   104,
    PSP_SET_MOTOR_TEST_VALUE:  105,
    
    PSP_INF_ACK:       201,
    PSP_INF_REFUSED:   202,
    PSP_INF_CRC_FAIL:  203,
    PSP_INF_DATA_TOO_LONG: 204
};

var packet_state = 0;
var command;

var message_length_expected = 0;
var message_length_received = 0;
var message_buffer;
var message_buffer_uint8_view;
var message_crc = 0;
var char_counter = 0;

function onCharRead(readInfo) {
    if (readInfo && readInfo.bytesRead > 0 && readInfo.data) {
        var data = new Uint8Array(readInfo.data);
        
        for (var i = 0; i < data.length; i++) {
            switch (packet_state) {
                case 0:
                    if (data[i] == PSP.PSP_SYNC1) {               
                        packet_state++;
                    }
                    break;
                case 1:
                    if (data[i] == PSP.PSP_SYNC2) {             
                        packet_state++;
                    } else {
                        packet_state = 0; // Restart and try again
                    }                    
                    break;
                case 2: // command
                    command = data[i];
                    message_crc = data[i];
                    
                    packet_state++;
                    break;
                case 3: // payload length LSB
                    message_length_expected = data[i];
                    message_crc ^= data[i];
                    
                    packet_state++;
                    break;
                case 4: // payload length MSB
                    message_length_expected |= data[i] << 8;
                    message_crc ^= data[i];
                    
                    // setup arraybuffer
                    message_buffer = new ArrayBuffer(message_length_expected);
                    message_buffer_uint8_view = new Uint8Array(message_buffer);
                    
                    packet_state++;
                    break;
                case 5: // payload
                    message_buffer_uint8_view[message_length_received] = data[i];
                    message_crc ^= data[i];
                    message_length_received++;
                    
                    if (message_length_received >= message_length_expected) {
                        packet_state++;
                    }
                break;
                case 6:
                    if (message_crc == data[i]) {
                        // message received, process
                        process_data(command, message_buffer);
                    } else {
                        // crc failed
                        console.log('crc failed');
                    }   
                    
                    // Reset variables
                    message_length_received = 0;
                    
                    packet_state = 0;
                    break;
            }
            
            char_counter++;
        }
    }
}

function send_message(code, data, callback) {
    // always reserve 6 bytes for protocol overhead !
    if (typeof data === 'object') {
        var size = 6 + data.length;
        var checksum = 0;
        
        var bufferOut = new ArrayBuffer(size);
        var bufView = new Uint8Array(bufferOut); 

        bufView[0] = PSP.PSP_SYNC1;
        bufView[1] = PSP.PSP_SYNC2;
        bufView[2] = code;
        bufView[3] = lowByte(data.length);
        bufView[4] = highByte(data.length);
        
        checksum = bufView[2] ^ bufView[3] ^ bufView[4];
        
        for (var i = 0; i < data.length; i++) {
            bufView[i + 5] = data[i];
            checksum ^= bufView[i + 5];
        }        
        
        bufView[5 + data.length] = checksum;
    } else {
        var bufferOut = new ArrayBuffer(6 + 1);
        var bufView = new Uint8Array(bufferOut);
        
        bufView[0] = PSP.PSP_SYNC1;
        bufView[1] = PSP.PSP_SYNC2;
        bufView[2] = code;
        bufView[3] = 0x01; // payload length LSB
        bufView[4] = 0x00; // payload length MSB
        bufView[5] = data; // payload
        bufView[6] = bufView[2] ^ bufView[3] ^ bufView[4] ^ bufView[5]; // crc        
    }
    
    chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
        if (writeInfo.bytesWritten > 0) {
            if (typeof callback !== 'undefined') {
                callback();
            }
        }
        
        // for debugging purposes
        // console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
    });    
}

function process_data(command, message_buffer) {
    var data = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
    
    switch (command) {
        case PSP.PSP_REQ_CONFIGURATION:
            console.log('Expected UNION size: ' + message_length_expected + ', Received UNION size: ' + message_buffer_uint8_view.length);
            // Store UNION size for later usage
            eepromConfigSize = message_length_expected;
            
            var view = new DataView(message_buffer, 0);
            view.parseUNION(eepromConfig); 
            
            if (version != eepromConfig.version) {
                command_log('Configurator version doesn\'t match the Flight software version');
                command_log('Configurator version: <strong>' + version + '</strong> - Flight software version: <strong>' + eepromConfig.version + '</strong>');
                command_log('<span style="color: red">Please upgrade your flight software and configurator to the lastest version.</span>');
                
                // Disconnect
                $('div#port-picker a.connect').click();
                
                break;
            }
            
            $('#tabs li a:first').click();
            command_log('Configuration UNION received -- <span style="color: green">OK</span>');
            break;
        case PSP.PSP_REQ_GYRO_ACC:
            sensor_data.gyro[0] = data.getFloat32(0, 1); // X
            sensor_data.gyro[1] = data.getFloat32(4, 1); // Y
            sensor_data.gyro[2] = data.getFloat32(8, 1); // Z

            sensor_data.accel[0] = data.getFloat32(12, 1); // X
            sensor_data.accel[1] = data.getFloat32(16, 1); // Y
            sensor_data.accel[2] = data.getFloat32(20, 1); // Z
            break;
        case PSP.PSP_REQ_MAG:
            sensor_data.mag[0] = data.getInt16(0, 1); // X
            sensor_data.mag[1] = data.getInt16(2, 1); // Y
            sensor_data.mag[2] = data.getInt16(4, 1); // Z
            break;
        case PSP.PSP_REQ_BARO:
            sensor_data.baro[0] = data.getFloat32(0, 1); // baroRawAltitude
            sensor_data.baro[1] = data.getFloat32(4, 1); // baroAltitude
            break;
        case PSP.PSP_REQ_GPS:
            GPS_data.lat = data.getInt32(0, 1) / 10000000; // +- 90 deg  - degrees multiplied by 10000000
            GPS_data.lon = data.getInt32(4, 1) / 10000000; // +- 180 deg - degrees multiplied by 10000000
            GPS_data.course = data.getInt32(8, 1) / 1000; // 10E-5 to millidegrees
            GPS_data.speed = data.getUint32(12, 1) / 27.7777; // cm/s
            GPS_data.height = data.getInt32(16, 1) / 1000; // mm
            GPS_data.accuracy = data.getUint32(20, 1) / 1000; // mm
            GPS_data.fixtime = data.getUint32(24, 1);
            GPS_data.sentences = data.getUint32(28, 1);
            GPS_data.state = data.getUint8(32);
            GPS_data.sats = data.getUint8(33);
            break;
        case PSP.PSP_REQ_RC:
            receiver_data.raw[0] = data.getInt16(0, 1);
            receiver_data.raw[1] = data.getInt16(2, 1);
            receiver_data.raw[2] = data.getInt16(4, 1);
            receiver_data.raw[3] = data.getInt16(6, 1);
            receiver_data.raw[4] = data.getInt16(8, 1);
            receiver_data.raw[5] = data.getInt16(10, 1);
            receiver_data.raw[6] = data.getInt16(12, 1);
            receiver_data.raw[7] = data.getInt16(14, 1);
            receiver_data.raw[8] = data.getInt16(16, 1);
            receiver_data.raw[9] = data.getInt16(18, 1);
            receiver_data.raw[10] = data.getInt16(20, 1);
            receiver_data.raw[11] = data.getInt16(22, 1);
            receiver_data.raw[12] = data.getInt16(24, 1);
            receiver_data.raw[13] = data.getInt16(26, 1);
            receiver_data.raw[14] = data.getInt16(28, 1);
            receiver_data.raw[15] = data.getInt16(30, 1);
            break;
        case PSP.PSP_REQ_KINEMATICS:
            // 57.2957795 = rad to deg scale factor
            sensor_data.kinematics[0] = data.getFloat32(0, 1) * 57.2957795; // roll
            sensor_data.kinematics[1] = data.getFloat32(4, 1) * 57.2957795; // pitch
            sensor_data.kinematics[2] = data.getFloat32(8, 1) * 57.2957795; // yaw
            
            sensor_data.kinematics[1] *= -1.0; // Reverse Pitch
            break;
        case PSP.PSP_REQ_MOTORS_OUTPUT:
            var needle = 0;
            for (var i = 0; i < motors; i++) {
                motors_output[i] = data.getInt16(needle, 1);
                needle += 2;
            }
            break;
        case PSP.PSP_SET_ACCEL_CALIBRATION:
            eepromConfig.ACCEL_BIAS[0] = data.getInt16(0, 1); // x
            eepromConfig.ACCEL_BIAS[1] = data.getInt16(2, 1); // y
            eepromConfig.ACCEL_BIAS[2] = data.getInt16(4, 1); // z
            
            process_accel_calibration();
            break;
        case PSP.PSP_REQ_MOTORS_COUNT:
            motors = parseInt(message_buffer_uint8_view[0]);
            break;
        case PSP.PSP_REQ_SENSORS_ALIVE:
            var sensors_detected = parseInt((message_buffer_uint8_view[1] << 8) | message_buffer_uint8_view[0]);
            sensor_status(sensors_detected);            
            break;
        case PSP.PSP_REQ_AUX_TRIGGERED:
            AUX_triggered_mask[0] = message_buffer_uint8_view[0];
            AUX_triggered_mask[0] |= (message_buffer_uint8_view[1] << 8);
            AUX_triggered_mask[0] |= (message_buffer_uint8_view[2] << 16);
            AUX_triggered_mask[0] |= (message_buffer_uint8_view[3] << 24);
            
            AUX_triggered_mask[1] = message_buffer_uint8_view[4];
            AUX_triggered_mask[1] |= (message_buffer_uint8_view[5] << 8);
            AUX_triggered_mask[1] |= (message_buffer_uint8_view[6] << 16);
            AUX_triggered_mask[1] |= (message_buffer_uint8_view[7] << 24);
            break;
        case PSP.PSP_SET_MOTOR_TEST_VALUE:
            // acknowledged valid motor value
            break;
        case PSP.PSP_INF_ACK:
            command_log('Flight Controller responds with -- <span style="color: green">ACK</span>');
            break;
        case PSP.PSP_INF_REFUSED:
            command_log('Flight Controller responds with -- <span style="color: red">REFUSED</span>');  
            break;
        case PSP.PSP_INF_CRC_FAIL:
            console.log('crc check failed, code: ' + message_buffer_uint8_view[0] + ' crc value: ' + message_buffer_uint8_view[1]);
            break;
        case PSP.PSP_INF_DATA_TOO_LONG:
            console.log('Flight Controller - Message too long (shorter the message or increase buffer size) !!!');
            break;
        default:
            console.log('Unknown command: ' + command);
    }
}