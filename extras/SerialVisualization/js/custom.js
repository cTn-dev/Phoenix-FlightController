var connectionId = -1;
var port_list = true;

var graph_gyro;
var graph_accel;
var graph_kinematics;
var graph_tx;

var ar;
var yaw_fix = 0.0;

var cube

$(document).ready(function() {    
    var port_picker = $('div#port-picker .port');
    var baud_picker = $('div#port-picker #baud');
    cube = $('div#cube');
    
    $('div#port-picker a.refresh').click(function() {
        console.log("Available port list requested.");
        port_picker.html('');

        chrome.serial.getPorts(function(ports) {
            if (ports.length > 0) {
                // Port list received
                port_list = true;
                port_picker.html('<select id="port"></select>');
                
                ports.forEach(function(port) {
                    $('select', port_picker).append($("<option/>", {
                        value: port,
                        text: port
                    }));        
                });
            } else {
                // Looks like this check is kinda useless as the serial API doesn't seem to work in windows
                // at all, requires v25>
                // No serial ports found (do something/offer solution)
                port_list = false;
                console.log("No serial ports detected");
                
                port_picker.html('<input id="port" type="text" value=""></input>');
                
            }
        });
    });
    // software click to refresh port picker select during first load
    $('div#port-picker a.refresh').click();
    
    $('div#port-picker a.connect').toggle(function() {
        if (port_list) {
            var selected_port = $('select#port', port_picker).val();
        } else {
            var selected_port = $('input#port', port_picker).val();
        }   

        var selected_baud = parseInt(baud_picker.val());
        
        chrome.serial.open(selected_port, {
            bitrate: selected_baud
        }, onOpen);
        
        $(this).text('Disconnect');
    }, function() {
        chrome.serial.close(connectionId, onClosed); // Seems to be broken
        
        $(this).text('Connect');
    });
    
    // Reset Z axis in 3D visualization
    $('div#interactive_block > a.reset').click(function() {
        yaw_fix = ar[8];
        console.log("YAW reset to 0");
    });
    
    
    // =========== Setup Graphs ===========
    graph_gyro = new Rickshaw.Graph( {
        element: document.getElementById("graph_gyro"),
        width: 600,
        height: 150,
        renderer: 'line',
        min: -10,
        max: 10,
        series: new Rickshaw.Series.FixedDuration([{ name: 'one' }], undefined, {
            timeInterval: 10,
            maxDataPoints: 600,
            timeBase: new Date().getTime()
        }) 
    } );

    graph_gyro.render();   

    graph_accel = new Rickshaw.Graph( {
        element: document.getElementById("graph_accel"),
        width: 600,
        height: 150,
        renderer: 'line',
        min: -1.5,
        max: 1.5,
        series: new Rickshaw.Series.FixedDuration([{ name: 'one' }], undefined, {
            timeInterval: 10,
            maxDataPoints: 600,
            timeBase: new Date().getTime()
        }) 
    } );

    graph_gyro.render();       
    
    graph_kinematics = new Rickshaw.Graph( {
        element: document.getElementById("graph_kinematics"),
        width: 600,
        height: 150,
        renderer: 'line',
        min: -200,
        max: 200,
        series: new Rickshaw.Series.FixedDuration([{ name: 'one' }], undefined, {
            timeInterval: 10,
            maxDataPoints: 600,
            timeBase: new Date().getTime()
        }) 
    } );

    graph_kinematics.render();
    
    graph_tx = new Rickshaw.Graph( {
        element: document.getElementById("graph_tx"),
        width: 350,
        height: 150,
        renderer: 'line',
        min: -600,
        max: 1100,
        series: new Rickshaw.Series.FixedDuration([{ name: 'one' }], undefined, {
            timeInterval: 10,
            maxDataPoints: 350,
            timeBase: new Date().getTime()
        }) 
    } );

    graph_tx.render();    
    
});

function onOpen(openInfo) {
    connectionId = openInfo.connectionId;
    
    if (connectionId != -1) {
        console.log('Connection was opened with ID: ' + connectionId);
        
        // Start reading
        chrome.serial.read(connectionId, 1000, onCharRead);
    } else {
        console.log('There was a problem in opening the connection.');
    }    
};

