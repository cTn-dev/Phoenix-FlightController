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
}