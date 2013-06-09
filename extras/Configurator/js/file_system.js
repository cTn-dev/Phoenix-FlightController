function configuration_backup() {
    var chosenFileEntry = null;
    
    var accepts = [{
        extensions: ['txt']
    }];
    
    // load up the file
    chrome.fileSystem.chooseEntry({type: 'saveFile', suggestedName: 'phoenix_config_backup', accepts: accepts}, function(fileEntry) {
        if (!fileEntry) {
            command_log('<span style="color: red;">No</span> file selected');
            console.log('No file selected');
            
            return;
        }
        
        chosenFileEntry = fileEntry;
        
        // echo/console log path specified
        chrome.fileSystem.getDisplayPath(chosenFileEntry, function(path) {
            command_log('<span style="color: green;">Backup</span> file path: ' + path);
            console.log('Backup file path: ' + path);
        });        
        
        // change file entry from read only to read/write
        chrome.fileSystem.getWritableEntry(chosenFileEntry, function(fileEntryWritable) {
            // check if file is writable
            chrome.fileSystem.isWritableEntry(fileEntryWritable, function(isWritable) {
                if (isWritable) {
                    chosenFileEntry = fileEntryWritable;
                    
                    // crunch the config object
                    var serialized_config_object = JSON.stringify(eepromConfig);
                    var blob = new Blob([serialized_config_object], {type: 'text/plain'}); // first parameter for Blob needs to be an array
                    
                    chosenFileEntry.createWriter(function(writer) {
                        writer.onerror = function (e) {
                            console.error(e);
                        };
                        
                        writer.onwriteend = function() {
                            command_log('Write -- <span style="color: green">SUCCESSFUL</span>');
                            console.log('Write SUCCESSFUL');
                        };
                        
                        //writer.truncate(blob.size);
                        writer.write(blob);
                    }, function (e) {
                        console.error(e);
                    });
                } else {
                    // Something went wrong or file is set to read only and cannot be changed
                    command_log('File appears to be <span style="color: red;">read ONLY</span>');
                    console.log('File appears to be read only, sorry.');
                }
            });
        });
    });
}

function configuration_restore() {
    var chosenFileEntry = null;
    
    var accepts = [{
        extensions: ['txt']
    }];
    
    // load up the file
    chrome.fileSystem.chooseEntry({type: 'openFile', accepts: accepts}, function(fileEntry) {
        if (!fileEntry) {
            command_log('<span style="color: red;">No</span> file selected');
            console.log('No file selected');
            
            return;
        }
        
        chosenFileEntry = fileEntry; 
        
        // echo/console log path specified
        chrome.fileSystem.getDisplayPath(chosenFileEntry, function(path) {
            command_log('<span style="color: green;">Restore</span> file path: ' + path);
            console.log('Restore file path: ' + path);
        }); 

        // read contents into variable
        chosenFileEntry.file(function(file) {
            var reader = new FileReader();

            reader.onerror = function (e) {
                console.error(e);
            };
            
            reader.onloadend = function(e) {
                command_log('Read <span style="color: green;">SUCCESSFUL</span>');
                console.log('Read SUCCESSFUL');
                
                deserialized_config_object = JSON.parse(e.target.result);
                
                // replacing "old configuration" with configuration from backup file
                if (eepromConfig.version == deserialized_config_object.version) {
                    command_log('EEPROM version number/pattern -- <span style="color: green;">MATCH</span>');
                    console.log('EEPROM version number/pattern matches backup EEEPROM file version/pattern');
                } else {
                    command_log('EEPROM version number/pattern -- <span style="color: red;">MISSMATCH</span> (manual values re-validation is advised)');
                    console.log('EEPROM version number/pattern doesn\'t match backup EEPROM file version/pattern (manual values re-validation is advised)');
                }
                
                eepromConfig = deserialized_config_object;
                
                // Send updated UNION to the flight controller
                sendUNION();
            };

            reader.readAsText(file);
        });
    });
}