function tab_initialize_motor_command() {
    $('ul.sliders input').change(function() {
        var motor_n = $(this).parent().index();
        var motor_v = $(this).val();
        
        // Update UI
        $('ul.values li').eq(motor_n).html(motor_v + ' %');
        
        // Send data to flight controller
        // TODO
    });
    
    $('.tab-motor_command .stop').click(function() {
        // reset all to 0
        $('ul.sliders input').each(function() {
            if ($(this).val() != 0) { // Protects .change event firing on motors that are already at 0 %
                $(this).val(0);
                
                // trigger change events
                $(this).change();
            }
        });
        
    });
}