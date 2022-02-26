#!/usr/bin/python3

import cgi, os
import cgitb

cgitb.enable()


def isSSL():
    """Return true if we are on an SSL (https) connection."""
    return os.environ.get("SSL_PROTOCOL", "") != ""


def getScriptname():
    """Return the scriptname part of the URL ("/path/to/my.cgi")."""
    return os.environ.get("SCRIPT_NAME", "")


def getPathinfo():
    """Return the remaining part of the URL."""
    pathinfo = os.environ.get("PATH_INFO", "")

    # Fix for a well-known bug in IIS/4.0
    if os.name == "nt":
        scriptname = getScriptname()
        if string.find(pathinfo, scriptname) == 0:
            pathinfo = pathinfo[len(scriptname) :]

    return pathinfo


def getQualifiedURL(uri=None):
    """Return a full URL starting with schema, servername, and port.
    Specifying uri causes it to be appended to the server root URL (uri must
    start with a slash).
    """
    schema, stdport = (("http", "80"), ("https", "443"))[isSSL()]
    host = os.environ.get("HTTP_HOST", "")
    if not host:
        host = os.environ.get("SERVER_NAME", "localhost (default)")
        port = os.environ.get("SERVER_PORT", "80 (default)")
        if port != stdport:
            host = host + ":" + port

    result = "%s://%s" % (schema, host)
    if uri:
        result = result + uri

    return result


def getBaseURL():
    """Return a fully qualified URL to this script."""
    return getQualifiedURL(getScriptname())


print("Content-type: text/html")
print()  # Pas oublier cette putain de ligne vide
form = cgi.FieldStorage()
name = None
var_dict = {
    "ssl_var": isSSL(),
    "scriptname_var": getScriptname(),
    "pathinfo_var": getPathinfo(),
    "url_var": getQualifiedURL(),
}
if "name" in form.keys():
    name = form["name"].value
print(
    """<html>  <head>
    <title></title>
    <meta content="">
    <style></style>
  </head><body>"""
)
print("name from form")
print("<br/>")
print("<br/>")
print(name)
print("<br/>")
print("<br/>")
for key in var_dict:
    print(key)
    print("<br/>")
    print("<br/>")
    print(var_dict[key])
    print("<br/>")
    print("<br/>")

print("toto")
print("</body></html>")
