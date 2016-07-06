This simple simulation project is to demosnstrate how to build and run simple simulation using OpenIg.
The project contains three applications and one Plugin:

1) openig-client-ig: Uses the installed OpenIG-Plugin-Client plugin that waits for simple packets from the
openig-client-host application over UDP and moves an entity clamped on the terrain. Make sure the OpenIG-Plugin-Client
is installed properly which it should be happened automatically when run 'make install'

2) openig-client-tqserver: Simple TCP Server that reads a database and performs HOT (Height of Terrain) and LOS
(Line of Sight) tests based on TCP queries from the Host and sends back respose

3) openig-client-host: Simple host application. Maked TCP connection to the Terrain Query Server and performs
HAT and LOS queries, wait for their response, place a model on the terrain and send UDP packets to the Image
Generator to move an entity on the terrain

To run once properly built and installed:
1) run 'openig-client-ig' (the OpenIG-Plugin-Client.dll.xml contains settings for the network interface and port)
2) run 'openig-client-tqserver ..\data\terrain\master.flt.osg' (the first argument is the terrain to perform the
HAT/LOS tests against, also '--help' prints the other options for setting the network interface and port)
3) run 'openig-client-host' ('--help' prints the options for setting the network interface and ports)