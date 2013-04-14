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
    
    PSP_SET_CONFIGURATION:     101,
    PSP_SET_EEPROM_REINIT:     102,
    PSP_SET_ACCEL_CALIBRATION: 103,
    PSP_SET_MAG_CALIBRATION:   104,
    PSP_SET_MOTOR_TEST_VALUE:  105,
    
    PSP_INF_ACK:      201,
    PSP_INF_REFUSED:  202,
    PSP_INF_CRC_FAIL: 203
};

var packet_state = 0;
var command_buffer = new Array();
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
                case 3: // payload length MSB
                    message_length_expected = data[i] << 8;
                    message_crc ^= data[i];
                    
                    packet_state++;
                    break;
                case 4: // payload length LSB
                    message_length_expected |= data[i];
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
        bufView[3] = highByte(data.length);
        bufView[4] = lowByte(data.length);
        
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
        bufView[3] = 0x00; // payload length MSB
        bufView[4] = 0x01; // payload length LSB
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
        case PSP.PSP_REQ_RC:
            process_data_receiver();
            break;
        case PSP.PSP_REQ_KINEMATICS:
            // 57.2957795 = rad to deg scale factor
            sensor_data.kinematics[0] = data.getFloat32(0, 1) * 57.2957795; // roll
            sensor_data.kinematics[1] = data.getFloat32(4, 1) * 57.2957795; // pitch
            sensor_data.kinematics[2] = data.getFloat32(8, 1) * 57.2957795; // yaw
            
            sensor_data.kinematics[1] *= -1.0; // Reverse Pitch
            break;
        case PSP.PSP_REQ_MOTORS_OUTPUT:
            if ($('#tabs > ul .active').hasClass('tab_motor_command')) {
                update_motor_command();
            } else { // standard behaviour
                process_motor_output();
            }
            break;
        case PSP.PSP_SET_ACCEL_CALIBRATION:
            process_accel_calibration();
            break;
        case PSP.PSP_REQ_MOTORS_COUNT:
            motors = parseInt(message_buffer_uint8_view[0]);
            break;
        case PSP.PSP_REQ_SENSORS_ALIVE:
            var sensors_detected = parseInt((message_buffer_uint8_view[0] << 8) | message_buffer_uint8_view[1]);
            sensor_status(sensors_detected);            
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
        default:
            console.log('Unknown command: ' + command);
    }
}