These commands assume the CWD is the repository root (dds-security).

Generation of eProsima Identity private key:

    openssl genrsa -out eprosima_fastrtps_certs/private/TESTONLY_eprosima_fastrtps_identity_private_key.pem  2048

Generation of certificate request to be signed by the Identity CA:

    openssl req -config eprosima_fastrtps_certs/eprosima_fastrtps_openssl.cnf \
      -new -key eprosima_fastrtps_certs/private/TESTONLY_eprosima_fastrtps_identity_private_key.pem \
      -out eprosima_fastrtps_certs/identity_eprosima_fastrtps.csr

See identity_ca_files/README.md for the generation of eProsima Identity Certificate by the Identity CA (signing of certificate request).

See permissions_ca_files/README.md for the generation of eProsima Permission Signed documents by the Permissions CA (signing of Permissions XML documents).