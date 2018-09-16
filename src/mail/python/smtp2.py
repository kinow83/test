import smtplib
import threading
import os
import sys
import json

from email import Encoders
from email import Utils
from email.header import Header
from email.message import Message
from email.mime.audio import MIMEAudio
from email.mime.base import MIMEBase
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email import Utils
from email.header import Header
from email import Charset

def dosendmail(c):
    Charset.add_charset('utf-8', Charset.QP, Charset.QP, 'utf-8')

    msg = MIMEMultipart()
    msg["From"] = c["From"]
    msg["To"] = c["To"]
    msg["Subject"] = Header(s=c["Subject"], charset="utf-8")
    msg["Date"] = Utils.formatdate(localtime=1)
    """
    msg.attach(MIMEText(c["Text"], "plain", _charset="utf-8"))
    """
    msg.attach(MIMEText(c["Text"], "plain", _charset=Charset.QP))

    rcpts = c["To"].split(",")
    if c.get("Cc") is not None:
        rcpts.extend(c["Cc"].split(","))

    if c.get("attachment") is not None:
        for attach in c["attachment"].split(", "):
            part = MIMEBase("application", "octet-stream")
            part.set_payload(open(attach, "rb").read())
            Encoders.encode_base64(part)
            part.add_header("Content-Disposition", "attachment; filename=\"%s\"" % os.path.basename(attach))
            msg.attach(part)

    try:
        smtp = smtplib.SMTP(c["Server"], int(c["Port"]))
        smtp.login(c["Userid"], c["Passwd"])
        smtp.sendmail(c["From"], rcpts, msg.as_string())
        smtp.close()
    except smtplib.SMTPException:
        smtp = smtplib.SMTP(c["Server"], int(c["Port"]))
        smtp.sendmail(c["From"], rcpts, msg.as_string())
        smtp.close()

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