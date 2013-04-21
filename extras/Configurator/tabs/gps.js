function tab_initialize_gps() {

    // start pulling gps data (only if gps was detected
    if (sensors_alive.gps) {
        timers.push(setInterval(gps_pull, 100));  // 10 Hz - more then enough, as most of the modules run on 1-5Hz
    }
}

function gps_pull() {
    // update UI with latest data
    var e_inf = $('.tab-gps .gps-info dl');
    $('dd:eq(0)', e_inf).html((GPS_data.lat).toFixed(4) + ' Deg');
    $('dd:eq(1)', e_inf).html((GPS_data.lon).toFixed(4) + ' Deg');
    $('dd:eq(2)', e_inf).html((GPS_data.speed) + ' cm / s');
    $('dd:eq(3)', e_inf).html(GPS_data.height + ' m');
    $('dd:eq(4)', e_inf).html((GPS_data.accuracy).toFixed(2) + ' m');
    
    if (GPS_data.state == 1) { // no fix
        $('dd:eq(5)', e_inf).html('No fix');
    } else if (GPS_data.state == 2) { // 2D fix
        $('dd:eq(5)', e_inf).html('2D fix');
    } else if (GPS_data.state == 3) { // 3D fix
        $('dd:eq(5)', e_inf).html('3D fix');
    }
    
    
    
    $('dd:eq(6)', e_inf).html(GPS_data.sats); // number of satellites

    // request new data
    send_message(PSP.PSP_REQ_GPS, 1);
}