#!/usr/bin/python

import http.server
import os
import json

FILES_DIR = os.path.join(os.path.dirname(__file__), 'cogn')
TEST_DIR = "tests/"
TIME_COLLECTOR = "time_collector.txt"

class CognRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        try:
            path = self.path[1:]
            if len(path) == 0:
                path = "index.html"
            
            realpath = os.path.join(FILES_DIR, path)
            if path == "test_files.json":
                if os.path.exists(TIME_COLLECTOR):
                    data = [TIME_COLLECTOR];
                else:
                    data = [os.path.join(f, TIME_COLLECTOR)
                            for f in os.listdir(".")
                            if os.path.isdir(f)]
                data = json.dumps([os.path.join(TEST_DIR, f) for f in data]).encode('utf-8')
            elif path.startswith(TEST_DIR):
                f = open(os.path.join(".", path[len(TEST_DIR):]), 'rb')
                data = f.read()
                f.close()
            elif os.path.exists(realpath):
                f = open(realpath, 'rb')
                data = f.read()
                f.close()
            else:
                raise IOError() 
                        
            self.send_response(200)
            
            if path.endswith(".html"):
                self.send_header('Content-type', 'text/html')
            elif path.endswith(".css"):
                self.send_header('Content-type', 'text/css')
            elif path.endswith(".js"):
                self.send_header('Content-type', 'text/js')
            elif path.endswith(".json"):
                self.send_header('Content-type', 'text/json')
                
            self.end_headers()
            self.wfile.write(data)
        except IOError:
            self.send_error(404,'File Not Found: %s' % self.path)            

server_address = ('', 8000)
httpd = http.server.HTTPServer(server_address, CognRequestHandler)
httpd.serve_forever()
