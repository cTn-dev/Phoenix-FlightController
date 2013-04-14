function tab_initialize_rx() {
    // Setup variables
    samples_i = 300;
    
    for (var i = 0; i < receiver_data.plot.length; i++) {
        receiver_data.plot[i] = [];
    }
    
    for (var i = 0; i <= 300; i++) { 
        receiver_data.plot[0].push([i, 0]);
        receiver_data.plot[1].push([i, 0]);
        receiver_data.plot[2].push([i, 0]);
        receiver_data.plot[3].push([i, 0]);
        receiver_data.plot[4].push([i, 0]); 
        receiver_data.plot[5].push([i, 0]); 
        receiver_data.plot[6].push([i, 0]); 
        receiver_data.plot[7].push([i, 0]);  
    }
    
    // Graph definitions
    e_graph_receiver = document.getElementById("graph_receiver");
    
    receiver_options = {
        shadowSize: 0,
        yaxis : {
            max: 2200,
            min: 800
        },
        xaxis : {
            //noTicks = 0
        },
        grid : {
            backgroundColor: "#FFFFFF"
        },
        legend : {
            backgroundOpacity: 0
        }        
    }

    // request receiver data from flight controller
    timers.push(setInterval(rx_poll, 50));
}

function RX_channel_name(num) {
    var name;
    
    switch (num) {
        case 0:
            name = 'ROLL';
        break;
        case 1:
            name = 'PITCH';
        break;
        case 2:
            name = 'THROTTLE';
        break;
        case 3:
            name = 'YAW/RUDDER';
        break;
        case 4:
            name = 'MODE (rate/attitude)';
        break;
        case 5:
            name = 'Altitude Hold (off/sonar/baro)';
        break;
        case 6:
            name = 'Position Hold (off/GPS)';
        break;
        case 7:
            name = 'Undefined';
        break;        
    }
    
    return name;
}

function rx_poll() {
    // push latest data to the main array
    receiver_data.plot[0].push([samples_i, receiver_data.raw[0]]);
    receiver_data.plot[1].push([samples_i, receiver_data.raw[1]]);
    receiver_data.plot[2].push([samples_i, receiver_data.raw[2]]);
    receiver_data.plot[3].push([samples_i, receiver_data.raw[3]]);
    receiver_data.plot[4].push([samples_i, receiver_data.raw[4]]);
    receiver_data.plot[5].push([samples_i, receiver_data.raw[5]]);
    receiver_data.plot[6].push([samples_i, receiver_data.raw[6]]);
    receiver_data.plot[7].push([samples_i, receiver_data.raw[7]]);                

    // Remove old data from array
    while (receiver_data.plot[0].length > 300) {
        receiver_data.plot[0].shift();
        receiver_data.plot[1].shift();
        receiver_data.plot[2].shift(); 
        receiver_data.plot[3].shift(); 
        receiver_data.plot[4].shift(); 
        receiver_data.plot[5].shift(); 
        receiver_data.plot[6].shift(); 
        receiver_data.plot[7].shift();                     
    }

    Flotr.draw(e_graph_receiver, [ 
        {data: receiver_data.plot[0], label: "CH-0 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[0]) + " [" + receiver_data.raw[0] + "]"}, 
        {data: receiver_data.plot[1], label: "CH-1 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[1]) + " [" + receiver_data.raw[1] + "]"},
        {data: receiver_data.plot[2], label: "CH-2 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[2]) + " [" + receiver_data.raw[2] + "]"},
        {data: receiver_data.plot[3], label: "CH-3 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[3]) + " [" + receiver_data.raw[3] + "]"},
        {data: receiver_data.plot[4], label: "CH-4 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[4]) + " [" + receiver_data.raw[4] + "]"},
        {data: receiver_data.plot[5], label: "CH-5 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[5]) + " [" + receiver_data.raw[5] + "]"},
        {data: receiver_data.plot[6], label: "CH-6 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[6]) + " [" + receiver_data.raw[6] + "]"},
        {data: receiver_data.plot[7], label: "CH-7 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[7]) + " [" + receiver_data.raw[7] + "]"} ], receiver_options); 

    samples_i++;

    // request new data
    send_message(PSP.PSP_REQ_RC, 1); 
}