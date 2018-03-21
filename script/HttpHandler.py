# -*- coding: utf-8 -*-

import httplib
import mimetools
import mimetypes

def GetHttp(address, port, url):
    conn = httplib.HTTPConnection(address, port)
    conn.request("GET", url)
    rsp = conn.getresponse()
    status = rsp.status
    data = rsp.read()

    conn.close()
    return status, data


def encode_multipart_formdata(fields, files=None):
    """
    fields is a sequence of (name, value) elements for regular form fields.
    files is a sequence of (name, filename, value) elements for data to be uploaded as files
    Return (content_type, body) ready for httplib.HTTP instance
    """
    BOUNDARY = mimetools.choose_boundary()
    CRLF = '\r\n'
    L = []
    for (key, value) in fields:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"' % key)
        L.append('')
        L.append(value)
    if files is not None: #if files != None:
        for (key, filename, value) in files:
            L.append('--' + BOUNDARY)
            L.append('Content-Disposition: form-data; name="%s"; filename="%s"' % (key, filename))
            L.append('Content-Type: %s' % get_content_type(filename))
            L.append('')
            L.append(value)
    L.append('--' + BOUNDARY + '--')
    L.append('')
    body = CRLF.join(L)
    content_type = 'multipart/form-data; boundary=%s' % BOUNDARY
    return content_type, body


def get_content_type(filename):
    return mimetypes.guess_type(filename)[0] or 'application/octet-stream'


def PostHttp(address, port, url, fields, files=None):
    content_type, body = encode_multipart_formdata(fields, files)

    conn = httplib.HTTPConnection(address, port)
    headers = {'content-type': content_type}

    conn.request('POST', url, body, headers)
    rsp = conn.getresponse()

    status = rsp.status
    data = rsp.read()

    conn.close()
    return status, data




if __name__ == "__main__":
    url = '/access.cgi?action=user_query_access_domain'
    address = 'www.xvripc.net'
    port = 8888

    print GetHttp(address=address, port=port, url=url)

    url = '/access.cgi?action=device_query_access_domain'
    values = [('devid', 'C065414E55532D3030303134372D4A46')]

    print PostHttp(address=address, port= port, url=url, fields=values)

