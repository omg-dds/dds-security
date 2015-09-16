# dds-security

Validation of interoperability  of products compliant with [OMG DDS-SECURITY standard](http://www.omg.org/spec/DDS-SECURITY/). This is considered one of the core [DDS Specifications](http://portals.omg.org/dds/omg-dds-standard/). See http://portals.omg.org/dds/ for an overview of DDS.

The executables in this repository test mutual autentication of each *DDS DomainParticipant*, access control to the *DDS Domain* and each specific *DDS Topic*, exchange of key material and exchange of encrypted and autenticated messages. The goal is to validate that the implementations perform these functions in compliance with [OMG DDS-SECURITY standard](http://www.omg.org/spec/DDS-SECURITY/) and can interoperate with each other.

Example execution from CWD  srcCxx:

```
ubuntu:srcCxx$ ./rti_connext_dds_5.2_linux -pub PD_RWA_OM_OD -permissions ../rti_connext_dds_certs/TESTONLY_rti_connext_dds_permissions_write_signed.p7s -governance ../TESTONLY_governance_signed.p7s

Usage:  ./rti_connext_dds_5.2_linux [-pub <pubTopic>] [-sub <subTopic>] [-domain <domainId>] [-color <colorName>] [-governance <governenceFile>]  [-permissions <permissionsFile>]

Info: "-color" unspecified. Default to: "BLACK"
Info: "-domain" unspecified. Default to: 0
AccessControl_validate_remote_permissions:successfully validated remote participant permissions
AccessControl_check_create_participant:successfully checked participant permissions
AccessControl_check_create_topic:topic not allowed: cannot be published or subscribed
PRESParticipant_createTopic:!security function check_create_topic
DDS_Topic_createI:!create presentation topic
DDSTopic_impl::createI:!create topic
create_topic error for topic 'PD_RWA_OM_OD
```
