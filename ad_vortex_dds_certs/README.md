These commands assume the CWD is the repository root (dds-security).

Generation of ADLINK Identity private key:

    openssl genrsa -out ad_vortex_dds_certs/private/TESTONLY_ad_vortex_dds_identity_private_key.pem 2048

Generation of certificate request to be signed by the Identity CA:

    openssl req -config ad_vortex_dds_certs/ad_vortex_dds_openssl.cnf \
      -new -key ad_vortex_dds_certs/private/TESTONLY_ad_vortex_dds_identity_private_key.pem \
      -out ad_vortex_dds_certs/identity_ad_vortex_dds.csr

See identity_ca_files/README.md for the generation of ADLINK Identity Certificate by the Identity CA (signing of certificate request).

See permissions_ca_files/README.md for the generation of ADLINK Permission Signed documents by the Permissions CA (signing of Permissions XML documents).
