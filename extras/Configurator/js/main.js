var version = 4; // configurator version to check against version number stored in eeprom

var connectionId = -1;
var connection_delay = 0; // delay which defines "when" will the configurator request configurator data after connection was established

var timers = new Array();

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
                chrome.serial.close(connectionId, onClosed);
                
                clearTimeout(connection_delay);
                clearInterval(serial_poll);
                clearInterval(port_usage_poll);
                // Disable any active "data pulling" timer
                disable_timers();
                
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

            // Disable any active "data pulling" timer
            disable_timers();
            
            // disable previous active button
            $('li', tabs).removeClass('active');
            
            // Highlight selected button
            $(this).parent().addClass('active');
            
            if ($(this).parent().hasClass('tab_initial_setup')) {
                $('#content').load("./tabs/initial_setup.html", tab_initialize_initial_setup);
            } else if ($(this).parent().hasClass('tab_pid_tuning')) {
                $('#content').load("./tabs/pid_tuning.html", tab_initialize_pid_tuning);
            } else if ($(this).parent().hasClass('tab_channel_assigner')) {
                $('#content').load("./tabs/channel_assigner.html", tab_initialize_channel_assigner);
            } else if ($(this).parent().hasClass('tab_function_triggers')) {
                $('#content').load("./tabs/function_triggers.html", tab_initialize_function_triggers);
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
            } else if ($(this).parent().hasClass('tab_gps')) {
                $('#content').load("./tabs/gps.html", tab_initialize_gps);
            } else if ($(this).parent().hasClass('tab_about')) {
                $('#content').load("./tabs/about.html");
            }            
        }
    });
    
    // used for development only
    //$('#content').load("./tabs/initial_setup.html", tab_initialize_initial_setup);
});

function command_log(message) {
    var d = new Date();
    var time = ((d.getHours() < 10) ? '0' + d.getHours(): d.getHours()) 
        + ':' + ((d.getMinutes() < 10) ? '0' + d.getMinutes(): d.getMinutes()) 
        + ':' + ((d.getSeconds() < 10) ? '0' + d.getSeconds(): d.getSeconds());
    
    $('div#command-log > div.wrapper').append('<p>' + time + ' -- ' + message + '</p>');
    $('div#command-log').scrollTop($('div#command-log div.wrapper').height());    
};

function disable_timers() {
    for (var i = 0; i < timers.length; i++) {
        clearInterval(timers[i]);
    }
    
    // kill all the refferences
    timers = [];
}    

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
            send_message(PSP.PSP_REQ_MOTORS_COUNT, 1);

            // requesting sensors detected
            send_message(PSP.PSP_REQ_SENSORS_ALIVE, 1);
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
    chrome.serial.read(connectionId, 64, onCharRead);
}

function sensor_status(sensors_detected) {
    var e_sensor_status = $('div#sensor-status');
    
    if (bit_check(sensors_detected, 0)) { // Gyroscope detected
        $('.gyro', e_sensor_status).addClass('on');
        sensors_alive.gyro = 1;
    } else {
        $('.gyro', e_sensor_status).removeClass('on');
        sensors_alive.gyro = 0;
    }
    
    if (bit_check(sensors_detected, 1)) { // Accelerometer detected
        $('.accel', e_sensor_status).addClass('on');
        sensors_alive.accel = 1;
    } else {
        $('.accel', e_sensor_status).removeClass('on');
        sensors_alive.accel = 0;
    }

    if (bit_check(sensors_detected, 2)) { // Magnetometer detected
        $('.mag', e_sensor_status).addClass('on');
        sensors_alive.mag = 1;
    } else {
        $('.mag', e_sensor_status).removeClass('on');
        sensors_alive.mag = 0;
    }  

    if (bit_check(sensors_detected, 3)) { // Barometer detected
        $('.baro', e_sensor_status).addClass('on');
        sensors_alive.baro = 1;
    } else {
        $('.baro', e_sensor_status).removeClass('on');
        sensors_alive.baro = 0;
    }  
    
    if (bit_check(sensors_detected, 4)) { // GPS detected
        $('.gps', e_sensor_status).addClass('on');
        sensors_alive.gps = 1;
    } else {
        $('.gps', e_sensor_status).removeClass('on');
        sensors_alive.gps = 0;
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
    return ((num) & (1 << (bit)));
}

function bit_set(num, bit) {
    return num | 1 << bit;
}

function bit_clear(num, bit) {
    return num & ~(1 << bit);
}