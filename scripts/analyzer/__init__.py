#! /usr/bin/env python
# encoding: utf-8

import http.server
import os
import json

class AnalyzerRequestHandler(http.server.BaseHTTPRequestHandler):
    def __init__(self, self_path, result_path, *args):
        self.self_path = self_path
        self.result_path = result_path
        super().__init__(*args)
    
    def do_GET(self):
        try:
            path = self.path[1:]
            if len(path) == 0:
                path = "index.html"

            if path == "enumerate_tests.json":
                tests = [x for x in os.listdir(self.result_path) if x.endswith('_profile.txt')]
                tests.sort()
                tests.reverse()
                data = json.dumps(tests).encode('utf-8')
            else:
                res_path = os.path.join(self.result_path, path)
                print(res_path)
                if not os.path.exists(res_path):
                    res_path = os.path.join(self.self_path, path)
                print(res_path)
                if not os.path.exists(res_path):
                    raise IOError()

                f = open(res_path, 'rb')
                data = f.read()
                f.close()
                       
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

class AnalyzerContext:

    def __init__(self, result_path):
        self.self_path = os.path.dirname(os.path.realpath(__file__))
        self.result_path = result_path
    
    def handler(self, *args):
        return AnalyzerRequestHandler(self.self_path, self.result_path, *args)

class AnalyzerServer:

    def __init__(self, port, result_path):
        self.context = AnalyzerContext(result_path)
        self.server = http.server.HTTPServer(('', port), self.context.handler)

    def run(self):
        try:
            self.server.serve_forever()
        except KeyboardInterrupt:
            pass

        self.server.server_close()
