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
    
    $('#content .backup').click(configuration_backup);
    
    $('#content .restore').click(configuration_restore);
}

function process_accel_calibration() {
    command_log('Accel Calibration data received: ' + eepromConfig.ACCEL_BIAS);
    
    // Update the manual calibration inputs with latest data
    var i = 0;
    $('#content .calibrateAccelManual input').each(function() {
        $(this).val(eepromConfig.ACCEL_BIAS[i++]);
    });

    $('#content .minimumThrottle input').val(eepromConfig.minimumArmedThrottle);
}