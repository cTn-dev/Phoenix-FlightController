var samples_i;

var e_graph_receiver;
var receiver_options;
var receiver_data = new Array(8);

function tab_initialize_rx() {
    // Setup variables
    samples_i = 300;
    
    receiver_data[0] = new Array();
    receiver_data[1] = new Array();
    receiver_data[2] = new Array();
    receiver_data[3] = new Array();
    receiver_data[4] = new Array();
    receiver_data[5] = new Array();
    receiver_data[6] = new Array();
    receiver_data[7] = new Array();
    
    for (var i = 0; i <= 300; i++) {
        receiver_data[0].push([i, 0]);
        receiver_data[1].push([i, 0]);
        receiver_data[2].push([i, 0]);
        receiver_data[3].push([i, 0]);
        receiver_data[4].push([i, 0]); 
        receiver_data[5].push([i, 0]); 
        receiver_data[6].push([i, 0]); 
        receiver_data[7].push([i, 0]); 
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

    graph_receiver = Flotr.draw(e_graph_receiver, [ 
        {data: receiver_data[0], label: "Channel - 1"}, 
        {data: receiver_data[1], label: "Channel - 2"},
        {data: receiver_data[2], label: "Channel - 3"},
        {data: receiver_data[3], label: "Channel - 4"},
        {data: receiver_data[4], label: "Channel - 5"},
        {data: receiver_data[5], label: "Channel - 6"},
        {data: receiver_data[6], label: "Channel - 7"},
        {data: receiver_data[7], label: "Channel - 8"} ], receiver_options);                      
    
    // request receiver data from flight controller
    var bufferOut = new ArrayBuffer(6);
    var bufView = new Uint8Array(bufferOut);
    
    // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
    bufView[0] = 0xB5; // sync char 1
    bufView[1] = 0x62; // sync char 2
    bufView[2] = 0x04; // command
    bufView[3] = 0x00; // payload length MSB
    bufView[4] = 0x01; // payload length LSB
    bufView[5] = 0x01; // payload
    
    chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
        console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
        command_log('Requesting Receiver Data from Flight Controller');
    });    
};

function process_data_receiver() {
    if ($('#tabs > ul .active').index() == 3) { // used to protect against flotr object loss while switching to another tab
        var data = new Array();

        var data_counter = 0;
        for (var i = 0; i < message_buffer.length; i++) {
            if (i % 2 == 0) {
                data[data_counter] = (((message_buffer[i] << 8) | message_buffer[i + 1]) << 16) >> 16;
                data_counter++;
            }
        } 

        // Apply scale factors
        data[0] = data[0] / 16.0;
        data[1] = data[1] / 16.0;
        data[2] = data[2] / 16.0;
        data[3] = data[3] / 16.0;
        data[4] = data[4] / 16.0;
        data[5] = data[5] / 16.0;
        data[6] = data[6] / 16.0;
        data[7] = data[7] / 16.0;
        
        // push latest data to the main array
        receiver_data[0].push([samples_i, data[0]]);
        receiver_data[1].push([samples_i, data[1]]);
        receiver_data[2].push([samples_i, data[2]]);
        receiver_data[3].push([samples_i, data[3]]);
        receiver_data[4].push([samples_i, data[4]]);
        receiver_data[5].push([samples_i, data[5]]);
        receiver_data[6].push([samples_i, data[6]]);
        receiver_data[7].push([samples_i, data[7]]);                

        // Remove old data from array
        while (receiver_data[0].length > 300) {
            receiver_data[0].shift();
            receiver_data[1].shift();
            receiver_data[2].shift(); 
            receiver_data[3].shift(); 
            receiver_data[4].shift(); 
            receiver_data[5].shift(); 
            receiver_data[6].shift(); 
            receiver_data[7].shift();                     
        }; 

    graph_receiver = Flotr.draw(e_graph_receiver, [ 
        {data: receiver_data[0], label: "Channel - 1"}, 
        {data: receiver_data[1], label: "Channel - 2"},
        {data: receiver_data[2], label: "Channel - 3"},
        {data: receiver_data[3], label: "Channel - 4"},
        {data: receiver_data[4], label: "Channel - 5"},
        {data: receiver_data[5], label: "Channel - 6"},
        {data: receiver_data[6], label: "Channel - 7"},
        {data: receiver_data[7], label: "Channel - 8"} ], receiver_options); 

        samples_i++;
    }    
};