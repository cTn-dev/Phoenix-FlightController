chrome.app.runtime.onLaunched.addListener(function() {
    chrome.app.window.create('main.html', {
        frame: 'chrome',
        id: 'main-window',
        minWidth: 960,
        maxWidth: 960,
        minHeight: 550,
        maxHeight: 550
    }, function(main_window) {
        main_window.onClosed.addListener(function() {
            // connectionId is passed from the script side through the chrome.runtime.getBackgroundPage refference
            // allowing us to automatically close the port when application shut down
            if (connectionId != -1) {
                chrome.serial.close(connectionId, function() {
                    console.log('CLEANUP: Connection to serial port was opened after application closed, closing the connection.');
                });
            }
        });
    });
});