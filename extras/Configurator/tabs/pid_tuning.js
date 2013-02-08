function tab_initialize_pid_tuning() {
    var i = 0;
    
    // Command
    $('#content .command-yaw input').each(function() {
        $(this).val(eepromConfig.PID_YAW_c[i++]);
    });
    
    i = 0;
    $('#content .command-pitch input').each(function() {
        $(this).val(eepromConfig.PID_PITCH_c[i++]);
    });
    
    i = 0;
    $('#content .command-roll input').each(function() {
        $(this).val(eepromConfig.PID_ROLL_c[i++]);
    });

    // Motor
    i = 0;
    $('#content .motor-yaw input').each(function() {
        $(this).val(eepromConfig.PID_YAW_m[i++]);
    });
    
    i = 0;
    $('#content .motor-pitch input').each(function() {
        $(this).val(eepromConfig.PID_PITCH_m[i++]);
    });
    
    i = 0;
    $('#content .motor-roll input').each(function() {
        $(this).val(eepromConfig.PID_ROLL_m[i++]);
    });

    // Baro
    i = 0;
    $('#content .baro input').each(function() {
        $(this).val(eepromConfig.PID_BARO[i++]);
    });
    
    // Sonar
    i = 0;
    $('#content .sonar input').each(function() {
        $(this).val(eepromConfig.PID_SONAR[i++]);
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
        }
        
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
        bufView[3] = 0x01; // payload length MSB (0x108 = 264)
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
};