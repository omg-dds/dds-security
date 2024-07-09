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

#define CDXSEC_PROP_SHARED_SECRET_ALGO "com.toc.sec.shared_secret_algorithm"


static const char *get_qos_policy_name(DDS_QosPolicyId_t policy_id)
{
  return DDS_qos_policy_str(policy_id); // not standard...
}


class MyListenerClass : public DDS::DomainParticipantListener
  {
  public:
    MyListenerClass() {};
    ~MyListenerClass() {};

    virtual void on_inconsistent_topic( Topic                          * topic,
					const InconsistentTopicStatus  & /* status    */)
    {
      const char *topic_name = topic->get_name();
      const char *type_name  = topic->get_type_name();
      printf("%s() topic: '%s'  type: '%s'\n", __FUNCTION__, topic_name, type_name);
    }

    // ---
    
    virtual void on_offered_deadline_missed ( DataWriter                        * dw, 
					      const OfferedDeadlineMissedStatus & status )
    {
      Topic      *topic      = dw->get_topic( );
      const char *topic_name = topic->get_name( );
      const char *type_name  = topic->get_type_name( );
      printf("%s() topic: '%s'  type: '%s' : (total = %d, change = %d)\n", __FUNCTION__,
             topic_name, type_name, status.total_count, status.total_count_change);
    }
    
    virtual void on_offered_incompatible_qos( DataWriter                         * dw,
					      const OfferedIncompatibleQosStatus & status )
    {
      Topic      *topic       = dw->get_topic( );
      const char *topic_name  = topic->get_name( );
      const char *type_name   = topic->get_type_name( );
      const char *policy_name = NULL;
      policy_name = get_qos_policy_name(status.last_policy_id);
      printf("%s() topic: '%s'  type: '%s' : %d (%s)\n", __FUNCTION__,
             topic_name, type_name,
             status.last_policy_id,
             policy_name );
    }
    
    virtual void on_liveliness_lost         ( DataWriter                       * dw, 
					      const LivelinessLostStatus       & status )
    {
      Topic      *topic      = dw->get_topic( );
      const char *topic_name = topic->get_name( );
      const char *type_name  = topic->get_type_name( );
      printf("%s() topic: '%s'  type: '%s' : (total = %d, change = %d)\n", __FUNCTION__,
             topic_name, type_name, status.total_count, status.total_count_change);
    }
    
    virtual void on_publication_matched     ( DataWriter                        * dw,
					      const PublicationMatchedStatus    & status )
    {
      Topic      *topic      = dw->get_topic( );
      const char *topic_name = topic->get_name( );
      const char *type_name  = topic->get_type_name( );
      printf("%s() topic: '%s'  type: '%s' : matched readers %d (change = %d)\n", __FUNCTION__,
             topic_name, type_name, status.current_count, status.current_count_change);
    }

    // ---

    virtual void on_requested_deadline_missed ( DataReader                          * dr,
						const RequestedDeadlineMissedStatus & status )
    {
        TopicDescription *td         = dr->get_topicdescription( );
        const char       *topic_name = td->get_name( );
        const char       *type_name  = td->get_type_name( );
        printf("%s() topic: '%s'  type: '%s' : (total = %d, change = %d)\n", __FUNCTION__,
                topic_name, type_name, status.total_count, status.total_count_change);
    }
    virtual void on_requested_incompatible_qos( DataReader                           * dr, 
						const RequestedIncompatibleQosStatus & status )
    {
      TopicDescription *td         = dr->get_topicdescription( );
      const char       *topic_name = td->get_name( );
      const char       *type_name  = td->get_type_name( );
      const char *policy_name = NULL;
      policy_name = get_qos_policy_name(status.last_policy_id);
      printf("%s() topic: '%s'  type: '%s' : %d (%s)\n", __FUNCTION__,
             topic_name, type_name, status.last_policy_id,
             policy_name);
    }

    virtual void on_liveliness_changed        ( DataReader                    * dr,
						const LivelinessChangedStatus & status )
    {
      TopicDescription *td         = dr->get_topicdescription( );
      const char       *topic_name = td->get_name( );
      const char       *type_name  = td->get_type_name( );
      printf("%s() topic: '%s'  type: '%s' : (alive = %d, not_alive = %d)\n", __FUNCTION__,
             topic_name, type_name, status.alive_count, status.not_alive_count);
    }
    
    virtual void on_subscription_matched      ( DataReader                       * dr, 
						const SubscriptionMatchedStatus  & status )
    {
      TopicDescription *td         = dr->get_topicdescription( );
      const char       *topic_name = td->get_name( );
      const char       *type_name  = td->get_type_name( );
      printf("%s() topic: '%s'  type: '%s' : matched writers %d (change = %d)\n", __FUNCTION__,
             topic_name, type_name, status.current_count, status.current_count_change);
    }

    virtual void on_data_available            ( DataReader                   * /* the_reader */)
    {
    } 
    virtual void on_sample_rejected           ( DataReader                   * /* the_reader */, 
						const SampleRejectedStatus   & /* status     */)
    {
    }
    virtual void on_sample_lost               ( DataReader                   * /* the_reader */, 
						const SampleLostStatus       & /* status     */)
    {
    }
    virtual void on_data_on_readers( Subscriber *  /* the_subscriber */ )
    {
    }
    
  };
  
