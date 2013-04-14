var samples_i;

var mag_data = new Array(3);
var baro_data = new Array(1);

function tab_initialize_sensor_data() {
    // Setup variables
    samples_i = 300;

    // Graph definitions
    e_graph_gyro = document.getElementById("graph_gyro");
    e_graph_accel = document.getElementById("graph_accel");
    e_graph_mag = document.getElementById("graph_mag"); 
    e_graph_baro = document.getElementById("graph_baro");     
    
    gyro_options = {
        title: "Gyroscope (deg/s)",
        shadowSize: 0,
        yaxis: {
            max: 10,
            min: -10
        },
        xaxis: {
            //noTicks = 0
        },
        grid: {
            backgroundColor: "#FFFFFF"
        },
        legend: {
            position: "wn",
            backgroundOpacity: 0
        }
    }
    
    accel_options = {
        title: "Accelerometer (g)",
        shadowSize: 0,
        yaxis: {
            max : 1.5,
            min : -1.5
        },
        xaxis: {
        },
        grid: {
            backgroundColor : "#FFFFFF"
        },
        legend: {
            position: "wn",
            backgroundOpacity: 0
        }
    }

    mag_options = {
        title: "Magnetometer (Ga)",
        shadowSize: 0,
        yaxis: {
        },
        xaxis: {
        },
        grid: {
            backgroundColor : "#FFFFFF"
        },
        legend: {
            position: "wn",
            backgroundOpacity: 0
        }
    } 

    baro_options = {
        title: "Barometer (meters above sea level)",
        shadowSize: 0,
        yaxis: {
            tickDecimals: 2,
        },
        xaxis: {
        },
        grid: {
            backgroundColor : "#FFFFFF"
        },
        legend: {
            position: "wn",
            backgroundOpacity: 0
        }
    }     
    
    // disable plots for sensors that wasn't detected
    if (sensors_alive.gyro == 0) {
        $(e_graph_gyro).hide();
    }
    
    if (sensors_alive.accel == 0) {
        $(e_graph_accel).hide();
    }

    if (sensors_alive.mag == 0) {
        $(e_graph_mag).hide();
    }    

    if (sensors_alive.baro == 0) {
        $(e_graph_baro).hide();
    }
    
    // request sensor data from flight controller
    timers.push(setInterval(sensor_pull, 50));
}

function sensor_pull() {
    if (sensors_alive.gyro == 1 || sensors_alive.accel == 1) {
        // push latest data to the main array
        sensor_data.gyro_plot[0].push([samples_i, sensor_data.gyro[0]]);
        sensor_data.gyro_plot[1].push([samples_i, sensor_data.gyro[1]]);
        sensor_data.gyro_plot[2].push([samples_i, sensor_data.gyro[2]]);
        
        sensor_data.accel_plot[0].push([samples_i, sensor_data.accel[0]]);
        sensor_data.accel_plot[1].push([samples_i, sensor_data.accel[1]]);
        sensor_data.accel_plot[2].push([samples_i, sensor_data.accel[2]]); 

        // Remove old data from array
        while (sensor_data.gyro_plot[0].length > 300) {
            sensor_data.gyro_plot[0].shift();
            sensor_data.gyro_plot[1].shift();
            sensor_data.gyro_plot[2].shift();
            
            sensor_data.accel_plot[0].shift();
            sensor_data.accel_plot[1].shift();
            sensor_data.accel_plot[2].shift();        
        };            
        
        // Update graphs
        Flotr.draw(e_graph_gyro, [ 
            {data: sensor_data.gyro_plot[0], label: "X - rate [" + sensor_data.gyro[0].toFixed(2) + "]"}, 
            {data: sensor_data.gyro_plot[1], label: "Y - rate [" + sensor_data.gyro[1].toFixed(2) + "]"}, 
            {data: sensor_data.gyro_plot[2], label: "Z - rate [" + sensor_data.gyro[2].toFixed(2) + "]"} ], gyro_options);  

        Flotr.draw(e_graph_accel, [ 
            {data: sensor_data.accel_plot[0], label: "X - acceleration [" + sensor_data.accel[0].toFixed(2) + "]"}, 
            {data: sensor_data.accel_plot[1], label: "Y - acceleration [" + sensor_data.accel[1].toFixed(2) + "]"}, 
            {data: sensor_data.accel_plot[2], label: "Z - acceleration [" + sensor_data.accel[2].toFixed(2) + "]"} ], accel_options); 
    
        // request new data
        send_message(PSP.PSP_REQ_GYRO_ACC, 1);
    }
    
    if (sensors_alive.mag == 1) {
        // push latest data to the main array
        sensor_data.mag_plot[0].push([samples_i, sensor_data.mag[0]]);
        sensor_data.mag_plot[1].push([samples_i, sensor_data.mag[1]]);
        sensor_data.mag_plot[2].push([samples_i, sensor_data.mag[2]]);        
        
        // Remove old data from array
        while (sensor_data.mag_plot[0].length > 300) {            
            sensor_data.mag_plot[0].shift();
            sensor_data.mag_plot[1].shift();
            sensor_data.mag_plot[2].shift();        
        }
        
        // Update graph
        Flotr.draw(e_graph_mag, [ 
            {data: sensor_data.mag_plot[0], label: "X - Ga [" + sensor_data.mag[0].toFixed(2) + "]"}, 
            {data: sensor_data.mag_plot[1], label: "Y - Ga [" + sensor_data.mag[1].toFixed(2) + "]"}, 
            {data: sensor_data.mag_plot[2], label: "Z - Ga [" + sensor_data.mag[2].toFixed(2) + "]"} ], mag_options);  

        // request new data
        send_message(PSP.PSP_REQ_MAG, 1);
    }
    
    if (sensors_alive.baro == 1) {
        // push latest data to the main array
        sensor_data.baro_plot.push([samples_i, sensor_data.baro[1]]);

        // Remove old data from array
        while (sensor_data.baro_plot.length > 300) { 
            sensor_data.baro_plot.shift();
        }
        
        // Update graph
        Flotr.draw(e_graph_baro, [ 
            {data: sensor_data.baro_plot, label: "X - Meters [" + sensor_data.baro[1].toFixed(2) + "]"} ], baro_options);  
    
        // request new data
        send_message(PSP.PSP_REQ_BARO, 1);
    }
    
    samples_i++;
}