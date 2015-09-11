# dds-security

Validation of interoperability  of products compliant with [OMG DDS-SECURITY standard](http://www.omg.org/spec/DDS-SECURITY/). This is considered one of the core [DDS Specifications](http://portals.omg.org/dds/omg-dds-standard/). See http://portals.omg.org/dds/ for an overview of DDS.

The executables in this repository test mutual autentication of each *DDS DomainParticipant*, access control to the *DDS Domain* and each specific *DDS Topic*, exchange of key material and exchange of encrypted and autenticated messages. The goal is to validate that the implementations perform these functions in compliance with [OMG DDS-SECURITY standard](http://www.omg.org/spec/DDS-SECURITY/) and can interoperate with each other.

