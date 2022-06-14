#!/usr/bin/env python3
from http.server import HTTPServer, SimpleHTTPRequestHandler, test
import socketserver
import sys

class ThreadedHTTPServer(socketserver.ThreadingMixIn, HTTPServer, SimpleHTTPRequestHandler):
    daemon_threads = True

    def end_headers (self):
        self.send_header('Access-Control-Allow-Origin', '*')
        SimpleHTTPRequestHandler.end_headers(self)

class CORSRequestHandler (SimpleHTTPRequestHandler):
    def end_headers (self):
        self.send_header('Access-Control-Allow-Origin', '*')
        SimpleHTTPRequestHandler.end_headers(self)

if __name__ == '__main__':
    # port = int(sys.argv[1])
    # server = ThreadedHTTPServer(('', port), SimpleHTTPRequestHandler)

    # try:
    #     server.serve_forever()
    # except KeyboardInterrupt:
    #     pass
    test(CORSRequestHandler, HTTPServer, port=int(sys.argv[1]) if len(sys.argv) > 1 else 8000)

