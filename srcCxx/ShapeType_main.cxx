/*
 * This application can publish, subscribe, or publish and subscribe a single Topic
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <iostream>

#if defined(RTI_CONNEXT_DDS)
#include "rti_connext_dds/ShapeType_configurator_rti_connext_dds.h"
#elif defined(TWINOAKS_COREDX)
#include "toc_coredx_dds/ShapeType_configurator_toc_coredx_dds.h"
#elif defined(INTERCOM_DDS)
#include "intercom_dds/ShapeType_configurator_intercom_dds.h"
#elif defined(OPENDDS)
#include "opendds/ShapeType_configurator_opendds.h"
#else
#include "ShapeType_configurator_default.h"
#endif

#ifndef STRING_IN
#define STRING_IN
#endif
#ifndef STRING_INOUT
#define STRING_INOUT
#endif

using namespace DDS;

const char *DEFAULT_GOVERNANCE_FILE     = "TESTONLY_governance_signed.p7s";
const char *DEFAULT_PERMISSIONS_FILE    = "TESTONLY_permissions_signed.p7s";
const char *DEFAULT_PUBLISH_TOPIC       = "OD_OA_OM_OD";
const char *DEFAULT_SUBSCRIBE_TOPIC     = "OD_OA_OM_OD";
const char *DEFAULT_KEY_EST_ALGORITHM   = "ECDHE-CEUM+P256";
const char *DEFAULT_COLOR               = "BLACK";
const float DEFAULT_LIVELINESS_PERIOD   = 0.0;
const int   DEFAULT_DOMAIN_ID           = 0;

// Keeps main thread iterating until a signal changes its value to false
bool exit_application = false;
GuardCondition *exit_guard = NULL;

/*************************************************************/
// Signal handler changes the sets exit_service to true.
void signal_handler(int)
{
    exit_application = true;
    if ( exit_guard != NULL ) {
        exit_guard->set_trigger_value(BOOLEAN_TRUE);
    }
}

/*************************************************************/
// Assign all the exit signals to signal handler
void setup_signal_handler()
{
#ifndef _WIN32
    signal(SIGHUP,  signal_handler); //Terminal is closed
    signal(SIGQUIT, signal_handler); //Quit
#endif
    signal(SIGTERM, signal_handler); //Terminate
    signal(SIGINT,  signal_handler); //Interrupt
    signal(SIGABRT, signal_handler); //Abort
}

/*************************************************************/
enum Verbosity
{
    ERROR=1,
    DEBUG=2,
};

class Logger{
public:
    explicit Logger(enum Verbosity v)
    {
        verbosity_ = v;
    }

    void verbosity(enum Verbosity v)
    {
        verbosity_ = v;
    }

    enum Verbosity verbosity()
    {
        return verbosity_;
    }

    void log_message(std::string message, enum Verbosity level_verbosity)
    {
        if (level_verbosity <= verbosity_) {
            std::cout << message << std::endl;
        }
    }

private:
    enum Verbosity verbosity_;
};

/*************************************************************/
Logger logger(ERROR);


/*************************************************************/
Topic *create_topic(DomainParticipant *participant, const char *topic_name)
{
    printf("Create topic: %s\n", topic_name );

    /* Register the type before creating the topic */
#ifdef INTERCOM_DDS
    const ShapeTypeTypeSupport* typeSupport = ShapeTypeTypeSupport::get_instance();
    const char *type_name = typeSupport->get_type_name();
    ReturnCode_t retcode = typeSupport->register_type(participant, type_name);
#elif defined OPENDDS
    org::omg::dds::demo::ShapeTypeTypeSupport_var ts =
      new org::omg::dds::demo::ShapeTypeTypeSupportImpl;
    const char type_name[] = "ShapeType";
    const ReturnCode_t retcode = ts->register_type(participant, type_name);
#else
    const char *type_name = ShapeTypeTypeSupport::get_type_name();
    ReturnCode_t retcode = ShapeTypeTypeSupport::register_type(
        participant, type_name);
#endif
    if (retcode != RETCODE_OK) {
        printf("register_type error %d\n", (int)retcode);
        return NULL;
    }

    Topic *topic = participant->create_topic(topic_name, type_name,
            TOPIC_QOS_DEFAULT, NULL /* listener */, STATUS_MASK_NONE);
    if ( topic == NULL ) {
        logger.log_message("failed to create topic: " + std::string(topic_name), Verbosity::ERROR);
        return NULL;
    }

    return topic;
}

