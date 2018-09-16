import smtplib
import threading
import os
import sys
import json
import mimetypes

from email.header import Header
from email.message import EmailMessage
from email.mime.text import MIMEText
from email.headerregistry import Address
from ssl import SSLContext, PROTOCOL_TLSv1_2
from email.utils import make_msgid

def dosendmail(c):
    msg = EmailMessage()
    msg["From"] = Address(display_name=c["From"], addr_spec=c["From"])
    msg["To"] = Address(display_name=c["To"], addr_spec=c["To"])
    msg["Subject"] = c["Subject"]
    msg.set_content(c["Text"])

    asparagus_cid = make_msgid()
    if c.get("attachment") is not None:
        for attach in c["attachment"].split(", "):

            ctype, encoding = mimetypes.guess_type(attach)
            if ctype is None or encoding is not None:
                # No guess could be made, or the file is encoded (compressed), so use a generic bag-of-bits type.
                ctype = 'application/octet-stream'
            maintype, subtype = ctype.split('/', 1)
            with open(attach, 'rb') as fp:
                msg.add_attachment(fp.read(),
                                   maintype=maintype,
                                   subtype=subtype,
                                   filename=os.path.basename(attach))
            maintype, subtype = ctype.split('/', 1)

    with smtplib.SMTP(c["Server"]) as smtp_server:
        try:
            smtp_server.send_message(msg)
            smtp_server.close()
        except Exception as e:
            smtp_server.close()

    print("send mail --- OK ({0} -> {1})".format(msg["From"], msg["To"]))

def dowork(config) :
    m = {}
    c = {}
    threads = []

    for key1 in config.keys():
        if "common" != key1:
            c = {}
            c["Server"] = config["common"]["Server"]
            c["Port"] = config["common"]["Port"]
            for key2 in config[key1]:
                c[key2] = config[key1][key2]

            t = threading.Thread(target=dosendmail, args=(c,))
            t.daemon = True
            threads.append(t)

    for i in range(len(threads)):
        threads[i].start()

    for i in range(len(threads)):
        threads[i].join()

if __name__ == '__main__':
    with open(sys.argv[1]) as f:
        c = json.load(f)
        dowork(c)
