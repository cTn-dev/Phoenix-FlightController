var motor_output_gui_initialized = 0;

function tab_initialize_motor_output() {
    motor_output_gui_initialized = 0; // reset
    
    // request motor out data from flight controller
    var bufferOut = new ArrayBuffer(6);
    var bufView = new Uint8Array(bufferOut);

    // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
    bufView[0] = 0xB5; // sync char 1
    bufView[1] = 0x62; // sync char 2
    bufView[2] = 0x06; // command
    bufView[3] = 0x00; // payload length MSB
    bufView[4] = 0x01; // payload length LSB
    bufView[5] = 0x01; // payload
    
    chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
        console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
        command_log('Requesting Motor Output Data from Flight Controller');
    });  
}

function process_motor_output() {
    var view = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
    
    var data = new Array(); // array used to hold/store read values

    var needle = 0;
    for (var i = 0; i < (message_buffer_uint8_view.length / 2); i++) {
        data[i] = view.getInt16(needle, 0);
        needle += 2;
    }
    
    // initialize gui according to motor count
    if (motor_output_gui_initialized == 0) {
        for (var i = 0; i < data.length; i++) {
            $('div.tab-motor_output .titles li:eq(' + i + ')').addClass('active');
        }
        
        motor_output_gui_initialized = 1;
    }
    
    // Render data
    for (var i = 0; i < data.length; i++) {
        data[i] -= 1000; 
        var margin_top = 330.0 - (data[i] * 0.33);
        var height = (data[i] * 0.33);
        var color = parseInt(data[i] * 0.256);
        $('.motor-' + i + ' .indicator').css({'margin-top' : margin_top + 'px', 'height' : height + 'px', 'background-color' : 'rgb(' + color + ',0,0)'});
    }
    
}