Ubuntu (and ubuntu distro based) users might need to add their selfs to the dialout group
otherwise they would not have enough permissions to join the serial bus and configurator
would just end up returning an "Could not join the serial bus" error.
sudo usermod -aG dialout YOUR_USERNAME