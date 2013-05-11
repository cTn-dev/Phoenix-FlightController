function tab_initialize_function_triggers() {    
    // AUX names manually defined
    var AUX_names = [
        'Stable mode (gyro + acc)',
        'Altitude hold (barometer)',
        'Altitude hold (sonar)',
        'GPS (not implemented)'
    ];
    
    // generate table from the supplied AUX names and AUX data
    for (var i = 0; i < eepromConfig.CHANNEL_FUNCTIONS.length; i++) {
        $('.tab-function_triggers .functions > tbody:last').append(
            '<tr>' +
                '<td class="name">' + AUX_names[i] + '</td>' +
                // AUX 1
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 0) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 1) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 2) +
                
                // AUX 2
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 3) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 4) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 5) +

                // AUX 3
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 6) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 7) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 8) +

                // AUX 4
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 9) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 10) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 11) +
                
                // AUX 5
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 12) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 13) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 14) +
                
                // AUX 6
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 15) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 16) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 17) +

                // AUX 7
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 18) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 19) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 20) +

                // AUX 8
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 21) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 22) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 23) +  
                
                // AUX 9
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 24) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 25) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 26) +
                
                // AUX 10
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 27) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 28) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 29) +

                // AUX 11
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 30) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][0], 31) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][1], 32) +

                // AUX 12
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][1], 33) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][1], 34) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i][1], 35) +
            '</tr>'
        );
    }
    
    $('.tab-function_triggers a.update').click(function() {
        // catch the input changes
        var main_needle = 0;
        var needle = 0;
        $('.tab-function_triggers .functions input').each(function() {
            if ($(this).is(':checked')) {
                if (needle < 32) {
                    eepromConfig.CHANNEL_FUNCTIONS[main_needle][0] = bit_set(eepromConfig.CHANNEL_FUNCTIONS[main_needle][0], needle);
                } else if (needle >= 32 && needle < 64) {
                    eepromConfig.CHANNEL_FUNCTIONS[main_needle][1] = bit_set(eepromConfig.CHANNEL_FUNCTIONS[main_needle][1], needle - 32);
                }                
            } else {
                if (needle < 32) {
                    eepromConfig.CHANNEL_FUNCTIONS[main_needle][0] = bit_clear(eepromConfig.CHANNEL_FUNCTIONS[main_needle][0], needle);
                } else if (needle >= 32 && needle < 64) {
                    eepromConfig.CHANNEL_FUNCTIONS[main_needle][1] = bit_clear(eepromConfig.CHANNEL_FUNCTIONS[main_needle][1], needle - 32);
                }
            }
            
            needle++;
            
            if (needle >= 36) { // 12 aux * 3 checkboxes = 36 bits per line
                main_needle++;
                
                needle = 0;
            }
        });

        // Send updated UNION to the flight controller
        sendUNION();   
    });    
    
    // request AUX mask from flight controller
    timers.push(setInterval(AUX_pull, 50));    
}

function AUX_pull() {
    // Update the classes
    var needle = 0;
    $('.tab-function_triggers .functions th:not(.name)').each(function() {
        if (needle < 32) {
            if (bit_check(AUX_triggered_mask[0], needle)) { // 1
                $(this).addClass('on');
            } else { // 0
                $(this).removeClass('on');
            }
        } else if (needle >= 32 && needle < 64) {
            if (bit_check(AUX_triggered_mask[1], needle)) { // 1
                $(this).addClass('on');
            } else { // 0
                $(this).removeClass('on');
            }
        }
        
        needle++;
    });
    
    
    // request new data
    send_message(PSP.PSP_REQ_AUX_TRIGGERED, 1);
}

function box_check(num, pos) {
    if (bit_check(num, pos)) { // 1
        return '<td><input type="checkbox" checked="checked" /></td>';
    } else { // 0
        return '<td><input type="checkbox" /></td>';
    }
}