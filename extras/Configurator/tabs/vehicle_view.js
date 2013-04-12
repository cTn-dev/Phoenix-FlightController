var yaw_fix = 0.0;

function tab_initialize_vehicle_view() {
    // reset yaw button hook
    $('div#interactive_block > a.reset').click(function() {
        yaw_fix = data[2];
        console.log("YAW reset to 0");
    });    
    
    // request kinematics data from flight controller
    timers.push(setInterval(kinematics_pull, 50));
}

function kinematics_pull() {
    send_message(PSP.PSP_REQ_KINEMATICS, 1);
}

function process_vehicle_view() {
    if ($('#tabs > ul .active').hasClass('tab_vehicle_view')) { // used to protect against flotr object loss while switching to another tab        
        var view = new DataView(message_buffer, 0); // DataView (allowing is to view arrayBuffer as struct/union)
        
        data = new Array(); // array used to hold/store read values

        // 57.2957795 = rad to deg scale factor
        data[0] = view.getFloat32(0, 1) * 57.2957795;
        data[1] = view.getFloat32(4, 1) * 57.2957795;
        data[2] = view.getFloat32(8, 1) * 57.2957795;
        
        data[1] = -data[1]; // Reverse Pitch
        
        var cube = $('div#cube');
        
        cube.css('-webkit-transform', 'rotateY(' + (data[2] - yaw_fix) + 'deg)');
        $('#cubePITCH', cube).css('-webkit-transform', 'rotateX(' + data[1] + 'deg)');
        $('#cubeROLL', cube).css('-webkit-transform', 'rotateZ(' + data[0] + 'deg)');        
    }
}