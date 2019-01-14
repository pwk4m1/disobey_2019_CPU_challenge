
To set up the challenge, please do run following commands:

	$ cd server_side
	$ make
	$ ./wrapper.py <IP address to listen> <port>&>/dev/null 2>&1

The wrapper acts as a, well, wrapper between the player, and tiny emulator
found in server_side/bin folder.
The wrapper-script should have write-access to /tmp folder.

Please share the files on directory `share_to_player`
for players attending the CTF, as well as IP:PORT pair
used for wrapper.py script on `server_side` directory. 