/*************************************************************/
ShapeTypeDataWriter *create_writer(DomainParticipant *participant,
                                   Topic *topic,
                                   const char *partition,
                                   float livelinessPeriod,
                                   const char *color )

{
    logger.log_message("Running create_writer() function", Verbosity::DEBUG);
    PublisherQos pub_qos;
    participant->get_default_publisher_qos( pub_qos );

    if (partition)
      {
        char * pname;
        pname = new char[strlen(partition)+1];
        if (pname)
          {
            strcpy(pname, partition);
#if defined(OPENDDS) || defined(RTI_CONNEXT_DDS)
            const unsigned int i = pub_qos.partition.name.length();
            pub_qos.partition.name.length(i + 1);
            pub_qos.partition.name[i] = pname;
#else
            pub_qos.partition.name.push_back(pname);
#endif
          }
        else
          {
            logger.log_message("error setting partition (oom)", Verbosity::ERROR);
            return NULL;
          }
      }

    Publisher *publisher = participant->create_publisher( pub_qos, NULL /* listener */, STATUS_MASK_NONE);
    if (publisher == NULL) {
        logger.log_message("failed to create publisher", Verbosity::ERROR);
        return NULL;
    }

    DataWriterQos dw_qos;
    publisher->get_default_datawriter_qos( dw_qos );
    if (livelinessPeriod > 0.0)
      {
        dw_qos.liveliness.lease_duration.sec     = (int)livelinessPeriod;
        dw_qos.liveliness.lease_duration.nanosec = (livelinessPeriod - (int)livelinessPeriod) * 1000000000; // NSEC_PER_SEC;
      }

    printf("Create writer for topic: %s color: %s\n", topic->get_name(), color );
    DataWriter *writer = publisher->create_datawriter(
            topic, dw_qos, NULL /* listener */, STATUS_MASK_NONE);
    if (writer == NULL) {
        logger.log_message("failed to create datawriter " + std::string(topic->get_name()), Verbosity::ERROR);
        return NULL;
    }
#if defined INTERCOM_DDS || defined OPENDDS
    ShapeTypeDataWriter *shape_writer = ShapeTypeDataWriter::_narrow(writer);
#else
    ShapeTypeDataWriter *shape_writer = ShapeTypeDataWriter::narrow(writer);
#endif
    if (shape_writer == NULL) {
        printf("ShapeTypeDataWriter narrow error\n");
        return NULL;
    }
    return shape_writer;
}

/*************************************************************/
ShapeTypeDataReader *create_reader(DomainParticipant *participant,
                                   Topic *topic,
                                   const char * partition,
                                   float livelinessPeriod)

{
    logger.log_message("Running create_reader() function", Verbosity::DEBUG);
    SubscriberQos sub_qos;
    participant->get_default_subscriber_qos( sub_qos );

    if (partition)
      {
        char * pname;
        pname = new char[strlen(partition)+1];
        if (pname)
          {
            strcpy(pname, partition);
#if defined(OPENDDS) || defined(RTI_CONNEXT_DDS)
            const unsigned int i = sub_qos.partition.name.length();
            sub_qos.partition.name.length(i + 1);
            sub_qos.partition.name[i] = pname;
#else
            sub_qos.partition.name.push_back(pname);
#endif
          }
        else
          {
            logger.log_message("error setting partition (oom)", Verbosity::ERROR);
            return NULL;
          }
      }

    Subscriber *subscriber = participant->create_subscriber( sub_qos, NULL /* listener */, STATUS_MASK_NONE);
    if (subscriber == NULL) {
        logger.log_message("failed to create subscriber", Verbosity::ERROR);
        return NULL;
    }

    DataReaderQos dr_qos;
    subscriber->get_default_datareader_qos( dr_qos );
    if (livelinessPeriod > 0.0)
      {
        dr_qos.liveliness.lease_duration.sec     = (int)livelinessPeriod;
        dr_qos.liveliness.lease_duration.nanosec = (livelinessPeriod - (int)livelinessPeriod) * 1000000000; // NSEC_PER_SEC;
      }

    printf("Create reader for topic: %s\n", topic->get_name() );
    DataReader *reader = subscriber->create_datareader(
            topic, dr_qos, NULL /* listener */, STATUS_MASK_NONE);
    if (reader == NULL) {
        logger.log_message("failed to create datareader " + std::string(topic->get_name()), Verbosity::ERROR);
        return NULL;
    }
#if defined INTERCOM_DDS || defined OPENDDS
    ShapeTypeDataReader *shape_reader = ShapeTypeDataReader::_narrow(reader);
#else
    ShapeTypeDataReader *shape_reader = ShapeTypeDataReader::narrow(reader);
#endif
    if (shape_reader == NULL) {
        logger.log_message("ShapeTypeDataReader narrow error", Verbosity::ERROR);
        return NULL;
    }
    return shape_reader;
}


