function tab_initialize_initial_setup() {
    // Fill in the manual calibration values from eeprom
    var i = 0;
    $('#content .calibrateAccelManual input').each(function() {
        $(this).val(eepromConfig.ACCEL_BIAS[i++]);
    });

    $('#content .calibrateESC').click(function() {
        eepromConfig.calibrateESC = parseInt(1);
        
        var eepromConfigBytes = new ArrayBuffer(eepromConfigSize);
        var view = new DataView(eepromConfigBytes, 0);
        view.setUNION(eepromConfig); 

        var bufferOut = new ArrayBuffer(5);
        var bufView = new Uint8Array(bufferOut);
        
        // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
        bufView[0] = 0xB5; // sync char 1
        bufView[1] = 0x62; // sync char 2
        bufView[2] = 0x02; // command
        bufView[3] = highByte(eepromConfigSize); // payload length MSB
        bufView[4] = lowByte(eepromConfigSize); // payload length LSB    
        
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
    
    $('#content .calibrateAccelManualUpdate').click(function() {
        // Update eeprom object
        var i = 0;
        $('.tab-initial_setup .calibrateAccelManual input').each(function() {
            eepromConfig.ACCEL_BIAS[i++] = parseInt($(this).val());
        });
        
        // Update flight controller with the latest data
        var eepromConfigBytes = new ArrayBuffer(eepromConfigSize);
        var view = new DataView(eepromConfigBytes, 0);
        view.setUNION(eepromConfig); 

        var bufferOut = new ArrayBuffer(5);
        var bufView = new Uint8Array(bufferOut);
        
        // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
        bufView[0] = 0xB5; // sync char 1
        bufView[1] = 0x62; // sync char 2
        bufView[2] = 0x02; // command
        bufView[3] = highByte(eepromConfigSize); // payload length MSB
        bufView[4] = lowByte(eepromConfigSize); // payload length LSB   
        
        chrome.serial.write(connectionId, bufferOut, function(writeInfo) {});
        
        // payload
        chrome.serial.write(connectionId, eepromConfigBytes, function(writeInfo) {
            if (writeInfo.bytesWritten > 0) {
                console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
                
                command_log('Sending Configuration UNION to Flight Controller ...');
            }    
        });
    });
    
    $('#content .initializeEEPROM').click(function() {
        // initialize EEPROM
        var bufferOut = new ArrayBuffer(6);
        var bufView = new Uint8Array(bufferOut);
        
        // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
        bufView[0] = 0xB5; // sync char 1
        bufView[1] = 0x62; // sync char 2
        bufView[2] = 0x09; // command
        bufView[3] = 0x00; // payload length MSB
        bufView[4] = 0x01; // payload length LSB
        bufView[5] = 0x01; // payload
        
        chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
            console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
            command_log('Requesting EEPROM re-initialization.');
        });  
    });
}

function process_accel_calibration() {
    var view = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
    
    var data = new Array(); // array used to hold/store read values
    
    data[0] = view.getInt16(0, 0);
    data[1] = view.getInt16(2, 0);
    data[2] = view.getInt16(4, 0);
    
    // Update the current eepromConfig object with latest data
    eepromConfig.ACCEL_BIAS[0] = data[0];
    eepromConfig.ACCEL_BIAS[1] = data[1];
    eepromConfig.ACCEL_BIAS[2] = data[2];
    
    console.log('Calibration received, data: ' + data);
    
    // Update the manual calibration inputs with latest data
    var i = 0;
    $('#content .calibrateAccelManual input').each(function() {
        $(this).val(eepromConfig.ACCEL_BIAS[i++]);
    });    
}