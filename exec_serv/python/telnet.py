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
# Content-Lenght: 357 \r\n\r
# hello bebe chat mignon*****************************************************************************************************************
# *********************************************************************************************************************************************
# *******************************************************************************
# """
# 413 PLAYLOAD TOO LARGE ERROR PAGE FOUND IN CONF FILE

# raw = """POST /ugly_cat/chat.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 201 CREATED

# raw = """GET /ugly_cat/chat.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 200 OK
# OK *

# raw = """GET / HTTP/1.0
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 505 HTTP VERSION NOT SUPPORTED - ERROR PAGE FOUND IN CONF FILE
# OK *

# raw = """GET / HTTP/1.0
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 505 HTTP VERSION NOT SUPPORTED - ERROR PAGE NOT FOUND IN CONF FILE
# OK *

# raw = """GET /ugly_cat/graphic_design.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 200 OK
# OK *

# raw = """GET index.html HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 200 OK
# OK *

# raw = """GET /ugly_cat/unknown.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 404 NOT FOUND - ERROR PAGE FOUND IN CONF FILE
# OK *

# raw = """GET /ugly_cat/unknown.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 404 NOT FOUND - ERROR PAGE NOT FOUND IN CONF FILE
# OK *

raw = """GET /ugly_cat/percy.jpg HTTP/1.1
Content-Type: text/plain
Content-Length: 1000
filename= "hello.txt"
Transfer-Encoding: chunked\r\n\r
26\r\nVoici les données du premier morceau\r\n\r
1C\ret voici un second morceau\r\n\r\n
20\ret voici deux derniers morceaux \r\n
12\r\sans saut de ligne\r\n
0\r\r\n
"""
# 400 BAD REQUEST - ERROR PAGE FOUND IN CONF FILE
# OK *

# raw = """GET /ugly_cat/percy.jpg HTTP/1.1
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 400 BAD REQUEST - ERROR PAGE PATH INCORRECT IN CONF FILE
# OK *

# raw = """PUT /index.html HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 405 METHOD NOT ALLOWED - ERROR PAGE FOUND IN CONF FILE
# OK *

# raw = """RANDOMMETHOD /index.html HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 405 METHOD NOT ALLOWED - ERROR PAGE PATH INCORRECT IN CONF FILE
# OK *

# raw = """DELETE ugly_cat HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 403 FORBIDDEN - ERROR PAGE FOUND IN CONF FILE
# OK * - CHANGE i != 1 BY i > 1

# raw = """DELETE /ugly_cat/ HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 500 INTERNAL SERVER ERROR - ERROR PAGE NOT FOUND IN CONF FILE
# OK *

# raw = """DELETE /ugly_cat/mdr.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 500 INTERNAL SERVER ERROR - ERROR PAGE FOUND IN CONF FILE
# OK *

# raw = """DELETE /ugly_cat/mdr.jpg HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """
# 500 INTERNAL SERVER ERROR - ERROR PAGE NOT FOUND IN CONF FILE
# OK *

# PREVIOUS OOR
# raw = """GET index.html HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n
# """

# raw = """RANDOMMETHOD /index.html HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# Content-Length: 1000
# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
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
# raw = """POST / HTTP/1.1
# Host: 127.0.0.1:8082
# Content-Type: text/plain
# filename= "test.txt"\r\n\r
# hello bebe chat mignon

# filename= "hello.txt"
# Transfer-Encoding: chunked\r\n\r
# 26\r\nVoici les données du premier morceau\r\n\r
# 1C\ret voici un second morceau\r\n\r\n
# 20\ret voici deux derniers morceaux \r\n
# 12\r\sans saut de ligne\r\n
# 0\r\r\n

raw_byte = str.encode(raw)
with telnetlib.Telnet(host, port, timeout) as session:
    session.write(raw_byte)
    output = session.read_until(b"done", timeout)
    session.close()
    print(output)
    print("Done")