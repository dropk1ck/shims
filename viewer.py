#!/usr/bin/env python2
from __future__ import print_function
from collections import namedtuple
import os
import socket
import sys

socket_addr = '/tmp/preload_sock'
MemRegion = namedtuple('MemRegion', 'start end name')

class Viewer(object):
    def __init__(self):
        self.sock = None
        self.client_pid = 0
        self.maps = []
        if (os.path.exists(socket_addr)):
            os.remove(socket_addr)

    def recvline(self):
        buf = ''
        while True:
            c = self.sock.recv(1)
            if c == '\n':
                break
            buf += c
        return buf

    def parse_map(self):
        with open('/proc/{}/maps'.format(self.client_pid), 'r') as f:
            maps = f.read().splitlines()
            for line in maps:
                toks = line.split(' ')
                addr_range = toks[0].split('-')
                region = MemRegion(int(addr_range[0], 16), int(addr_range[1], 16), toks[-1].split('/')[-1])
                self.maps.append(region)

    def lookup_image(self, addr):
        for region in self.maps:
            if addr >= region.start and addr <= region.end:
                return region.name
        return '<unknown>' 

    def run(self):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.bind(socket_addr)
        sock.listen(1)
        print('[=] Listening...')

        self.sock, addr = sock.accept()
        self.client_pid = self.recvline()
        print('[=] Connection from PID {}'.format(self.client_pid))

        # parse the memory map of the process
        self.parse_map()

        # main loop
        while True:
            try:
                data = self.recvline()
            except:
                self.sock.close()
                sock.close()
                return
            if 'FIN' in data:
                # all done here
                break
            calling_addr, call = data.split(':')
            print('{} -> {}'.format(self.lookup_image(int(calling_addr.strip('\x00'),16)), call))

if __name__ == '__main__':
    while True:
        viewer = Viewer()
        viewer.run()
