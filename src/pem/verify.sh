#!/bin/bash

openssl verify -CAfile rootca.crt server.crt client.crt
