Ubuntu (and ubuntu distro based) users might need to add their selfs to the dialout group
otherwise they would not have enough permissions to join the serial bus and configurator
would just end up returning an "Could not join the serial bus" error.
sudo usermod -aG dialout YOUR_USERNAME

After more then 5 days of bug "hunting", there seems to be a bug in all linux chrome/chromium versions i tested 
(v24 stable, v25 beta, v27 nightly (183010))
which causes chrome serial to fail setting a correct baud rate (by default it reverts back to 9600).

Current issue ticket on chromium
https://code.google.com/p/chromium/issues/detail?id=176711

Chromium patch from X-warrior and me
https://codereview.chromium.org/12294009/

For now, the only "proper" way to avoid this problem is to set the serial bus baud rate manually with 3rd party
utility, for example stty

stty -F /dev/ttyUSB0 38400

I will update/remove this file when the underlying chrome bug gets fixed.
