function tab_initialize_motor_command() { 
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

    $('ul.sliders input').change(function() {
        var motor_n = parseInt($(this).parent().index()); // motor number
        var motor_v = parseInt($(this).val()); // motor value
        
        // Update UI
        $('ul.values li').eq(motor_n).html(motor_v + ' %');
        
        // Send data to flight controller
        send_message(PSP.PSP_SET_MOTOR_TEST_VALUE, [motor_n, motor_v]);
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