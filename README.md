Requires libtins and libpcap to be present on host system.

Program will track IPv4 packet stats and dump to an SQL database periodically. An HTTP server is included as well with a simple API which will grab info from the SQL database and send a response with the info encoded in JSON format. A sample Python script is included showing how one would query the API.
