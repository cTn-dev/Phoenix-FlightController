var yaw_fix = 0.0;

function tab_initialize_vehicle_view() {
    // reset yaw button hook
    $('div#interactive_block > a.reset').click(function() {
        yaw_fix = sensor_data.kinematics[2];
        console.log("YAW reset to 0");
    });    
    
    // request kinematics data from flight controller
    timers.push(setInterval(kinematics_pull, 50));
}

function kinematics_pull() {
    // update cube with latest data
    var cube = $('div#cube');
    
    cube.css('-webkit-transform', 'rotateY(' + (sensor_data.kinematics[2] - yaw_fix) + 'deg)');
    $('#cubePITCH', cube).css('-webkit-transform', 'rotateX(' + sensor_data.kinematics[1] + 'deg)');
    $('#cubeROLL', cube).css('-webkit-transform', 'rotateZ(' + sensor_data.kinematics[0] + 'deg)');   

    // request new data
    send_message(PSP.PSP_REQ_KINEMATICS, 1);
}