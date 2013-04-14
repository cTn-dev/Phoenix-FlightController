function tab_initialize_motor_output() {
    for (var i = 0; i < motors; i++) {
        $('div.tab-motor_output .titles li:eq(' + i + ')').addClass('active');
    }
    
    // request motor out data from flight controller
    timers.push(setInterval(motor_pull, 50));
}

function motor_pull() {
    // Render data
    for (var i = 0; i < motors; i++) {
        motors_output[i] -= 1000; 
        var margin_top = 330.0 - (motors_output[i] * 0.33);
        var height = (motors_output[i] * 0.33);
        var color = parseInt(motors_output[i] * 0.256);
        $('.motor-' + i + ' .indicator').css({'margin-top' : margin_top + 'px', 'height' : height + 'px', 'background-color' : 'rgb(' + color + ',0,0)'});
    }

    // request new data
    send_message(PSP.PSP_REQ_MOTORS_OUTPUT, 1);
}