function onClosed(result) {
    if (result) console.log('Connection closed successfully.');
    else console.log('There was an error that happened during "connection-close" procedure.');
};

function onCharRead(readInfo) {
    if (readInfo && readInfo.bytesRead > 0 && readInfo.data) {
        var str = ab2str(readInfo.data);
        if (str[readInfo.bytesRead-1]==='\n') {
            str = str.substring(0, readInfo.bytesRead-1);
        }
        
        // Line read
        // str = standard string
        // ar = str split into array
        ar = str.split(",");
        
        // Gyro
        ar[0] = parseFloat(ar[0]); // X
        ar[1] = parseFloat(ar[1]); // Y
        ar[2] = parseFloat(ar[2]); // Z
        
        var data_gyro = {one: ar[0], two: ar[1], three: ar[2]};
        graph_gyro.series.addData(data_gyro);
        graph_gyro.render();        
        
        // Accel
        ar[3] = parseFloat(ar[3]); // X
        ar[4] = parseFloat(ar[4]); // Y
        ar[5] = parseFloat(ar[5]); // Z

        var data_accel = {one: ar[4], two: ar[3], three: ar[5]}; // X and Y axis are in reverse here, to match kinematics colors
        graph_accel.series.addData(data_accel);
        graph_accel.render();          
        
        // Kinematics
        ar[6] = parseFloat(ar[6]); // Roll
        ar[7] = parseFloat(ar[7]); // Pitch
        ar[8] = parseFloat(ar[8]); // Yaw  

        var data_kinematics = {one: ar[6], two: ar[7], three: ar[8]};
        graph_kinematics.series.addData(data_kinematics);
        graph_kinematics.render();
        
        // Cube visualization 
        ar[7] = -ar[7]; // Reverse pitch
        
        //ar[6] = 0; // Roll disabled
        //ar[7] = 0; // Pitch disabled
        //ar[8] = 0; // YAW disabled
        
        $('#cube').css('-webkit-transform', 'rotateY(' +(ar[8] - yaw_fix) + 'deg)');
        $('#cubePITCH', cube).css('-webkit-transform', 'rotateX(' + ar[7] + 'deg)');
        $('#cubeROLL', cube).css('-webkit-transform', 'rotateZ(' + ar[6] + 'deg)');        
        
        // TX
        ar[9] = parseInt(ar[9]); // TX Roll
        ar[10] = parseInt(ar[10]); // TX Pitch
        ar[11] = parseInt(ar[11]) - 1000; // TX Throttle
        ar[12] = parseInt(ar[12]); // TX Yaw
        ar[13] = parseInt(ar[13]) - 1000; // TX Mode
        ar[14] = parseInt(ar[14]) - 1000; // TX baro
        ar[15] = parseInt(ar[15]) - 1000; // TX cam
        ar[16] = parseInt(ar[16]) - 1000; // TX last
        
        var data_tx = {one: ar[9], two: ar[10], three: ar[11], four: ar[12], five: ar[13], six: ar[14], seven: ar[15], eight: ar[16]};
        graph_tx.series.addData(data_tx);
        graph_tx.render();
        
        // PPM error
        ar[17] = parseInt(ar[17]);
        
        // Motors
        ar[18] = parseInt((parseInt(ar[18]) - 1000) * 0.255);
        ar[19] = parseInt((parseInt(ar[19]) - 1000) * 0.255);
        ar[20] = parseInt((parseInt(ar[20]) - 1000) * 0.255);
        ar[21] = parseInt((parseInt(ar[21]) - 1000) * 0.255);
        
        $('.motor.one', cube).css('background-color', 'rgb(' + ar[18] + ', 0, 0)');
        $('.motor.two', cube).css('background-color', 'rgb(' + ar[19] + ', 0, 0)');
        $('.motor.three', cube).css('background-color', 'rgb(' + ar[20] + ', 0, 0)');
        $('.motor.four', cube).css('background-color', 'rgb(' + ar[21]  + ', 0, 0)');
        
        
    }
    chrome.serial.read(connectionId, 1000, onCharRead);
}

var ab2str = function(buf) {
    return String.fromCharCode.apply(null, new Uint8Array(buf));
};