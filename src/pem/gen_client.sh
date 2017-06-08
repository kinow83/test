#!/bin/bash

openssl genrsa -aes256 -out client.key 2048
#remove pass phrase
openssl rsa -in client.key -out client.key.without_pass
chmod 600 client.key.without_pass
openssl req -new -key client.key.without_pass -config client.conf -out client.csr
openssl x509 -req \
    -days 2000 \
    -extensions v3_user \
    -in client.csr \
    -CA rootca.crt \
    -CAcreateserial \
    -CAkey rootca.key \
    -out client.crt \
    -extfile client.conf
