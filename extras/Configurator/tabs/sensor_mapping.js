var TEMP_SENSOR_AXIS_MAP;

function tab_initialize_sensor_mapping() {
    // populate the select boxes
    // gyroscope
    gyro_axis1 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[0], 0);
    gyro_axis2 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[0], 3);
    gyro_axis3 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[0], 6);
    
    $('table.gyroscope tr:eq(1) input').eq(gyro_axis1.axis).prop('checked', true);
    $('table.gyroscope tr:eq(1) input').eq(3).prop('checked', gyro_axis1.sign);
    $('table.gyroscope tr:eq(2) input').eq(gyro_axis2.axis).prop('checked', true);
    $('table.gyroscope tr:eq(2) input').eq(3).prop('checked', gyro_axis2.sign);
    $('table.gyroscope tr:eq(3) input').eq(gyro_axis3.axis).prop('checked', true);
    $('table.gyroscope tr:eq(3) input').eq(3).prop('checked', gyro_axis3.sign);
    
    // accelerometer
    accel_axis1 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[1], 0);
    accel_axis2 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[1], 3);
    accel_axis3 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[1], 6);   

    $('table.accelerometer tr:eq(1) input').eq(accel_axis1.axis).prop('checked', true);
    $('table.accelerometer tr:eq(1) input').eq(3).prop('checked', accel_axis1.sign);
    $('table.accelerometer tr:eq(2) input').eq(accel_axis2.axis).prop('checked', true);
    $('table.accelerometer tr:eq(2) input').eq(3).prop('checked', accel_axis2.sign);
    $('table.accelerometer tr:eq(3) input').eq(accel_axis3.axis).prop('checked', true); 
    $('table.accelerometer tr:eq(3) input').eq(3).prop('checked', accel_axis3.sign);    

    // magnetometer
    mag_axis1 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[2], 0);
    mag_axis2 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[2], 3);
    mag_axis3 = check_axis_position(eepromConfig.SENSOR_AXIS_MAP[2], 6);  
    
    $('table.magnetometer tr:eq(1) input').eq(mag_axis1.axis).prop('checked', true);
    $('table.magnetometer tr:eq(1) input').eq(3).prop('checked', mag_axis1.sign);
    $('table.magnetometer tr:eq(2) input').eq(mag_axis2.axis).prop('checked', true);
    $('table.magnetometer tr:eq(2) input').eq(3).prop('checked', mag_axis2.sign);
    $('table.magnetometer tr:eq(3) input').eq(mag_axis3.axis).prop('checked', true);
    $('table.magnetometer tr:eq(3) input').eq(3).prop('checked', mag_axis3.sign);    
    
    
    // UI hooks
    $('table input').click(function() {
        parent = $(this).parent().parent();
        
        if ($(this).attr('name') == 'sign') {
            // Special handling for sign
        } else {
            // Standard handling for axis
            $('input', parent).not('[name="sign"]').removeAttr('checked');
            $(this).prop('checked', true);
        }
    });
    
    $('div.tab-sensor_mapping a.restore').click(function() {
        eepromConfig.SENSOR_AXIS_MAP = [0, 0, 0];
    
        // Send updated UNION to the flight controller
        sendUNION();

        // refresh the UI
        $('#content').load("./tabs/sensor_mapping.html", tab_initialize_sensor_mapping);
    });
    
    $('div.tab-sensor_mapping a.update').click(function() {
        // re-calculate temporary maps
        TEMP_SENSOR_AXIS_MAP = [0, 0, 0];
        $('table.gyroscope tr:eq(1) input:checked').each(function(index, element) {
            set_axis_position(element, 0, 0);
        });
        $('table.gyroscope tr:eq(2) input:checked').each(function(index, element) {
            set_axis_position(element, 0, 3);
        });
        $('table.gyroscope tr:eq(3) input:checked').each(function(index, element) {
            set_axis_position(element, 0, 6);
        }); 

        $('table.accelerometer tr:eq(1) input:checked').each(function(index, element) {
            set_axis_position(element, 1, 0);
        });
        $('table.accelerometer tr:eq(2) input:checked').each(function(index, element) {
            set_axis_position(element, 1, 3);
        });
        $('table.accelerometer tr:eq(3) input:checked').each(function(index, element) {
            set_axis_position(element, 1, 6);
        });

        $('table.magnetometer tr:eq(1) input:checked').each(function(index, element) {
            set_axis_position(element, 2, 0);
        });
        $('table.magnetometer tr:eq(2) input:checked').each(function(index, element) {
            set_axis_position(element, 2, 3);
        });
        $('table.magnetometer tr:eq(3) input:checked').each(function(index, element) {
            set_axis_position(element, 2, 6);
        });  

        // we cant forget the initialization bit
        if (bit_check(eepromConfig.SENSOR_AXIS_MAP[0], 9)) {
            TEMP_SENSOR_AXIS_MAP[0] = bit_set(TEMP_SENSOR_AXIS_MAP[0], 9);
        }

        if (bit_check(eepromConfig.SENSOR_AXIS_MAP[1], 9)) {
            TEMP_SENSOR_AXIS_MAP[1] = bit_set(TEMP_SENSOR_AXIS_MAP[1], 9);
        }

        if (bit_check(eepromConfig.SENSOR_AXIS_MAP[2], 9)) {
            TEMP_SENSOR_AXIS_MAP[2] = bit_set(TEMP_SENSOR_AXIS_MAP[2], 9);
        }
        
        // push temp map values to the primary map
        eepromConfig.SENSOR_AXIS_MAP[0] = TEMP_SENSOR_AXIS_MAP[0];
        eepromConfig.SENSOR_AXIS_MAP[1] = TEMP_SENSOR_AXIS_MAP[1];
        eepromConfig.SENSOR_AXIS_MAP[2] = TEMP_SENSOR_AXIS_MAP[2];
        
        
        // Send updated UNION to the flight controller
        sendUNION();
    });
}

function check_axis_position(input, num) {
    var axis = 0;
    var sign = 0;
    
    if (bit_check(input, num)) { // offset start
        axis += 1;
    }
    
    if (bit_check(input, num + 1)) { // offset + 1
        axis += 2;
    }
    
    if (bit_check(input, num + 2)) { // offset + 2
        sign += 1;
    }
    
    return {axis: axis, sign: sign};
}

function set_axis_position(element, sensor, offset) {
    var name = $(element).attr('name');
    
    switch(name) {
        case 'x':
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_clear(TEMP_SENSOR_AXIS_MAP[sensor], offset);
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_clear(TEMP_SENSOR_AXIS_MAP[sensor], offset + 1);
            break;
        case 'y':
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_set(TEMP_SENSOR_AXIS_MAP[sensor], offset);
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_clear(TEMP_SENSOR_AXIS_MAP[sensor], offset + 1);
            break;
        case 'z':
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_clear(TEMP_SENSOR_AXIS_MAP[sensor], offset);
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_set(TEMP_SENSOR_AXIS_MAP[sensor], offset + 1);
            break;
        case 'sign':
            TEMP_SENSOR_AXIS_MAP[sensor] = bit_set(TEMP_SENSOR_AXIS_MAP[sensor], offset + 2);
            break;
    }
}