function tab_initialize_pid_tuning() {
    var i = 0;
    
    // Command
    $('#content .command-yaw input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_YAW_c[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_YAW_c[i++].toFixed(0));
        }        
    });
    
    i = 0;
    $('#content .command-pitch input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_PITCH_c[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_PITCH_c[i++].toFixed(0));
        }
    });
    
    i = 0;
    $('#content .command-roll input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_ROLL_c[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_ROLL_c[i++].toFixed(0));
        }
    });

    // Motor
    i = 0;
    $('#content .motor-yaw input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_YAW_m[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_YAW_m[i++].toFixed(0));
        }
    });
    
    i = 0;
    $('#content .motor-pitch input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_PITCH_m[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_PITCH_m[i++].toFixed(0));
        }
    });
    
    i = 0;
    $('#content .motor-roll input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_ROLL_m[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_ROLL_m[i++].toFixed(0));
        }        
    });

    // Baro
    i = 0;
    $('#content .baro input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_BARO[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_BARO[i++].toFixed(0));
        }
    });
    
    // Sonar
    i = 0;
    $('#content .sonar input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_SONAR[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_SONAR[i++].toFixed(0));
        }
    }); 

    // GPS
    i = 0;
    $('#content .GPS input').each(function() {
        if (i != 3) {
            $(this).val(eepromConfig.PID_GPS[i++].toFixed(2));
        } else {
            $(this).val(eepromConfig.PID_GPS[i++].toFixed(0));
        }
    });     
    
    // UI hooks
    $('#content .pid_tuning a.update').click(function() {
        var parent = $(this).parent().parent();
        
        var i = 0;
        switch (parent.index()) {
            case 1: // command yaw
                $('input', parent).each(function() {
                    eepromConfig.PID_YAW_c[i] = parseFloat($(this).val());
                    i++;
                });            
            break;
            case 2: // command pitch
                $('input', parent).each(function() {
                    eepromConfig.PID_PITCH_c[i] = parseFloat($(this).val());
                    i++;
                });  
            break;
            case 3: // command roll
                $('input', parent).each(function() {
                    eepromConfig.PID_ROLL_c[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
            case 5: // yaw
                $('input', parent).each(function() {
                    eepromConfig.PID_YAW_m[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
            case 6: // pitch
                $('input', parent).each(function() {
                    eepromConfig.PID_PITCH_m[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
            case 7: // roll
                $('input', parent).each(function() {
                    eepromConfig.PID_ROLL_m[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
            case 9: // baro
                $('input', parent).each(function() {
                    eepromConfig.PID_BARO[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
            case 10: // sonar
                $('input', parent).each(function() {
                    eepromConfig.PID_SONAR[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
            case 11: // GPS
                $('input', parent).each(function() {
                    eepromConfig.PID_GPS[i] = parseFloat($(this).val());
                    i++;
                });              
            break;
        }
        
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
};