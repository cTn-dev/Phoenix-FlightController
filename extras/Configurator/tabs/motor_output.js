function tab_initialize_motor_output() {
    // Request current state of motors
    send_message(PSP.PSP_REQ_MOTORS_OUTPUT, 1); 
    
    // Enable specific titles
    for (var i = 0; i < motors; i++) {
        $('div.tab-motor_output .titles li:eq(' + i + ')').addClass('active');
    }

    // Disable the rest
    $('ul.sliders input').each(function() {
        var index = $(this).parent().index();
        
        if (index >= motors) {
            $(this).attr('disabled', 'disabled');
        }
    });
    
    $('.tab-motor_output .stop').click(function() {
        // reset all to 0
        $('ul.sliders input').each(function() {
            if ($(this).attr('disabled') != 'disabled') { // Protects .change event firing on motors that are disabled
                $(this).val(0);
                
                // trigger change events
                $(this).change();
            }
        });
        
    });    
    
    // Delayed initialize (due to current motor output requirements
    setTimeout(function() {
        for (var i = 0; i < motors; i++) {
            $('ul.sliders input').eq(i).val((motors_output[i] - 1000) / 10);
            $('ul.values li').eq(i).html(((motors_output[i] - 1000) / 10) + ' %');
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

        // request motor out data from flight controller
        timers.push(setInterval(motor_pull, 50));        
    }, 25);    
}

function motor_pull() {
    // Render data
    for (var i = 0; i < motors; i++) {
        motors_output[i] -= 1000; 
        var margin_top = 130.0 - (motors_output[i] * 0.13);
        var height = (motors_output[i] * 0.13);
        var color = parseInt(motors_output[i] * 0.256);
        $('.motor-' + i + ' .indicator').css({'margin-top' : margin_top + 'px', 'height' : height + 'px', 'background-color' : 'rgb(' + color + ',0,0)'});
    }

    // request new data
    send_message(PSP.PSP_REQ_MOTORS_OUTPUT, 1);
}
