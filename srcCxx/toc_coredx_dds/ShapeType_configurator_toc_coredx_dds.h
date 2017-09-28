#include <dds/dds.hh>
#include <dds/dds_security.h>
#include <dds/coredx_sec_plugins.h>

#include "ShapeType.hh"
#include "ShapeTypeTypeSupport.hh"
namespace DDS {
  static const bool BOOLEAN_TRUE  = true;
  static const bool BOOLEAN_FALSE = false;
  static const Duration_t DURATION_INFINITE(DDS_DURATION_INFINITE_SEC, DDS_DURATION_INFINITE_NSEC);
};

#define DDSSEC_PROP_IDENTITY_CA        "dds.sec.auth.identity_ca"
#define DDSSEC_PROP_IDENTITY_CERT      "dds.sec.auth.identity_certificate"
#define DDSSEC_PROP_IDENTITY_PRIVKEY   "dds.sec.auth.private_key"
#define DDSSEC_PROP_IDENTITY_PASSWORD  "dds.sec.auth.password"

#define DDSSEC_PROP_PERM_CA            "dds.sec.access.permissions_ca"
#define DDSSEC_PROP_PERM_GOV_DOC       "dds.sec.access.governance"
#define DDSSEC_PROP_PERM_DOC           "dds.sec.access.permissions"

class ShapeTypeConfigurator {
 public:

  /********************************************************************
   *
   ********************************************************************/
  static char *cpp_strdup(const char * s)
  {
    char * retval = NULL;
    if (s)
      {
        retval = new char[strlen(s)+1];
        if (retval)
          strcpy(retval, s);
      }
    return retval;
  }
  
  /********************************************************************
   *
   ********************************************************************/
  static void set_property(DDS::PropertySeq * properties, const char * name, const char * value)
  {
    DDS::Property_t prop;
    prop.name      = cpp_strdup(name);
    prop.value     = cpp_strdup(value);
    prop.propagate = 0;
    properties->push_back( prop ); /* shallow copy */
  }
 
  
  /********************************************************************
   *
   ********************************************************************/
  static DomainParticipant *
    create_participant(int         domain_id,
                       bool        use_security,
                       const char *governance_uri,
                       const char *permissions_uri)
  {
    static const char *auth_ca_file  = "file:../TESTONLY_identity_ca_cert.pem";
    static const char *perm_ca_file  = "file:../TESTONLY_permissions_ca_cert.pem";
    static const char *id_cert_file  = "file:../toc_coredx_dds_certs/TESTONLY_toc_coredx_dds_identity_cert.pem";
    static const char *id_key_file   = "file:../toc_coredx_dds_certs/private/TESTONLY_twinoaks_coredx_identity_private_key.pem";

    /* 
       printf("GOVERNANCE_FILE:  %s\n", governance_uri);
       printf("PERMISSIONS_FILE: %s\n", permissions_uri);
    */
    
    DDS::DomainParticipantFactory      * dpf          = DomainParticipantFactory::get_instance();
    DDS::DomainParticipant             * participant = NULL;
    DDS::DomainParticipantQos            dp_qos;
    
    dpf->get_default_participant_qos(dp_qos);

    if ( use_security )
      {
        DDS::PropertySeq * properties = &dp_qos.properties.value;
    
        /* AUTHENTICATION: */
        set_property( properties, DDSSEC_PROP_IDENTITY_CA,      auth_ca_file );
        set_property( properties, DDSSEC_PROP_IDENTITY_CERT,    id_cert_file );
        set_property( properties, DDSSEC_PROP_IDENTITY_PRIVKEY, id_key_file );
        /* (optional) passphrase to acces 'private key' */
        // set_property(properties, DDSSEC_PROP_IDENTITY_PASSWORD, "data:,thereisnopassword");

        /* ACCESS CONTROL: */
        set_property( properties, DDSSEC_PROP_PERM_CA,       perm_ca_file );
        set_property( properties, DDSSEC_PROP_PERM_GOV_DOC,  governance_uri );
        set_property( properties, DDSSEC_PROP_PERM_DOC,      permissions_uri );

        /* use DynamicLoad to get function */
        set_property(properties,
                     "com.toc.sec.create_plugins",
                     // ":DSREF_create_plugins");                    /* this loads from 'main' executable */
                     "dds_security_log:DSREF_create_plugins"); /* this loads from dynamic library 'dds_security_log' */

      }

    participant = dpf->create_participant(domain_id, dp_qos,
                                          NULL, STATUS_MASK_NONE);
    if ( participant == NULL ) 
      {
        printf("create_participant error for domain_id '%d'\n", domain_id);
      }

    return participant;
  }
 
    
  static void destroy_participant( DomainParticipant * dp )
  {
    DDS::DomainParticipantFactory      * dpf      = DomainParticipantFactory::get_instance();
    if (dp) dp->delete_contained_entities();
    if (dp) dpf->delete_participant(dp);
  }

};
