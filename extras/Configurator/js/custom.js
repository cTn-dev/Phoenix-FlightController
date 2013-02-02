var connectionId = -1;
var port_list;
var serial_poll;

$(document).ready(function() { 
    var port_picker = $('div#port-picker .port');
    var baud_picker = $('div#port-picker #baud');
    var delay_picker = $('div#port-picker #delay');
    
    $('div#port-picker a.refresh').click(function() {
        console.log("Available port list requested.");
        port_picker.html('');

        chrome.serial.getPorts(function(ports) {
            if (ports.length > 0) {
                // Port list received
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
                console.log("No serial ports detected");
            }
        });
    });
    
    // software click to refresh port picker select during first load
    $('div#port-picker a.refresh').click();
    
    $('div#port-picker a.connect').click(function() {
        var clicks = $(this).data('clicks');
        
        if (clicks) { // odd number of clicks
            chrome.serial.close(connectionId, onClosed);
            
            clearInterval(serial_poll);
            
            $(this).text('Connect');        
        } else { // even number of clicks         
            var selected_port = $('select#port', port_picker).val();
            var selected_baud = parseInt(baud_picker.val());
            var selected_delay = parseInt(delay_picker);
            
            chrome.serial.open(selected_port, {
                bitrate: selected_baud
            }, onOpen);
            
            setTimeout(function() {
                serial_poll = setInterval(readPoll, 10);
            }, selected_delay * 1000);    
            
            $(this).text('Disconnect');            
        }
        
        $(this).data("clicks", !clicks);
    }); 

    // Tabs
    var tabs = $('#tabs > ul');
    $('a', tabs).click(function() {
        // disable previous active button
        $('li', tabs).removeClass('active');
        
        // Highlight selected button
        $(this).parent().addClass('active');
        
        switch ($(this).parent().index()) {
            case 0: // initial setup
                $('#content').load("./tabs/initial_setup.html");
            break;
            case 1: // sensor data
                $('#content').load("./tabs/sensor_data.html");
            break;
            case 2: // test
                $('#content').load("./tabs/test.html");
            break;
        }
    });
 
    // Load initial tab to content div
    $('li > a:first', tabs).click(); 
    
    // Specific functions in content
    $('#content').delegate('.calibrateESC', 'click', function() {
        var message = str2ab("[2:0]");
        
        chrome.serial.write(connectionId, message, function(writeInfo) {
            console.log("Written: " + writeInfo.bytesWritten + " bytes");
        });
    });    
    
});

function onOpen(openInfo) {
    connectionId = openInfo.connectionId;
    
    if (connectionId != -1) {
        console.log('Connection was opened with ID: ' + connectionId);
        
        // Start reading
        chrome.serial.read(connectionId, 1, onCharRead);
    } else {
        console.log('There was a problem in opening the connection.');
    }    
};

function onClosed(result) {
    if (result) { // All went as expected
        console.log('Connection closed successfully.');
    } else { // Something went wrong
        console.log('There was an error that happened during "connection-close" procedure.');
    }    
};

function readPoll() {
    chrome.serial.read(connectionId, 24, onCharRead);
};

function onCharRead(readInfo) {
    if (readInfo && readInfo.bytesRead > 0 && readInfo.data) {
        var data = new Uint8Array(readInfo.data);
        
        for (var i = 0; i < data.length; i++) {
            console.log(data[i]);
        }
    }
};


// String to array buffer
function str2ab(str) {
    var buf = new ArrayBuffer(str.length);
    var bufView = new Uint8Array(buf);
    
    for (var i = 0, strLen = str.length; i < strLen; i++) {
        bufView[i] = str.charCodeAt(i);
    }
    
    return buf;
};