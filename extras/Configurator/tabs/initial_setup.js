function tab_initialize_initial_setup() {
    // Fill in the manual calibration values from eeprom
    var i = 0;
    $('#content .calibrateAccelManual input').each(function() {
        $(this).val(eepromConfig.ACCEL_BIAS[i++]);
    });
    
    $('#content .minimumThrottle input').val(eepromConfig.minimumArmedThrottle);

    $('#content .calibrateESC').click(function() {
        eepromConfig.calibrateESC = parseInt(1);
        
        // Send updated UNION to the flight controller
        sendUNION();
    });
    
    $('#content .calibrateAccel').click(function() {
        // Start accel calibration
        command_log('Starting Accel calibration...');
        send_message(PSP.PSP_SET_ACCEL_CALIBRATION, 1);  
    });
    
    $('#content .calibrateAccelManualUpdate').click(function() {
        // Update eeprom object
        var i = 0;
        $('.tab-initial_setup .calibrateAccelManual input').each(function() {
            eepromConfig.ACCEL_BIAS[i++] = parseInt($(this).val());
        });
        
        // Send updated UNION to the flight controller
        sendUNION();
    });
    
    $('#content .minimumThrottleUpdate').click(function() {
        // Update eeprom object
        eepromConfig.minimumArmedThrottle = parseInt($('#content .minimumThrottle input').val());
        
        // Send updated UNION to the flight controller
        sendUNION();
    });
    
    $('#content .initializeEEPROM').click(function() {
        // initialize EEPROM
        command_log('Requesting EEPROM re-initialization.');
        send_message(PSP.PSP_SET_EEPROM_REINIT, 1);
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
    
    command_log('Accel Calibration received, data: ' + data);
    
    // Update the manual calibration inputs with latest data
    var i = 0;
    $('#content .calibrateAccelManual input').each(function() {
        $(this).val(eepromConfig.ACCEL_BIAS[i++]);
    });

    $('#content .minimumThrottle input').val(eepromConfig.minimumArmedThrottle);
}