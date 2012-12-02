#!/usr/bin/env python

import BaseHTTPServer;

def HTTPGetApp(line):
    if line.find("?") >= 0:
        tmpstr = line[:line.find("?")];
    else:
        tmpstr = line;
    return tmpstr[(tmpstr.rfind("/")+1):];

def HTTPGetValue(tmpstr, name):
    if tmpstr.find(name) >= 0:
        sidx = tmpstr.find(name) + len(name) + 1;
        if tmpstr.find('&', sidx) >= 0:
            eidx = tmpstr.find('&', sidx);
        else:
            eidx = len(tmpstr);
        return tmpstr[sidx:eidx];
    else:
        return '';

class CAppServer(BaseHTTPServer.BaseHTTPRequestHandler):

    def do_GET(self):
        app = HTTPGetApp(self.path)
        self.reply(app, self.path)

    def do_POST(self):
        app = HTTPGetApp(self.path)
        line = self.rfile.readline()
        self.reply(app, line)

    def reply(self, app, line):
        global handleHash
        hasValue = False
        val = {}
        for name in handleHash[app].namespace:
            val[name] = HTTPGetValue(line, name)
            if len(val[name]) > 0:
                hasValue = True
        if len(handleHash[app].namespace) == 0 or hasValue:
            ret = handleHash[app].process(val)
            self.send_response(200, "OK")
            self.send_header("Content-Type", "Text/Plain")
            self.send_header("Content-Length", str(len(ret)))
            self.end_headers()
            self.wfile.write(ret)
        else:
            self.send_response(404, "Error")
            self.send_header("Content-Type", "Text/Plain")
            self.end_headers()
            tmpstr = "Usage: http://[your_url]:" + str(port) + "/?" + str(namespace[0]) + "=" + str(namespace[0]) + "_val"
            for name in namespace[1:]:
                tmpstr = tmpstr + "&" + str(name) + "=" + str(name) + "_val"
            self.wfile.write(tmpstr)

class CHTTPApp:
    handler = None
    namespace = []
    def __init__(self, handler, namespace, *args, **kwargs):
        self.handler = handler
        self.namespace = namespace
        self.args = args
        self.kwargs = kwargs

    def process(self, input_vals):
        return self.handler(input_vals, *self.args, **self.kwargs)

handleHash = {};

def httpServerRun(handlelist, pt):
    global handleHash, port;
    handleHash = handlelist;
    port = pt;
    server_address = ('', port);
    httpd = BaseHTTPServer.HTTPServer(server_address, CAppServer);
    httpd.serve_forever();

#paralist = ["src_", "dst_"];
#def test(para):
#    return "test" + para["src_"] + para["dst_"];
# httpServerRun(test, paralist, 8002);
