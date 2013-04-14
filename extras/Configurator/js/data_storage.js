var sensors_alive = {
    gyro:  0,
    accel: 0,
    mag:   0,
    baro:  0,
    gps:   0
}

var sensor_data = {
    gyro:       [0, 0, 0],
    accel:      [0, 0, 0],
    mag:        [0, 0, 0],
    baro:       [0, 0],
    kinematics: [0, 0, 0],
    
    // plot data
    gyro_plot: [new Array(), new Array(), new Array()],
    accel_plot: [new Array(), new Array(), new Array()],
    mag_plot: [new Array(), new Array(), new Array()],
    baro_plot: new Array()
}

var receiver_data = {
    raw:    [0, 0, 0, 0, 0, 0, 0, 0],
    plot:   [new Array(), new Array(), new Array(), new Array(), new Array(), new Array(), new Array(), new Array()]
}

// populate the plot arrays
for (var i = 0; i <= 300; i++) { 
    sensor_data.gyro_plot[0].push([i, 0]);
    sensor_data.gyro_plot[1].push([i, 0]);
    sensor_data.gyro_plot[2].push([i, 0]);
    
    sensor_data.accel_plot[0].push([i, 0]);
    sensor_data.accel_plot[1].push([i, 0]);
    sensor_data.accel_plot[2].push([i, 0]);  

    sensor_data.mag_plot[0].push([i, 0]);
    sensor_data.mag_plot[1].push([i, 0]);
    sensor_data.mag_plot[2].push([i, 0]);   

    sensor_data.baro_plot.push([i, 0]);
    
    receiver_data.plot[0].push([i, 0]);
    receiver_data.plot[1].push([i, 0]);
    receiver_data.plot[2].push([i, 0]);
    receiver_data.plot[3].push([i, 0]);
    receiver_data.plot[4].push([i, 0]); 
    receiver_data.plot[5].push([i, 0]); 
    receiver_data.plot[6].push([i, 0]); 
    receiver_data.plot[7].push([i, 0]);     
} 

var motors_output = new Array();