var connectionId = -1;
var connection_delay = 0; // delay which defines "when" will the configurator request configurator data after connection was established

var version = 2; // configurator version to check against version number stored in eeprom
var eepromConfigSize;

var motors = 0;

$(document).ready(function() { 
    port_picker = $('div#port-picker .port select');
    baud_picker = $('div#port-picker #baud');
    delay_picker = $('div#port-picker #delay');
    
    $('div#port-picker a.refresh').click(function() {
        console.log("Available port list requested.");
        port_picker.html('');

        chrome.serial.getPorts(function(ports) {
            if (ports.length > 0) {
                // Port list received
                
                ports.forEach(function(port) {
                    $(port_picker).append($("<option/>", {
                        value: port,
                        text: port
                    }));        
                });
            } else {
                $(port_picker).append($("<option/>", {
                    value: 0,
                    text: 'NOT FOUND'
                }));
                
                console.log("No serial ports detected");
            }
        });
    });
    
    // software click to refresh port picker select (during initial load)
    $('div#port-picker a.refresh').click();
    
    $('div#port-picker a.connect').click(function() {
        var clicks = $(this).data('clicks');
        
        selected_port = String($(port_picker).val());
        selected_baud = parseInt(baud_picker.val());
        connection_delay = parseInt(delay_picker.val());
        
        if (selected_port != '0') {
            if (clicks) { // odd number of clicks
                stop_data_stream();
                chrome.serial.close(connectionId, onClosed);
                
                clearTimeout(connection_delay);
                clearInterval(serial_poll);
                clearInterval(port_usage_poll);
                
                // Reset port usage indicator to 0
                $('span.port-usage').html(0 + ' %');
                
                $(this).text('Connect');
                $(this).removeClass('active');            
            } else { // even number of clicks        
                console.log('Connecting to: ' + selected_port);
                
                chrome.serial.open(selected_port, {
                    bitrate: selected_baud
                }, onOpen);
                
                $(this).text('Disconnect');  
                $(this).addClass('active');
            }
            
            $(this).data("clicks", !clicks);
        }
    }); 

    // Tabs
    var tabs = $('#tabs > ul');
    $('a', tabs).click(function() {
        if ($(this).parent().hasClass('active') == false) { // only initialize when the tab isn't already active
            if (connectionId < 1) { // if there is no active connection, return
                command_log('You <span style="color: red;">can\'t</span> view the tabs unless you <span style="color: green">connect</span> to the flight controller.');
                return;
            }
            
            // disable previous active button
            $('li', tabs).removeClass('active');
            stop_data_stream();
            
            // Highlight selected button
            $(this).parent().addClass('active');
            
            if ($(this).parent().hasClass('tab_initial_setup')) {
                $('#content').load("./tabs/initial_setup.html", tab_initialize_initial_setup);
            } else if ($(this).parent().hasClass('tab_pid_tuning')) {
                $('#content').load("./tabs/pid_tuning.html", tab_initialize_pid_tuning);
            } else if ($(this).parent().hasClass('tab_channel_assigner')) {
                $('#content').load("./tabs/channel_assigner.html", tab_initialize_channel_assigner);
            } else if ($(this).parent().hasClass('tab_sensor_data')) {
                $('#content').load("./tabs/sensor_data.html", tab_initialize_sensor_data);
            } else if ($(this).parent().hasClass('tab_tx_rx')) {
                $('#content').load("./tabs/rx.html", tab_initialize_rx);
            } else if ($(this).parent().hasClass('tab_vehicle_view')) {
                $('#content').load("./tabs/vehicle_view.html", tab_initialize_vehicle_view);
            } else if ($(this).parent().hasClass('tab_motor_output')) {
                $('#content').load("./tabs/motor_output.html", tab_initialize_motor_output);
            } else if ($(this).parent().hasClass('tab_motor_command')) {
                $('#content').load("./tabs/motor_command.html", tab_initialize_motor_command);
            } else if ($(this).parent().hasClass('tab_about')) {
                $('#content').load("./tabs/about.html");
            }            
        }
    });
});

