#!/bin/bash
# USAGE: sh ./ocsp_test.sh [host] [port]
# This is a script that verifies a site's certificate using OCSP via openssl

HOST=${1:-"steelcomputers.com"} # Use first arg or steelcomputers for host
PORT=${2:-"443"}                # use second arg or 443 as port
TEMP_CERT_FILE=`mktemp -t ssl_certificate_XXX.pem`
TEMP_CERT_CHAIN_FILE=`mktemp -t ssl_certificate_chain_XXX.pem`

fileprogress() {
    if hash pv 2>/dev/null; then
	  # Use pv to display file progress if it's installed
	  cat | pv
    else
	  # Just send the data along if pv isn't installed
	  cat
    fi
}

printf "======== Downloading certificates ========\n"
echo "Saving certificate in \"`basename $TEMP_CERT_FILE`\"..."
# Download the certificate, use sed to get the pem formatted cert, display download & save
openssl s_client -connect $HOST:$PORT 2>&1 < /dev/null | sed -n '/-----BEGIN/,/-----END/p' |
	fileprogress > $TEMP_CERT_FILE

echo "Saving certificate chain in \"`basename $TEMP_CERT_CHAIN_FILE`\"..."
# sed the pem certs, perl to delete the one we have already
openssl s_client -connect $HOST:$PORT 2>&1 -showcerts < /dev/null |
	sed -n '/-----BEGIN/,/-----END/p' |
	perl -0777 -pe 's/.*?-{5}END\sCERTIFICATE-{5}\n//s'  | 
	fileprogress > $TEMP_CERT_CHAIN_FILE

printf "\n============ Locating OCSP URI ===========\n"
# Get the OCSP address from the certificate that we downloaded
SSL_OCSP_ADDRESS=`openssl x509 -noout -ocsp_uri -in $TEMP_CERT_FILE`
SSL_OCSP_HOST=`echo $SSL_OCSP_ADDRESS | awk -F/ '{print $3}'`
echo "The full OCSP address is \"$SSL_OCSP_ADDRESS\", so the host is \"$SSL_OCSP_HOST\""

printf "\n======= Performing OCSP Validation =======\n"
# Request verification on the status of the certificate where both the OCSP and CERT are signed by
#   the same certificate.  Some OCSP servers require that the host header is specified, add -text 
#   to get more detailed output
openssl ocsp -issuer $TEMP_CERT_CHAIN_FILE -cert $TEMP_CERT_FILE -url $SSL_OCSP_ADDRESS \
	-header "HOST" $SSL_OCSP_HOST -VAfile $TEMP_CERT_CHAIN_FILE

printf "\n=========== Removing temp files ==========\n"
rm -v $TEMP_CERT_FILE $TEMP_CERT_CHAIN_FILE