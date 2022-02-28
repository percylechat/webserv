import telnetlib
import base64

import cv2
# raw = """POST /test/ HTTP/1.0
# From: frog@jmarshall.com
# User-Agent: HTTPTool/1.0
# Content-Type: application/x-www-form-urlencoded
# Content-Length: 32

# veme=Cosby&favorite+flavor=flies
# """
# raw_byte = str.encode(raw)


host = "127.0.0.1"
port = 8082
timeout = 100

#classic post upload
# raw = """POST / HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "test.txt"
# Content-Lenght: 22 \r\n\r
# hello bebe chat mignon
# """

#chunked encoding upload
# raw = """POST / HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les donnees du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 0\r\r\n
# """

#error method
# raw = """PUT / HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "test.txt"
# Content-Lenght: 22 \r\n\r
# hello bebe chat mignon
# """

#error content lenght
raw = """POST / HTTP/1.1
Host: 127.0.0.1:8082
Content-Type: text/plain
filename= "test.txt"\r\n\r
hello bebe chat mignon
"""

raw_byte = str.encode(raw)
with telnetlib.Telnet(host, port, timeout) as session:
    session.write(raw_byte)
    output = session.read_until(b"done", timeout)
    session.close()
    print(output)
    print("Done")