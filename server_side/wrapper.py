#!/usr/local/bin/python

import subprocess as sp
import socket
import select
import base64
import string
import random
import sys
from threading import Thread

def tcp_socket():
    try:
        return socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except Exception as Err:
        print(Err)
        return None

def write_bin(fname, data):
    ret = None
    try:
        with open(fname, 'wb') as f:
            f.write(data)
        f.close()
    except Exception as Err:
        print(Err)
        ret = -1
    finally:
        return ret

class API:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = tcp_socket()
        self.running = False
        self.cli_cnt = 0

    # Listen for 65 kilobytes of data from client,
    # decode it from base64, and pass it on for emulator via
    # /tmp/emul_<ident>.bin file.
    #
    def serve_client(self, conn):
        ret = None
        conn.setblocking(0)
        try:
            r1 = select.select([conn], [], [], 10)
            if (r1[0]):
                data = conn.recv(65534)
                data = base64.b64decode(data)
                name = "/tmp/emul_bin_"
                name += str(random.choice(string.letters))
                name += ".bin"

                if (write_bin(name, data)):
                    return
                
                # Data written to file, pass it to emulator
                proc = sp.Popen(["./bin/emulator", name], stdout=sp.PIPE,
                        stderr=sp.STDOUT)
                while (self.running):
                    conn.send(proc.stdout.readline())
                    if (proc.poll() != None):
                        break
                conn.close()
            else:
                conn.close()
        except Exception as Err:
            print("serve_client")
            print(Err)
            conn.send("Incorrect padding, unable to decode b64\r\n")
            conn.close()
            ret = -1
        finally:
            self.cli_cnt -= 1
            return ret

     # Main code to serve all the clients
    def run(self):
       if (not self.sock):
           return
       self.running = True
       try:
           self.sock.bind((self.host, self.port))
       except Exception as Err:
           print("run")
           print(Err)
           return

       while True:
           try:
               self.sock.listen(1)
               conn, addr = self.sock.accept()
               self.cli_cnt += 1
               if (self.cli_cnt > 25):
                   conn.send("Please try again soon\n")
                   conn.send("Server is under too much load atm\n")
               else:
                   Thread(target=self.serve_client, args=(conn,)).start()
           except KeyboardInterrupt:
               self.running = False
               return
           except Exception as Err:
               print("run while")
               print(Err)
               self.running = False
               return

def usage(name):
    print("%s <host> <port>" % str(name))
    
def main(argc, argv):
    if (argc != 3):
        return usage(argv[0])
    try:
        argv[2] = int(argv[2])
    except:
        print("Invalid port")
        return
    api = API(argv[1], argv[2])
    api.run()
    return 0

if __name__ == "__main__":
    sys.exit(main(len(sys.argv), sys.argv))
    

