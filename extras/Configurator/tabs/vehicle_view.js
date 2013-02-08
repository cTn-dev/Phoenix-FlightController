function tab_initialize_vehicle_view() {
    /*
    // reset yaw button hook
    $('div#interactive_block > a.reset').click(function() {
        yaw_fix = data[3];
        console.log("YAW reset to 0");
    });    
    */
    
    // request kinematics data from flight controller
    var bufferOut = new ArrayBuffer(6);
    var bufView = new Uint8Array(bufferOut);
    
    // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
    bufView[0] = 0xB5; // sync char 1
    bufView[1] = 0x62; // sync char 2
    bufView[2] = 0x05; // command
    bufView[3] = 0x00; // payload length MSB
    bufView[4] = 0x01; // payload length LSB
    bufView[5] = 0x01; // payload
    
    chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
        console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
        command_log('Requesting Kinematics Data (vehicle state) from Flight Controller');
    }); 
};

function process_vehicle_view() {
    if ($('#tabs > ul .active').index() == 4) { // used to protect against flotr object loss while switching to another tab
        data = new Array();
        
        var data_counter = 0;
        for (var i = 0; i < message_buffer.length; i++) {
            if (i % 2 == 0) {
                data[data_counter] = (((message_buffer[i] << 8) | message_buffer[i + 1]) << 16) >> 16;
                data_counter++;
            }
        }

        // Apply scale factors
        // (rad_to_deg = 57.29)
        data[0] = (data[0] / 10435.0) * 57.29; // Roll
        data[1] = (data[1] / 10435.0) * 57.29; // Pitch
        data[2] = (data[2] / 10435.0) * 57.29; // Yaw  
        
        data[1] = -data[1]; // Reverse Pitch
        
        var cube = $('div#cube');
        
        //cube.css('-webkit-transform', 'rotateY(' + data[2] + 'deg)');
        $('#cubePITCH', cube).css('-webkit-transform', 'rotateX(' + data[1] + 'deg)');
        $('#cubeROLL', cube).css('-webkit-transform', 'rotateZ(' + data[0] + 'deg)');        
    }
};