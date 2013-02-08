var samples_i;

var e_graph_gyro;
var gyro_options;
var gyro_data = new Array(3);

var e_graph_accel;
var accel_options;
var accel_data = new Array(3);

function tab_initialize_sensor_data() {
    // Setup variables
    samples_i = 300;
    
    gyro_data[0] = new Array();
    gyro_data[1] = new Array();
    gyro_data[2] = new Array();
    
    accel_data[0] = new Array();
    accel_data[1] = new Array();
    accel_data[2] = new Array();    
    
    for (var i = 0; i <= 300; i++) {
        gyro_data[0].push([i, 0]);
        gyro_data[1].push([i, 0]);
        gyro_data[2].push([i, 0]);
        
        accel_data[0].push([i, 0]);
        accel_data[1].push([i, 0]);
        accel_data[2].push([i, 0]);     
    } 

    // Graph definitions
    e_graph_gyro = document.getElementById("graph_gyro");
    e_graph_accel = document.getElementById("graph_accel");  
    
    gyro_options = {
        title: "Gyroscope (deg/s)",
        shadowSize: 0,
        yaxis : {
            max: 10,
            min: -10
        },
        xaxis : {
            //noTicks = 0
        },
        grid : {
            backgroundColor: "#FFFFFF"
        },
        legend : {
            position: "wn",
            backgroundOpacity: 0
        }
    }
    
    accel_options = {
        title: "Accelerometer (+- 1g = 9.81 m/s^2)",
        shadowSize: 0,
        yaxis : {
            max : 1.5,
            min : -1.5
        },
        xaxis : {
            //noTicks = 0
        },
        grid : {
            backgroundColor : "#FFFFFF"
        },
        legend : {
            position: "wn",
            backgroundOpacity: 0
        }
    } 

    graph_gyro = Flotr.draw(e_graph_gyro, [ 
        {data: gyro_data[0], label: "X - rate"}, 
        {data: gyro_data[1], label: "Y - rate"}, 
        {data: gyro_data[2], label: "Z - rate"} ], gyro_options);  

    graph_accel = Flotr.draw(e_graph_accel, [ 
        {data: accel_data[1], label: "X - acceleration"}, 
        {data: accel_data[0], label: "Y - acceleration"}, 
        {data: accel_data[2], label: "Z - acceleration"} ], accel_options);                         
    
    // request sensor data from flight controller
    var bufferOut = new ArrayBuffer(6);
    var bufView = new Uint8Array(bufferOut);
    
    // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
    bufView[0] = 0xB5; // sync char 1
    bufView[1] = 0x62; // sync char 2
    bufView[2] = 0x03; // command
    bufView[3] = 0x00; // payload length MSB
    bufView[4] = 0x01; // payload length LSB
    bufView[5] = 0x01; // payload
    
    chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
        console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
        command_log('Requesting Sensor Data from Flight Controller');
    });                      
};

function process_data_sensors() {
    if ($('#tabs > ul .active').index() == 2) { // used to protect against flotr object loss while switching to another tab
        var data = new Array();
        
        var data_counter = 0;
        for (var i = 0; i < message_buffer.length; i++) {
            if (i % 2 == 0) {
                data[data_counter] = (((message_buffer[i] << 8) | message_buffer[i + 1]) << 16) >> 16;
                data_counter++;
            }
        }
        
        // Apply scale factors
        // Gyro
        data[0] = data[0] / 3276.0; // X
        data[1] = data[1] / 3276.0; // Y
        data[2] = data[2] / 3276.0; // Z

        // Accel
        data[3] = data[3] / 21845.0; // X
        data[4] = data[4] / 21845.0; // Y
        data[5] = data[5] / 21845.0; // Z  

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
        graph_gyro = Flotr.draw(e_graph_gyro, [ 
            {data: gyro_data[0], label: "X - rate"}, 
            {data: gyro_data[1], label: "Y - rate"}, 
            {data: gyro_data[2], label: "Z - rate"} ], gyro_options);  

        graph_accel = Flotr.draw(e_graph_accel, [ 
            {data: accel_data[1], label: "X - acceleration"}, 
            {data: accel_data[0], label: "Y - acceleration"}, 
            {data: accel_data[2], label: "Z - acceleration"} ], accel_options); 

        samples_i++;
    }
};