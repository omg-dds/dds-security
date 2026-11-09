These commands assume the CWD is the repository root (dds-security).

Generation of ZeroDDS Identity private key:

    openssl genrsa -out zerodds_certs/private/TESTONLY_zerodds_identity_private_key.pem 2048

Generation of certificate request to be signed by the Identity CA:

    openssl req -config zerodds_certs/zerodds_openssl.cnf \
      -new -key zerodds_certs/private/TESTONLY_zerodds_identity_private_key.pem \
      -out zerodds_certs/identity_zerodds.csr

See identity_ca_files/README.md for the generation of ZeroDDS Identity Certificate by the Identity CA (signing of certificate request).

See permissions_ca_files/README.md for the generation of ZeroDDS Permission Signed documents by the Permissions CA (signing of Permissions XML documents).
