function tab_initialize_channel_assigner() {
    var i = 0;
    
    $('#content table.channel_assigner input').each(function() {
        $(this).val(eepromConfig.CHANNEL_ASSIGNMENT[i++]);
    }); 
    
    // UI hooks
    $('#content table.channel_assigner input').focus(function() {
        $(this).data('prev_val', $(this).val());
        $(this).val(''); // empty input field
    });
    
    $('#content table.channel_assigner input').blur(function() {
        if ($(this).val() == '') { // if input field is empty restore previous value
            $(this).val($(this).data('prev_val'));
        }
    });    
    
    $('#content table.channel_assigner a.update').click(function() {
        var parent = $(this).parent().parent();
        
        // - 1 because .index() starts at 1 not 0
        eepromConfig.CHANNEL_ASSIGNMENT[parent.index() - 1] = parseInt($('input', parent).val());
        
        // Send updated UNION to the flight controller
        sendUNION();      
    });
}