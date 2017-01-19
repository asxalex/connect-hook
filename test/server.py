#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2017 alex <alex@localhost>
#
# Distributed under terms of the MIT license.

import socket

HOST = "127.0.0.1"
PORT = 8085

def serve():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print "dst port is %d" % socket.htons(PORT)
    sock.bind((HOST, PORT))
    sock.listen(5)
    while True:
        s, _ = sock.accept()
        s.close()
    sock.close()

if __name__ == "__main__":
    serve()

