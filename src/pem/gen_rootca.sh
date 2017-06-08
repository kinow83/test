#!/bin/bash

openssl genrsa -aes256 -out rootca.key 2048
openssl req -new -key rootca.key -out rootca.csr -config rootca.conf
openssl x509 -req \
    -days 3650 \
    -extensions v3_ca \
    -set_serial 1 \
    -in rootca.csr \
    -signkey rootca.key \
    -out rootca.crt \
    -sha512 \
    -extfile rootca.conf
