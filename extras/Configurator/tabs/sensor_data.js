var samples_i;

var e_graph_gyro;
var gyro_options;
var gyro_data = new Array(3);

var e_graph_accel;
var accel_options;
var accel_data = new Array(3);

var e_graph_mag;
var mag_options;
var mag_data = new Array(3);

var e_graph_baro;
var baro_options;
var baro_data = new Array(1);

function tab_initialize_sensor_data() {
    // Setup variables
    samples_i = 300;
    
    gyro_data[0] = new Array();
    gyro_data[1] = new Array();
    gyro_data[2] = new Array();
    
    accel_data[0] = new Array();
    accel_data[1] = new Array();
    accel_data[2] = new Array();

    mag_data[0] = new Array();
    mag_data[1] = new Array();
    mag_data[2] = new Array(); 

    baro_data[0] = new Array();
    
    for (var i = 0; i <= 300; i++) {
        gyro_data[0].push([i, 0]);
        gyro_data[1].push([i, 0]);
        gyro_data[2].push([i, 0]);
        
        accel_data[0].push([i, 0]);
        accel_data[1].push([i, 0]);
        accel_data[2].push([i, 0]);
        
        mag_data[0].push([i, 0]);
        mag_data[1].push([i, 0]);
        mag_data[2].push([i, 0]); 

        baro_data[0].push([i, 0]);
    } 

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
            //noTicks = 0
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
            //noTicks = 0
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
            //noTicks = 0
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
        send_message(PSP.PSP_REQ_GYRO_ACC, 1);
    }
    
    if (sensors_alive.mag == 1) {
        send_message(PSP.PSP_REQ_MAG, 1);
    }
    
    if (sensors_alive.baro == 1) {
        send_message(PSP.PSP_REQ_BARO, 1);
    }
}

function process_data_gyro_acc(view) {
    if ($('#tabs > ul .active').hasClass('tab_sensor_data')) { // used to protect against flotr object loss while switching to another tab
        var data = new Array(); // array used to hold/store read values
        
        // Gyro
        data[0] = view.getFloat32(0, 1); // X
        data[1] = view.getFloat32(4, 1); // Y
        data[2] = view.getFloat32(8, 1); // Z

        // Accel
        data[3] = view.getFloat32(12, 1); // X
        data[4] = view.getFloat32(16, 1); // Y
        data[5] = view.getFloat32(20, 1); // Z  

        // push latest data to the main array
        gyro_data[0].push([samples_i, data[0]]);
        gyro_data[1].push([samples_i, data[1]]);
        gyro_data[2].push([samples_i, data[2]]);
        
        accel_data[0].push([samples_i, data[3]]);
        accel_data[1].push([samples_i, data[4]]);
        accel_data[2].push([samples_i, data[5]]); 

        // Remove old data from array
        while (gyro_data[0].length > 300) {
            gyro_data[0].shift();
            gyro_data[1].shift();
            gyro_data[2].shift();
            
            accel_data[0].shift();
            accel_data[1].shift();
            accel_data[2].shift();        
        };            
        
        // Update graphs
        Flotr.draw(e_graph_gyro, [ 
            {data: gyro_data[0], label: "X - rate [" + data[0].toFixed(2) + "]"}, 
            {data: gyro_data[1], label: "Y - rate [" + data[1].toFixed(2) + "]"}, 
            {data: gyro_data[2], label: "Z - rate [" + data[2].toFixed(2) + "]"} ], gyro_options);  

        Flotr.draw(e_graph_accel, [ 
            {data: accel_data[1], label: "X - acceleration [" + data[3].toFixed(2) + "]"}, 
            {data: accel_data[0], label: "Y - acceleration [" + data[4].toFixed(2) + "]"}, 
            {data: accel_data[2], label: "Z - acceleration [" + data[5].toFixed(2) + "]"} ], accel_options); 

        samples_i++;
    }
}

function process_data_mag(view) {
    if ($('#tabs > ul .active').hasClass('tab_sensor_data')) { // used to protect against flotr object loss while switching to another tab
        var data = new Array(); // array used to hold/store read values
        
        data[0] = view.getInt16(0, 0);
        data[1] = view.getInt16(2, 0);
        data[2] = view.getInt16(4, 0);

        // push latest data to the main array
        mag_data[0].push([samples_i, data[0]]);
        mag_data[1].push([samples_i, data[1]]);
        mag_data[2].push([samples_i, data[2]]);        
        
        // Remove old data from array
        while (mag_data[0].length > 300) {            
            mag_data[0].shift();
            mag_data[1].shift();
            mag_data[2].shift();        
        }
        
        // Update graph
        Flotr.draw(e_graph_mag, [ 
            {data: mag_data[0], label: "X - Ga [" + data[0].toFixed(2) + "]"}, 
            {data: mag_data[1], label: "Y - Ga [" + data[1].toFixed(2) + "]"}, 
            {data: mag_data[2], label: "Z - Ga [" + data[2].toFixed(2) + "]"} ], mag_options);  
    }
}

function process_data_baro(view) {
    if ($('#tabs > ul .active').hasClass('tab_sensor_data')) { // used to protect against flotr object loss while switching to another tab
        var data = new Array(); // array used to hold/store read values
        
        data[0] = view.getFloat32(4, 1);
        
        // push latest data to the main array
        baro_data[0].push([samples_i, data[0]]);

        // Remove old data from array
        while (baro_data[0].length > 300) { 
            baro_data[0].shift();
        }
        
        // Update graph
        Flotr.draw(e_graph_baro, [ 
            {data: baro_data[0], label: "X - Meters [" + data[0].toFixed(2) + "]"} ], baro_options);  
    }
}