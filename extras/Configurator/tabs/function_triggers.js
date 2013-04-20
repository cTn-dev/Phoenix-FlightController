var AUX_triggered_mask = 0;

function tab_initialize_function_triggers() {    
    // AUX names manually defined
    var AUX_names = [
        'Stable mode (gyro + acc)',
        'Altitude hold (barometer)',
        'Altitude hold (sonar)',
        'GPS (not implemented - yet)'
    ];
    
    // generate table from the supplied AUX names and AUX data
    for (var i = 0; i < eepromConfig.CHANNEL_FUNCTIONS.length; i++) {
        $('.tab-function_triggers .functions > tbody:last').append(
            '<tr>' +
                '<td class="name">' + AUX_names[i] + '</td>' +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 0) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 1) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 2) +
                
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 3) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 4) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 5) +

                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 6) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 7) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 8) +

                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 9) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 10) +
                box_check(eepromConfig.CHANNEL_FUNCTIONS[i], 11) +            
            '</tr>'
        );
    }
    
    $('.tab-function_triggers a.update').click(function() {
        // catch the input changes
        var main_needle = 0;
        var needle = 0;
        $('.tab-function_triggers .functions input').each(function() {
            if ($(this).is(':checked')) {
                eepromConfig.CHANNEL_FUNCTIONS[main_needle] = bit_set(eepromConfig.CHANNEL_FUNCTIONS[main_needle], needle);
            } else {
                eepromConfig.CHANNEL_FUNCTIONS[main_needle] = bit_clear(eepromConfig.CHANNEL_FUNCTIONS[main_needle], needle);
            }
            
            needle++;
            
            if (needle >= 12) { // 4 aux * 3 checkboxes = 12 bits per line
                main_needle++;
                
                needle = 0;
            }
        });

        // Send updated UNION to the flight controller
        sendUNION();   
    });    
    
    // request AUX mask from flight controller
    timers.push(setInterval(AUX_pull, 250));    
}

function AUX_pull() {
    // Update the classes
    var needle = 0;
    $('.tab-function_triggers .functions input').each(function() {
        if (bit_check(AUX_triggered_mask, needle)) { // 1
            $(this).parent().addClass('on');
        } else { // 0
            $(this).parent().removeClass('on');
        }
        
        needle++;
        
        if (needle >= 12) { // 4 aux * 3 checkboxes = 12 bits per line
            needle = 0;
        }
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