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
        receiver_data.plot[8].push([i, 0]);
        receiver_data.plot[9].push([i, 0]);
        receiver_data.plot[10].push([i, 0]);
        receiver_data.plot[11].push([i, 0]);
        receiver_data.plot[12].push([i, 0]); 
        receiver_data.plot[13].push([i, 0]); 
        receiver_data.plot[14].push([i, 0]); 
        receiver_data.plot[15].push([i, 0]);          
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
            name = 'AUX 1';
        break;
        case 5:
            name = 'AUX 2';
        break;
        case 6:
            name = 'AUX 3';
        break;
        case 7:
            name = 'AUX 4';
        break;    

        case 8:
            name = 'AUX 5';
        break; 
        case 9:
            name = 'AUX 6';
        break; 
        case 10:
            name = 'AUX 7';
        break; 
        case 11:
            name = 'AUX 8';
        break; 
        case 12:
            name = 'AUX 9';
        break; 
        case 13:
            name = 'AUX 10';
        break; 
        case 14:
            name = 'AUX 11';
        break;   
        case 15:
            name = 'AUX 12';
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
    receiver_data.plot[8].push([samples_i, receiver_data.raw[8]]);
    receiver_data.plot[9].push([samples_i, receiver_data.raw[9]]);
    receiver_data.plot[10].push([samples_i, receiver_data.raw[10]]);
    receiver_data.plot[11].push([samples_i, receiver_data.raw[11]]);
    receiver_data.plot[12].push([samples_i, receiver_data.raw[12]]);
    receiver_data.plot[13].push([samples_i, receiver_data.raw[13]]);
    receiver_data.plot[14].push([samples_i, receiver_data.raw[14]]);
    receiver_data.plot[15].push([samples_i, receiver_data.raw[15]]);       

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
        receiver_data.plot[8].shift();
        receiver_data.plot[9].shift();
        receiver_data.plot[10].shift(); 
        receiver_data.plot[11].shift(); 
        receiver_data.plot[12].shift(); 
        receiver_data.plot[13].shift(); 
        receiver_data.plot[14].shift(); 
        receiver_data.plot[15].shift();         
    }

    Flotr.draw(e_graph_receiver, [ 
        {data: receiver_data.plot[0], label: "CH-0 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[0]) + " [" + receiver_data.raw[0] + "]"}, 
        {data: receiver_data.plot[1], label: "CH-1 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[1]) + " [" + receiver_data.raw[1] + "]"},
        {data: receiver_data.plot[2], label: "CH-2 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[2]) + " [" + receiver_data.raw[2] + "]"},
        {data: receiver_data.plot[3], label: "CH-3 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[3]) + " [" + receiver_data.raw[3] + "]"},
        {data: receiver_data.plot[4], label: "CH-4 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[4]) + " [" + receiver_data.raw[4] + "]"},
        {data: receiver_data.plot[5], label: "CH-5 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[5]) + " [" + receiver_data.raw[5] + "]"},
        {data: receiver_data.plot[6], label: "CH-6 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[6]) + " [" + receiver_data.raw[6] + "]"},
        {data: receiver_data.plot[7], label: "CH-7 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[7]) + " [" + receiver_data.raw[7] + "]"}, 
        {data: receiver_data.plot[8], label: "CH-8 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[8]) + " [" + receiver_data.raw[8] + "]"}, 
        {data: receiver_data.plot[9], label: "CH-9 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[9]) + " [" + receiver_data.raw[9] + "]"},
        {data: receiver_data.plot[10], label: "CH-10 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[10]) + " [" + receiver_data.raw[10] + "]"},
        {data: receiver_data.plot[11], label: "CH-11 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[11]) + " [" + receiver_data.raw[11] + "]"},
        {data: receiver_data.plot[12], label: "CH-12 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[12]) + " [" + receiver_data.raw[12] + "]"},
        {data: receiver_data.plot[13], label: "CH-13 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[13]) + " [" + receiver_data.raw[13] + "]"},
        {data: receiver_data.plot[14], label: "CH-14 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[14]) + " [" + receiver_data.raw[14] + "]"},
        {data: receiver_data.plot[15], label: "CH-15 - " + RX_channel_name(eepromConfig.CHANNEL_ASSIGNMENT[15]) + " [" + receiver_data.raw[15] + "]"} ], receiver_options); 

    samples_i++;

    // request new data
    send_message(PSP.PSP_REQ_RC, 1); 
}