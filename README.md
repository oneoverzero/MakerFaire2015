# MakerFaire2015
=
Code for all things Maker Faire 2015

mission-control contains all server code
That is the web page and the MQTT gateway python script


MarsRover contains everything else taht runs on the Rover itself

Arduino0 collects and sends telemetry
Arduino2 takes care of physical electronics
Raspi2 provides video streams and gateway to electronics

runs python scripts as both client and publisher to MQTT queues on Mission Control



WARNING: the web page is a "git submodule" you must run to get it

