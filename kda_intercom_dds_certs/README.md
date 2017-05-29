These commands assume the CWD is the repository root (dds-security)

Generation of private key and certificate request to be signed by the Identity CA. The InterCOM specific ShapeType
header file assumes the password "intercom" on the private key:

    openssl req -config identity_ca_files/identity_ca_openssl.cnf -newkey rsa:2048 -keyout kda_intercom_dds_certs/private/TESTONLY_kda_intercom_dds_identity_cert.key.pem -new -out kda_intercom_dds_certs/identity_kda_intercom_dds.csr -subj "/CN=kda_intercom_dds/O=Kongsberg Defence and Aerospace/C=NO"

See identity_ca_files/README.md for the generation of Kongsberg Identity
Certificate by the Identity CA (signing of certificate request)

See permissions_ca_files/README.md for the generation of Kognsberg Permission
Signed documents by the Permissions CA (signing of Permissions XML documents)