function command_log(message) {
    var d = new Date();
    var time = ((d.getHours() < 10) ? '0' + d.getHours(): d.getHours()) 
        + ':' + ((d.getMinutes() < 10) ? '0' + d.getMinutes(): d.getMinutes()) 
        + ':' + ((d.getSeconds() < 10) ? '0' + d.getSeconds(): d.getSeconds());
    
    $('div#command-log > div.wrapper').append('<p>' + time + ' -- ' + message + '</p>');
    $('div#command-log').scrollTop($('div#command-log div.wrapper').height());    
};

function onOpen(openInfo) {
    connectionId = openInfo.connectionId;
    
    if (connectionId != -1) {
        var selected_port = String($(port_picker).val());
        
        console.log('Connection was opened with ID: ' + connectionId);
        command_log('Connection to: ' + selected_port + ' was opened with ID: ' + connectionId);
        
        connection_delay = setTimeout(function() {
            // start polling
            serial_poll = setInterval(readPoll, 10);
            port_usage_poll = setInterval(port_usage, 1000);
            
            // request configuration data (so we have something to work with)
            requestUNION();
            
            // request number of motors used in this setup
            var bufferOut = new ArrayBuffer(6);
            var bufView = new Uint8Array(bufferOut);
            
            // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
            bufView[0] = 0xB5; // sync char 1
            bufView[1] = 0x62; // sync char 2
            bufView[2] = 0x0B; // command 11
            bufView[3] = 0x00; // payload length MSB
            bufView[4] = 0x01; // payload length LSB
            bufView[5] = 0x01; // payload

            chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
                console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
            });

            // requesting sensors detected
            var bufferOut = new ArrayBuffer(6);
            var bufView = new Uint8Array(bufferOut);
            
            // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
            bufView[0] = 0xB5; // sync char 1
            bufView[1] = 0x62; // sync char 2
            bufView[2] = 0x0C; // command // 12
            bufView[3] = 0x00; // payload length MSB
            bufView[4] = 0x01; // payload length LSB
            bufView[5] = 0x01; // payload

            chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
                console.log("Wrote: " + writeInfo.bytesWritten + " bytes");
            });
        }, connection_delay * 1000);            
        
    } else {
        $('div#port-picker a.connect').click(); // reset the connect button back to "disconnected" state
        console.log('There was a problem while opening the connection.');
        command_log('Could not join the serial bus -- <span style="color: red;">ERROR</span>');
    }    
};

function onClosed(result) {
    if (result) { // All went as expected
        console.log('Connection closed successfully.');
        command_log('Connection closed -- <span style="color: green;">OK</span>');
        
        connectionId = -1; // reset connection id
        $('#content').empty(); // empty content
        $('#tabs > ul li').removeClass('active'); // de-select any selected tabs
        sensor_status(sensors_detected = 0x00); // reset active sensor indicators
    } else { // Something went wrong
        if (connectionId > 0) {
            console.log('There was an error that happened during "connection-close" procedure.');
            command_log('Connection closed -- <span style="color: red;">ERROR</span>');
        }
    }    
};

function readPoll() {
    chrome.serial.read(connectionId, 24, onCharRead);
};

function stop_data_stream() {
    var bufferOut = new ArrayBuffer(6);
    var bufView = new Uint8Array(bufferOut);

    // sync char 1, sync char 2, command, payload length MSB, payload length LSB, payload
    bufView[0] = 0xB5; // sync char 1
    bufView[1] = 0x62; // sync char 2
    bufView[2] = 0x07; // command
    bufView[3] = 0x00; // payload length MSB
    bufView[4] = 0x01; // payload length LSB
    bufView[5] = 0x01; // payload   

    chrome.serial.write(connectionId, bufferOut, function(writeInfo) {
        console.log("STOP DATA STREAM command - Wrote: " + writeInfo.bytesWritten + " bytes");
    });     
};


var packet_state = 0;
var command_buffer = new Array();
var command;

var message_length_expected = 0;
var message_length_received = 0;
var message_buffer;
var message_buffer_uint8_view;
var char_counter = 0;

