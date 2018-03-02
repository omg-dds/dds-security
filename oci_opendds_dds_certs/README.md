These commands assume the CWD is the repository root (dds-security)

Generation of OCI Identity private key:

    openssl genrsa -out oci_opendds_dds_certs/private/TESTONLY_oci_opendds_dds_identity_private_key.pem 2048

Generation of certificate request to be signed by the Identity CA:

    openssl req -config oci_opendds_dds_certs/oci_opendds_dds_openssl.cnf -new -key oci_opendds_dds_certs/private/TESTONLY_oci_opendds_dds_identity_private_key.pem -out oci_opendds_dds_certs/identity_oci_opendds_dds.csr

See identity_ca_files/README.md for the generation of OCI Identity
Certificate by the Identity CA (signing of certificate request)

See permissions_ca_files/README.md for the generation of OCI Permission
Signed documents by the Permissions CA (signing of Permissions XML documents)
