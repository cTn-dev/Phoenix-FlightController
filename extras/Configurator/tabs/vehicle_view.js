var yaw_fix = 0.0;

function tab_initialize_vehicle_view() {
    // reset yaw button hook
    $('div#interactive_block > a.reset').click(function() {
        yaw_fix = data[2];
        console.log("YAW reset to 0");
    });    
    
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
    if ($('#tabs > ul .active').hasClass('tab_vehicle_view')) { // used to protect against flotr object loss while switching to another tab        
        var view = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
        
        data = new Array(); // array used to hold/store read values

        data[0] = view.getFloat32(0, 1) * 57.29;
        data[1] = view.getFloat32(4, 1) * 57.29;
        data[2] = view.getFloat32(8, 1) * 57.29;
        
        data[1] = -data[1]; // Reverse Pitch
        
        var cube = $('div#cube');
        
        cube.css('-webkit-transform', 'rotateY(' + (data[2] - yaw_fix) + 'deg)');
        $('#cubePITCH', cube).css('-webkit-transform', 'rotateX(' + data[1] + 'deg)');
        $('#cubeROLL', cube).css('-webkit-transform', 'rotateZ(' + data[0] + 'deg)');        
    }
};