/*************************************************************/
int run(DomainId_t domain_id, bool use_security,
        const char *pub_topic_name, const char *sub_topic_name, const char *color,
        const char *governance_file, const char *permissions_file,
        const char *key_establishment_algorithm,
        const char *partition, float livelinessPeriod, bool enable_logging)
{
    ShapeTypeDataWriter *writer = NULL;
    ShapeTypeDataReader *reader = NULL;
    Topic *pub_topic = NULL;
    Topic *sub_topic = NULL;
    ReturnCode_t retcode;

    logger.log_message("Running create_participant() function", Verbosity::DEBUG);
    
    DomainParticipant *participant = ShapeTypeConfigurator::create_participant(
            domain_id,
            use_security,
            governance_file,
            permissions_file,
            key_establishment_algorithm,
            enable_logging);

    if ( participant == NULL ) {
      logger.log_message("failed to create participant", Verbosity::ERROR);
      return -1;
    }
    
    logger.log_message("Participant created", Verbosity::DEBUG);
    
    Duration_t send_period = {0, 100000000};
    Time_t current_time = {0, 0};

    WaitSet *wait_set = new WaitSet();
    exit_guard = new GuardCondition();
    wait_set->attach_condition(exit_guard);

    ConditionSeq active_cond;

    if ( pub_topic_name != NULL) {
        pub_topic = create_topic(participant, pub_topic_name);
        if (pub_topic == NULL ) {   return -1;  }
    }
    if ( sub_topic_name != NULL ) {
        if ( sub_topic_name != pub_topic_name ) {
            sub_topic = create_topic(participant, sub_topic_name);
        }
        else {
            sub_topic = pub_topic;
        }
        if (sub_topic == NULL ) {   return -1;  }
    }


    if ( pub_topic != NULL ) {
        writer = create_writer(participant, pub_topic, partition, livelinessPeriod, color);
        if ( writer == NULL ) { return -1; }
    }
    if ( sub_topic != NULL ) {
        reader = create_reader(participant, sub_topic, partition, livelinessPeriod);
        if ( reader == NULL ) { return -1; }
        retcode = wait_set->attach_condition(
                reader->create_readcondition(ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE));
        if ( retcode != RETCODE_OK ) {
            logger.log_message("attach_condition error", Verbosity::ERROR);
            return -1;
        }
    }

    participant->get_current_time(current_time);

    Time_t last_send_time    = {0, 0};
    Time_t send_time = { send_period.sec, send_period.nanosec};

    Time_t next_send_time    = current_time + send_time;
    Duration_t wait_timeout  = (writer==NULL)?DURATION_INFINITE:send_period;

    ShapeType shape;
#if defined(RTI_CONNEXT_DDS)
    ShapeType_initialize(&shape);
#endif
    SampleInfo info;

    int wait_count = 0;
    int sent_count = 0;
    int recv_count = 0;
    while  ( exit_application == false ) {
        char msg[256];
        int nw = snprintf(msg, 256, "\nLoop: wait count = %d, sent count = %d, "
	          "received count = %d\n",
	          wait_count, sent_count, recv_count);
        logger.log_message(msg, Verbosity::DEBUG);
	if ( nw >= 256 ) 
		logger.log_message( "[output truncated]\n", Verbosity::DEBUG );
        
        wait_count++;
        wait_set->wait(active_cond, wait_timeout);

        if ( reader != NULL ) {
            while ( reader->take_next_sample(shape, info) == DDS_RETCODE_OK )
              {
                if ( info.valid_data ) {
                    recv_count++;
                    printf("%-10s %-10s %03d %03d [%d]\n",
                           sub_topic_name,
                           shape.color STRING_IN,
                           shape.x,
                           shape.y,
                           shape.shapesize );
                }
                fflush(stdout);
            }
        }

        if ( writer != NULL )  {
            participant->get_current_time(current_time);
            if ( current_time >= next_send_time ) {
                last_send_time = current_time;
                next_send_time = last_send_time + send_time;

                {
                  char msg[256];
                  int nw = snprintf(msg, 256, "Time: %d:%u -- Sending sample: count = %d\n",
                            current_time.sec, current_time.nanosec, sent_count );
                  logger.log_message(msg, Verbosity::DEBUG);
		  if ( nw >= 256 ) 
			logger.log_message( "[output truncated]\n", Verbosity::DEBUG );
                }
                
                sent_count++;
#ifdef OPENDDS
                shape.color = color;
#else
                strcpy(shape.color, color);
#endif
                shape.x = 5*(sent_count%50);
                shape.y = (4 * (color[0] - 'A')) % 250;
                shape.shapesize = 30;
                writer->write(shape, DDS_HANDLE_NIL);
                fflush(stdout);
            }

            // Workaround missing operation wait_timeout = next_send_time - last_send_time
            Duration_t d1 = {next_send_time.sec,  next_send_time.nanosec};
            Duration_t d2 = {last_send_time.sec, last_send_time.nanosec};

            wait_timeout = d1 - d2;
        }
    }

    // clean up
    ShapeTypeConfigurator::destroy_participant( participant );
#if defined(RTI_CONNEXT_DDS)
    ShapeType_finalize(&shape);
#endif
    fprintf(stderr, "Done...\n");

    delete exit_guard;
    delete wait_set;
    return 0;
}

