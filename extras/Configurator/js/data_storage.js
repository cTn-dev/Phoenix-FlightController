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
    raw:  [0, 0, 0, 0, 0, 0, 0, 0],
    plot: [new Array(), new Array(), new Array(), new Array(), new Array(), new Array(), new Array(), new Array()]
}

var motors_output = [0, 0, 0, 0, 0, 0, 0, 0];