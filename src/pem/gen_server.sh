#!/bin/bash

openssl genrsa -aes256 -out server.key 2048
#remove pass phrase
openssl rsa -in server.key -out server.key.without_pass
chmod 600 server.key.without_pass
openssl req -new -key server.key.without_pass -config server.conf -out server.csr
openssl x509 -req \
    -days 2000 \
    -extensions v3_server \
    -in server.csr \
    -CA rootca.crt \
    -CAcreateserial \
    -CAkey rootca.key \
    -out server.crt \
    -extfile server.conf