function onCharRead(readInfo) {
    if (readInfo && readInfo.bytesRead > 0 && readInfo.data) {
        var data = new Uint8Array(readInfo.data);
        
        for (var i = 0; i < data.length; i++) {
            switch (packet_state) {
                case 0:
                    if (data[i] == 0xB5) { // sync char 1                 
                        packet_state++;
                    }
                break;
                case 1:
                    if (data[i] == 0x62) { // sync char 2                 
                        packet_state++;
                    } else {
                        packet_state = 0; // Restart and try again
                    }                    
                break;
                case 2: // command
                    command = data[i];
                    
                    packet_state++;
                break;
                case 3: // payload length MSB
                    message_length_expected = data[i] << 8;
                    
                    packet_state++;
                break;
                case 4: // payload length LSB
                    message_length_expected |= data[i];
                    
                    // setup arraybuffer
                    message_buffer = new ArrayBuffer(message_length_expected);
                    message_buffer_uint8_view = new Uint8Array(message_buffer);
                    
                    packet_state++;
                break;
                case 5: // payload
                    message_buffer_uint8_view[message_length_received] = data[i];
                    message_length_received++;
                    
                    if (message_length_received >= message_length_expected) {
                        // message received, process
                        process_data();
                        
                        // Reset variables
                        message_length_received = 0;
                        
                        packet_state = 0;
                    }
                break;
            }
            
            char_counter++;
        }
    }
}

function process_data() {
    switch (command) {
        case 1: // configuration data
            console.log('Expected UNION size: ' + message_length_expected + ', Received UNION size: ' + message_buffer_uint8_view.length);
            // Store UNION size for later usage
            eepromConfigSize = message_length_expected;
            
            var view = new DataView(message_buffer, 0);
            view.parseUNION(eepromConfig); 
            
            if (version != eepromConfig.version) {
                command_log('Configurator version doesn\'t match the Flight software version');
                command_log('Configurator version: <strong>' + version + '</strong> - Flight software version: <strong>' + eepromConfig.version + '</strong>');
                command_log('<span style="color: red">Please upgrade your flight software and configurator to the lastest version.</span>');
                
                // Disconnect
                $('div#port-picker a.connect').click();
                
                break;
            }
            
            $('#tabs li a:first').click();
            command_log('Configuration UNION received -- <span style="color: green">OK</span>');
        break;
        case 3: // sensor data
            process_data_sensors();
        break;
        case 4: // receiver data
            process_data_receiver();
        break;
        case 5: // vehicle view
            process_vehicle_view();
        break;
        case 6:
            process_motor_output();
        break;
        case 8: // accel calibration data
            process_accel_calibration();
        break;
        case 9: // ACK
            var message = parseInt(message_buffer_uint8_view[0]);
            
            if (message == 1) {
                console.log("ACK");
                command_log('Flight Controller responds with -- <span style="color: green">ACK</span>');
            } else {
                console.log("REFUSED");
                command_log('Flight Controller responds with -- <span style="color: red">REFUSED</span>');
            }
        break;
        case 11: // Motor amount / count
            motors = parseInt(message_buffer_uint8_view[0]);
        break;
        case 12:
            sensors_detected = parseInt((message_buffer_uint8_view[0] << 8) | message_buffer_uint8_view[1]);
            sensor_status(sensors_detected);            
        break;
    }
}

function sensor_status(sensors_detected) {
    var e_sensor_status = $('div#sensor-status');
    
    if (bit_check(sensors_detected, 0)) { // Gyroscope detected
        $('.gyro', e_sensor_status).addClass('on');
    } else {
        $('.gyro', e_sensor_status).removeClass('on');
    }
    
    if (bit_check(sensors_detected, 1)) { // Accelerometer detected
        $('.accel', e_sensor_status).addClass('on');
    } else {
        $('.accel', e_sensor_status).removeClass('on');
    }

    if (bit_check(sensors_detected, 2)) { // Magnetometer detected
        $('.mag', e_sensor_status).addClass('on');
    } else {
        $('.mag', e_sensor_status).removeClass('on');
    }  

    if (bit_check(sensors_detected, 3)) { // Barometer detected
        $('.baro', e_sensor_status).addClass('on');
    } else {
        $('.baro', e_sensor_status).removeClass('on');
    }  
}

function port_usage() {
    var port_usage = (char_counter * 10 / selected_baud) * 100;
    $('span.port-usage').html(parseInt(port_usage) + ' %');

    // reset counter
    char_counter = 0;
}

function highByte(num) {
    return num >> 8;
}

function lowByte(num) {
    return 0x00FF & num;
}

function bit_check(num, bit) {
    return ((num >> bit) % 2 != 0)
}