/*************************************************************/
void print_usage( const char * name )
{
  printf( "Usage:  %s\n", name );
  printf( "    [-pub <pubTopic>]                                  :  default: '%s'\n", DEFAULT_PUBLISH_TOPIC );
  printf( "    [-sub <subTopic>]                                  :  default: '%s'\n", DEFAULT_SUBSCRIBE_TOPIC );
  printf( "    [-domain <domainId>]                               :  default: '%d'\n", DEFAULT_DOMAIN_ID );
  printf( "    [-color <colorName>]                               :  default: '%s'\n", DEFAULT_COLOR );
  printf( "    [-governance <governanceFile>]                     :  default: '%s'\n", DEFAULT_GOVERNANCE_FILE );
  printf( "    [-permissions <permissionsFile>]                   :  default: '%s'\n", DEFAULT_PERMISSIONS_FILE );
  printf( "    [-kest <dds.sec.auth.key_establishment_algorithm>] :  default: '%s'\n", DEFAULT_KEY_EST_ALGORITHM);
  printf( "    [-partition <partitionStr>]                        :  default: '%s'\n", "<empty>" );
  printf( "    [-livelinessPeriod <float>]                        :  default: %f\n",   DEFAULT_LIVELINESS_PERIOD );
  printf( "    [-disableSecurity]                                 :  default: %s\n",   "false" );
  printf( "    [-logging]                                         :  default: %s\n",   "false" );
}

