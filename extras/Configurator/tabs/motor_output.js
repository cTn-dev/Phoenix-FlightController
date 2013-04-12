function tab_initialize_motor_output() {
    for (var i = 0; i < motors; i++) {
        $('div.tab-motor_output .titles li:eq(' + i + ')').addClass('active');
    }
    
    // request motor out data from flight controller
    timers.push(setInterval(motor_pull, 50));
}

function motor_pull() {
    send_message(PSP.PSP_REQ_MOTORS_OUTPUT, 1);
}

function process_motor_output() {
    var view = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
    
    var data = new Array(); // array used to hold/store read values

    var needle = 0;
    for (var i = 0; i < (message_buffer_uint8_view.length / 2); i++) {
        data[i] = view.getInt16(needle, 0);
        needle += 2;
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