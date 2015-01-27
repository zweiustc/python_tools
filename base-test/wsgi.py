from wsgiref.simple_server import make_server
import webob

def my_app(environ, start_response):
    """ a test"""
    status = '200 OK'
    resp = webob.Response(status_int=202)
    resp.content_type = 'application/json'
    resp.body = 'hello world'
    #response_headers = [('Content-type', 'text/plain')]
    response_headers = [('Content-type', resp.content_type)]
    start_response(status, response_headers)
    return resp.body
    #return resp
    
httpd = make_server('', 8000, my_app)
print "serving on port 8000"
httpd.serve_forever()
