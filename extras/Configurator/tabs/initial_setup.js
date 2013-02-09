function tab_initialize_initial_setup() {
    $('#content .calibrateESC').click(function() {
        eepromConfig.calibrateESC = parseInt(1);
        
        var eepromConfigBytes = new ArrayBuffer(264);
        var view = new jDataView(eepromConfigBytes, 0, undefined, true);
        
        var composer = new jComposer(view, eepromConfigDefinition);
        var eepromBuffer = view.buffer;
        composer.compose(['eepromConfigDefinition'], eepromConfig);

        var bufferOut = new ArrayBuffer(5);
        var bufView = new Uint8Array(bufferOut);
        
        // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
        bufView[0] = 0xB5; // sync char 1
        bufView[1] = 0x62; // sync char 2
        bufView[2] = 0x02; // command
        bufView[3] = 0x01; // payload length MSB (0x108)
        bufView[4] = 0x08; // payload length LSB   
        
        chrome.serial.write(connectionId, bufferOut, function(writeInfo) {});
        
        // payload
        chrome.serial.write(connectionId, eepromConfigBytes, function(writeInfo) {
            if (writeInfo.bytesWritten > 0) {
                console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
                
                command_log('Sending Configuration UNION to Flight Controller ...');
            }    
        });
    });
    
    $('#content .calibrateAccel').click(function() {
        // Start accel calibration
        var bufferOut = new ArrayBuffer(6);
        var bufView = new Uint8Array(bufferOut);
        
        // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
        bufView[0] = 0xB5; // sync char 1
        bufView[1] = 0x62; // sync char 2
        bufView[2] = 0x08; // command
        bufView[3] = 0x00; // payload length MSB
        bufView[4] = 0x01; // payload length LSB
        bufView[5] = 0x01; // payload
        
        chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
            console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
            command_log('Starting Accel calibration, when FC responds with ACK, calibration is done.');
        });   
    });
}

function process_accel_calibration() {
    var data = new Array();
    
    var data_counter = 0;
    for (var i = 0; i < message_buffer.length; i++) {
        if (i % 2 == 0) {
            data[data_counter] = (((message_buffer[i] << 8) | message_buffer[i + 1]) << 16) >> 16;
            data_counter++;
        }
    }
    
    // Update the current eepromConfig object with latest data
    eepromConfig.ACCEL_BIAS[0] = data[0];
    eepromConfig.ACCEL_BIAS[1] = data[1];
    eepromConfig.ACCEL_BIAS[2] = data[2];
    
    console.log('Calibration received, data: ' + data);
}