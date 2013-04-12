function tab_initialize_motor_command() { 
    // Request current state of motors
    send_message(PSP.PSP_REQ_MOTORS_OUTPUT, 1);    
    
    // Enable specific titles
    for (var i = 0; i < motors; i++) {
        $('div.tab-motor_command .names li:eq(' + i + ')').addClass('active');
    }
    
    // Disable the rest
    $('ul.sliders input').each(function() {
        var index = $(this).parent().index();
        
        if (index >= motors) {
            $(this).attr('disabled', 'disabled');
        }
    });
    
    $('.tab-motor_command .stop').click(function() {
        // reset all to 0
        $('ul.sliders input').each(function() {
            if ($(this).attr('disabled') != 'disabled') { // Protects .change event firing on motors that are disabled
                $(this).val(0);
                
                // trigger change events
                $(this).change();
            }
        });
        
    });     
}

function update_motor_command() {
    var view = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
    
    var data = new Array(); // array used to hold/store read values

    var needle = 0;
    for (var i = 0; i < (message_buffer_uint8_view.length / 2); i++) {
        data[i] = parseInt((view.getInt16(needle, 0) - 1000) / 10);
        needle += 2;
    }
    
    for (var i = 0; i < motors; i++) {
        $('ul.sliders input').eq(i).val(data[i]);
    }

    // Change handler is "hooked up" after all the data is processed (to avoid double sending of the values)
    
    $('ul.sliders input').change(function() {
        var motor_n = parseInt($(this).parent().index()); // motor number
        var motor_v = parseInt($(this).val()); // motor value
        
        // Update UI
        $('ul.values li').eq(motor_n).html(motor_v + ' %');
        
        // Send data to flight controller
        send_message(PSP.PSP_SET_MOTOR_TEST_VALUE, [motor_n, motor_v]);
    });   
}