/*************************************************************/
int main(int argc, char *argv[])
{
    DomainId_t domain_id   = -1;
    const char *published_topic = NULL;
    const char *subscribed_topic = NULL;
    const char *color_name = NULL;
    const char *governance_file = NULL;
    const char *permissions_file = NULL;
    const char *key_establishment_algorithm = NULL;
    const char *partition = NULL;
    float livelinessPeriod = DEFAULT_LIVELINESS_PERIOD;
    bool        use_security = true;
    bool        enable_logging = false;

    for (int i=1; i<argc; ++i) {

        if ( strcmp( argv[i], "-v") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing 'level' after \"-v\" (expected one of 'd', or 'e')\n");
                return -1;
            }
            char vlevel = argv[i][0];
            switch ( vlevel )
              {
              case 'd':
                {
                  logger.verbosity(DEBUG);
                  break;
                }
              case 'e':
                {
                  logger.verbosity(ERROR);
                  break;
                }
              default:
                {
                  logger.log_message("unrecognized value for verbosity "
                                     + std::string(1, argv[i][0]),
                                     Verbosity::ERROR);
                  return -1;
                }
              }
        }
        else if ( strcmp(argv[i], "-domain") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <domainId> after \"-domain\"\n");
                return -1;
            }
            domain_id = atoi(argv[i]);
        }
        else if ( strcmp(argv[i], "-pub") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <topicName> after \"-publish\"\n");
                return -1;
            }
            published_topic = argv[i];
        }
        else if ( strcmp(argv[i], "-sub") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <topicName> after \"-subscribe\"\n");
                return -1;
            }
            subscribed_topic = argv[i];
        }
        else if ( strcmp(argv[i], "-color") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <colorName> after \"-color\"\n");
                return -1;
            }
            color_name = argv[i];
        }
        else if ( strcmp(argv[i], "-governance") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <governanceFile> after \"-governance\"\n");
                return -1;
            }
            governance_file = argv[i];
        }
        else if ( strcmp(argv[i], "-permissions") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <permissionsFile> after \"-permissions\"\n");
                return -1;
            }
            permissions_file = argv[i];
        }
        else if ( strcmp(argv[i], "-kest") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <value of dds.sec.auth.key_establishment_algorithm> after \"-kest\"\n");
                return -1;
            }
            key_establishment_algorithm = argv[i];
        }
        else if ( strcmp(argv[i], "-partition") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <partitionStr> after \"-parition\"\n");
                return -1;
            }
            partition = argv[i];
        }
        else if ( strcmp(argv[i], "-livelinessPeriod") == 0 ) {
            if ( ++i == argc) {
                fprintf(stderr, "Error: missing <livelinessPeriod> after \"-livelinessPeriod\"\n");
                return -1;
            }
            livelinessPeriod = atof(argv[i]);
        }
        else if ( strcmp(argv[i], "-disableSecurity") == 0 ) {
          use_security = false;
        }
        else if ( strcmp(argv[i], "-logging") == 0 ) {
          enable_logging = true;
        }
        else if ( ( strcmp(argv[i], "-help") == 0 )  ||
                  ( strcmp(argv[i], "-h") == 0 )  ) {
          print_usage( argv[0] );
          return -1;
        }
        else {
           fprintf(stderr, "Error: unrecognized option: \"%s\"\n", argv[i]);
           return -1;
        }
    }

    if ( use_security ) {
      if ( governance_file == NULL ){
        governance_file  = DEFAULT_GOVERNANCE_FILE;
        fprintf(stderr, "Info: \"-governance\" unspecified. Default to \"%s\"\n",
                governance_file);
      }

      if ( permissions_file == NULL ){
        permissions_file  = DEFAULT_PERMISSIONS_FILE;
        fprintf(stderr, "Info: \"-permissions\" unspecified. Default to \"%s\"\n",
                permissions_file);
      }

      if ( key_establishment_algorithm == NULL ){
        key_establishment_algorithm  = DEFAULT_KEY_EST_ALGORITHM;
        fprintf(stderr, "Info: \"-kest\" unspecified. Default to \"%s\"\n",
                key_establishment_algorithm);
      }
    }

    if ( ( published_topic == NULL ) && ( subscribed_topic == NULL) ){
        published_topic  = DEFAULT_PUBLISH_TOPIC;
        subscribed_topic = DEFAULT_SUBSCRIBE_TOPIC;
        fprintf(stderr, "Info: Neither \"-pub\" nor \"-sub\" was specified. Default to publish \"%s\" AND subscribe \"%s\"\n",
                published_topic, subscribed_topic);
    }

    if ( color_name == NULL ) {
        color_name = DEFAULT_COLOR;
        fprintf(stderr, "Info: \"-color\" unspecified. Default to: \"%s\"\n", color_name);
    }

    if ( (int)domain_id == -1 ) {
        domain_id = DEFAULT_DOMAIN_ID;
        fprintf(stderr, "Info: \"-domain\" unspecified. Default to: %d\n", domain_id);
    }

    setup_signal_handler();

    if (published_topic)
      printf("Publishing:  '%s'\n", published_topic);
    if (subscribed_topic)
      printf("Subscribing: '%s'\n", subscribed_topic);

    return run(
            domain_id,
            use_security,
            published_topic,
            subscribed_topic,
            color_name,
            governance_file,
            permissions_file,
            key_establishment_algorithm,
            partition,
            livelinessPeriod,
            enable_logging);
}
