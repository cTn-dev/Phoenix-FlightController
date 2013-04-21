function tab_initialize_gps() {

    // start pulling gps data
    timers.push(setInterval(gps_pull, 100));  // 10 Hz - more then enough, as most of the modules run on 1-5Hz
}

function gps_pull() {
    // request new data
    send_message(PSP.PSP_REQ_GPS, 1);
}