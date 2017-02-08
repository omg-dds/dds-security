These commands assume the CWD is the repository root (dds-security).

Generation of RTI Identity private key:

    openssl genrsa -out rti_connext_certs/private/TESTONLY_rti_connext_dds_identity_private_key.pem  2048

Generation of certificate request to be signed by the Identity CA:

    openssl req -config rti_connext_certs/rti_connext_dds_openssl.cnf \
      -new -key rti_connext_certs/private/TESTONLY_rti_connext_dds_identity_private_key.pem \
      -out rti_connext_certs/identity_rti_connext_dds.csr

See identity_ca_files/README.md for the generation of RTI Identity Certificate by the Identity CA (signing of certificate request).

See permissions_ca_files/README.md for the generation of RTI Permission Signed documents by the Permissions CA (signing of Permissions XML documents).
