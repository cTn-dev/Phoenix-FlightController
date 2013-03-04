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
        var motor_n = parseInt($(this).parent().index());
        var motor_v = parseInt($(this).val());
        
        // Update UI
        $('ul.values li').eq(motor_n).html(motor_v + ' %');
        
        // Send data to flight controller
        var bufferOut = new ArrayBuffer(7);
        var bufView = new Uint8Array(bufferOut);
        
        // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
        bufView[0] = 0xB5; // sync char 1
        bufView[1] = 0x62; // sync char 2
        bufView[2] = 0x0A; // command // 10
        bufView[3] = 0x00; // payload length MSB
        bufView[4] = 0x02; // payload length LSB
        bufView[5] = motor_n; // motor number
        bufView[6] = motor_v; // motor value
        
        chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
            console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
        });
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