static MyListenerClass dpListener;

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
                       const char *permissions_uri,
                       const char *key_establishment_algo,
                       bool        enable_logging)
  {
    static const char *auth_ca_file  = "file:../TESTONLY_identity_ca_cert.pem";
    static const char *perm_ca_file  = "file:../TESTONLY_permissions_ca_cert.pem";
#if defined( CDX_ALT_CERT ) && ( CDX_ALT_CERT )
    static const char *id_cert_file  = "file:../toc_coredx_dds_alt_certs/TESTONLY_toc_coredx_dds_identity_cert.pem";
    static const char *id_key_file   = "file:../toc_coredx_dds_alt_certs/private/TESTONLY_twinoaks_coredx_identity_private_key.pem";
#else    
    static const char *id_cert_file  = "file:../toc_coredx_dds_certs/TESTONLY_toc_coredx_dds_identity_cert.pem";
    static const char *id_key_file   = "file:../toc_coredx_dds_certs/private/TESTONLY_twinoaks_coredx_identity_private_key.pem";
#endif
    
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
        
        /* AUTH DH algo: */
        set_property( properties, CDXSEC_PROP_SHARED_SECRET_ALGO, key_establishment_algo );
        
        /* ACCESS CONTROL: */
        set_property( properties, DDSSEC_PROP_PERM_CA,       perm_ca_file );
        set_property( properties, DDSSEC_PROP_PERM_GOV_DOC,  governance_uri );
        set_property( properties, DDSSEC_PROP_PERM_DOC,      permissions_uri );
        
        /* PLUGIN CREATION: */
        set_property(properties,
                     "com.toc.sec.create_plugins",
                     ":DSREF_create_plugins");                    /* this loads from 'main' executable */
                     // "dds_security_log:DSREF_create_plugins"); /* this loads from dynamic library 'dds_security_log' */

        if (enable_logging)
          {
            /* defaults are just fine: very similar to this, but writes to a file in /tmp/ */
            /* SECURITY LOGGING: */
            char buf[16];
            sprintf(buf, "%d", DDS_Security_NOTICE_LEVEL );
            set_property( properties, "com.toc.sec.log_level", buf );
            set_property( properties, "com.toc.sec.log_file", "./toc_coredx_security_log.txt" );
            sprintf(buf, "%d", 0);
            set_property( properties, "com.toc.sec.log_publish", buf );
          }

      }

    participant = dpf->create_participant(domain_id, dp_qos,
                                          &dpListener, DDS_STATUS_MASK_ALL );
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
