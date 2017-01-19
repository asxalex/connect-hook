#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2017 alex <alex@localhost>
#
# Distributed under terms of the MIT license.

import socket
import time

LOCALADDR = "127.0.0.1"
LOCALPORT = 12310

REMOTEADDR = LOCALADDR
REMOTEPORT = 8085

def test():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
    print socket.htons(LOCALPORT)
    sock.bind((LOCALADDR, LOCALPORT))
    try:
        sock.connect((REMOTEADDR, REMOTEPORT))
    except Exception, e:
        print e
    sock.close()

if __name__ == "__main__":
    test()
