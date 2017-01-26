#include "ShapeType.h"
#include "ShapeTypeSupport.h"
#include "ndds/ndds_namespace_cpp.h"
#include "security/security_default.h"

using namespace DDS;
/*
                <property>
                        <value>
                                <element>
                                </element>
                                </element>
                                <element>
                                </element>
                                <element>
                                </element>
                                <element>
                                </element>
                                <element>
                                </element>

                                <!--  Already defined in the Generic.Security -->
                                    <element>
                                      <name>com.rti.serv.load_plugin</name>
                                      <value>com.rti.serv.secure</value>
                                    </element>
                                    <!-- Security can be enabled via XML with dynamic linking. If you
                                         have a C or C++ application that is statically linked, please
                                         refer to the Hello_security examples for instructions on how
                                         to enable security.-->
                                    <element>
                                      <name>com.rti.serv.secure.library</name>
                                      <value>rtisecurity</value>
                                    </element>
                                    <element>
                                      <name>com.rti.serv.secure.create_function</name>
                                      <value>SecurityPluginSuite_create</value>
                                    </element>
                        </value>
                </property>


 */
class ShapeTypeConfigurator {
    public:
    static DomainParticipant *create_participant(
            int domain_id,
            const char *governance_file,
            const char *permissions_file )
    {
        DomainParticipant *participant = NULL;

        DDS_DomainParticipantQos pQos;
        TheParticipantFactory->get_participant_qos_from_profile(pQos, "BuiltinQosLib", "Generic.Security");
        //get_default_participant_qos(pQos);

        pQos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;

        DDS_PropertyQosPolicyHelper_assert_pointer_property(&pQos.property,
                 RTI_SECURITY_BUILTIN_PLUGIN_PROPERTY_NAME ".create_function_ptr",
                 (void *) RTI_Security_PluginSuite_create);

        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                 "com.rti.serv.secure.logging.log_level",
                 "4", DDS_BOOLEAN_FALSE);

        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                governance_file, DDS_BOOLEAN_FALSE);
        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                "com.rti.serv.secure.access_control.permissions_file",
                permissions_file, DDS_BOOLEAN_FALSE);

        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                "com.rti.serv.secure.authentication.ca_file",
                "../TESTONLY_identity_ca_cert.pem", DDS_BOOLEAN_FALSE);

        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                "com.rti.serv.secure.access_control.permissions_authority_file",
                "../TESTONLY_permissions_ca_cert.pem", DDS_BOOLEAN_FALSE);

        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                "com.rti.serv.secure.authentication.certificate_file",
                "../rti_connext_dds_certs/TESTONLY_rti_connext_dds_identity_cert.pem", DDS_BOOLEAN_FALSE);

        DDS_PropertyQosPolicyHelper_assert_property(&pQos.property,
                "com.rti.serv.secure.authentication.private_key_file",
                "../rti_connext_dds_certs/private/TESTONLY_rti_connext_dds_identity_private_key.pem", DDS_BOOLEAN_FALSE);



        participant = TheParticipantFactory->create_participant(
            domain_id, pQos,
            NULL /* listener */, DDS_STATUS_MASK_NONE);
        if ( participant == NULL ) {
            printf("create_participant error for domain_id '%d'\n", domain_id);
            return NULL;
        }

        return participant;
    }
};
