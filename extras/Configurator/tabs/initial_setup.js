function tab_initialize_initial_setup() {
    $('#content .calibrateESC').click(function() {
        eepromConfig.calibrateESC = parseInt(1);
        
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
        bufView[3] = 0x01; // payload length MSB (0x108)
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