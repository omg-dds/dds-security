/*
 * This application can publish, subscribe, or publish and subscribe a single Topic
 */

#include <condition_variable>
#include "ShapeType_configurator_eprosima_fastrtps.h"
#include "ShapeTypePubSubTypes.h"
#include <fastrtps/utils/eClock.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <chrono>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

//const char *DEFAULT_GOVERNANCE_FILE     = "TESTONLY_eprosima_fastrtps_governance_signed.p7s";
//const char *DEFAULT_PERMISSIONS_FILE    = "TESTONLY_eprosima_fastrtps_permissions_allowall_signed.p7s";
//const char *DEFAULT_GOVERNANCE_FILE     = "../TESTONLY_governance_signed.p7s";
//const char *DEFAULT_PERMISSIONS_FILE    = "../TESTONLY_permissions_signed.p7s";
const char *DEFAULT_GOVERNANCE_FILE     = "../eprosima_fastrtps_certs/TESTONLY_eprosima_fastrtps_governance_signed.p7s";
const char *DEFAULT_PERMISSIONS_FILE    = "../eprosima_fastrtps_certs/TESTONLY_eprosima_fastrtps_permissions_allowall_signed.p7s";
const char *DEFAULT_PUBLISH_TOPIC       = "OD_OA_OM_OD";
const char *DEFAULT_SUBSCRIBE_TOPIC     = "OD_OA_OM_OD";
const char *DEFAULT_COLOR               = "BLACK";
const float DEFAULT_LIVELINESS_PERIOD   = 0.0;
const int   DEFAULT_DOMAIN_ID           = 0;

// Keeps main thread iterating until a signal changes its value to false
bool exit_application = false;
std::condition_variable exit_guard;
std::condition_variable read_guard;
eClock mClock;

// Signal handler changes the sets exit_service to true.
void signal_handler(int)
{
    exit_application = true;
    exit_guard.notify_all();
}

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

ShapeTypePubSubType m_type;
std::map<Participant*, std::string> m_topic_types;

TopicDataType *create_topic(Participant *participant, const char *topic_name)
{
    Domain::registerType(participant, &m_type);
    m_topic_types[participant] = topic_name;
    return &m_type;
}

ShapePublisher *create_writer(Participant *participant, TopicDataType *topic,
                                   const char * partition, float livelinessPeriod)

{
    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = WITH_KEY;
    Wparam.topic.topicDataType = topic->getName();
    Wparam.topic.topicName = m_topic_types[participant];
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.topic.resourceLimitsQos.max_samples_per_instance = 30;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    if (partition != nullptr)
    {
        Wparam.qos.m_partition.push_back(partition);
    }

    if (livelinessPeriod > 0.0)
    {
        Wparam.qos.m_liveliness.lease_duration.seconds = (int)livelinessPeriod;
        Wparam.qos.m_liveliness.lease_duration.fraction = (livelinessPeriod - (int)livelinessPeriod) * 4294967000; // ms
    }

    ShapePublisher *shapepub = new ShapePublisher();
    Publisher *pub = Domain::createPublisher(participant, Wparam, (PublisherListener*)&shapepub->m_listener);
    if (pub == nullptr)
    {
        delete shapepub;
        return nullptr;
    }

    shapepub->mp_publisher = pub;
    shapepub->mp_participant = participant;
    return shapepub;
}

ShapeSubscriber *create_reader(Participant *participant, TopicDataType *topic,
                                   const char * partition, float livelinessPeriod)

{
    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = WITH_KEY;
    Rparam.topic.topicDataType = topic->getName();
    Rparam.topic.topicName = m_topic_types[participant];
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 30;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    if (partition != nullptr)
    {
        Rparam.qos.m_partition.push_back(partition);
    }

    if (livelinessPeriod > 0.0)
    {
        Rparam.qos.m_liveliness.lease_duration.seconds = (int)livelinessPeriod;
        Rparam.qos.m_liveliness.lease_duration.fraction = (livelinessPeriod - (int)livelinessPeriod) * 4294967000; // ms
    }

    ShapeSubscriber *shapesub = new ShapeSubscriber();
    Subscriber *sub = Domain::createSubscriber(participant, Rparam, (SubscriberListener*)&shapesub->m_listener);

    if (sub == nullptr)
    {
        delete shapesub;
        return nullptr;
    }

    shapesub->mp_subscriber = sub;
    shapesub->mp_participant = participant;

    return shapesub;
}


int run(int domain_id, bool use_security,
        const char *pub_topic_name, const char *sub_topic_name, const char *color,
        const char *governance_file, const char *permissions_file,
        const char *partition, float livelinessPeriod, bool enable_logging)
{
    ShapePublisher *writer = NULL;
    ShapeSubscriber *reader = NULL;
    TopicDataType *pub_topic = NULL;
    TopicDataType *sub_topic = NULL;

    Participant *participant = ShapeTypeConfigurator::create_participant(
            domain_id,
            use_security,
            governance_file,
            permissions_file,
            key_establishment_algorithm,
            enable_logging);

    if ( participant == NULL ) { return -1; }

    Duration_t send_period = {1, 0};
    Time_t current_time = {0, 0};
    bool error = false;

    if ( pub_topic_name != NULL) {
        pub_topic = create_topic(participant, pub_topic_name);
        if (pub_topic == NULL ) {   error = true;  }
    }
    if ( !error && sub_topic_name != NULL ) {
        if ( (pub_topic_name == NULL) || strcmp(sub_topic_name, pub_topic_name) != 0 ) {
            sub_topic = create_topic(participant, sub_topic_name);
        }
        else {
            sub_topic = pub_topic;
        }
        if (sub_topic == NULL ) {   error = true;  }
    }


    if ( !error && pub_topic != NULL ) {
        writer = create_writer(participant, pub_topic, partition, livelinessPeriod);
        if ( writer == NULL ) { error = true; }
    }
    if ( !error && sub_topic != NULL ) {
        reader = create_reader(participant, sub_topic, partition, livelinessPeriod);
        if ( reader == NULL ) { error = true; }
        else { reader->set_read_guard(&read_guard); }
    }
    
    if(error)
    {
        ShapeTypeConfigurator::destroy_participant( participant );
        return -1;
    }

    mClock.setTimeNow(&current_time);


    Time_t last_send_time    = {0, 0};
    Time_t send_time = { send_period.seconds, send_period.fraction};

    Time_t next_send_time    = current_time + send_time;
    Duration_t wait_timeout  = (writer==NULL) ? c_TimeInfinite : send_period;

    ShapeType shape;
    SampleInfo_t info;
    std::mutex mutex;

    int wait_count = 0;
    int sent_count = 0;
    int recv_count = 0;
    while  ( exit_application == false ) {
        std::unique_lock<std::mutex> lock(mutex);
        printf("\nLoop: wait count = %d, sent count = %d, "
           "received count = %d\n",
           wait_count++, sent_count, recv_count);
        read_guard.wait_for(lock, std::chrono::nanoseconds(wait_timeout.to_ns()));

        if ( reader != NULL ) {
            while ( reader->take_next_sample(shape, info) )
              {
                printf("Received sample...\n");
                if ( info.sampleKind == ALIVE ) {
                    recv_count++;
                    std::cout << shape << std::endl;
                }
                fflush(stdout);
            }
        }

        if ( writer != NULL )  {
            mClock.setTimeNow(&current_time);
            if ( current_time >= next_send_time ) {
                last_send_time = current_time;
                next_send_time = last_send_time + send_time;

                printf("Time: %d:%u -- Sending sample: count = %d\n",
                    current_time.seconds,
                    static_cast<uint32_t>(current_time.to_ns() - (current_time.seconds * 1000000000)),
                    sent_count++);
                shape.color(color);
                shape.x(5*(sent_count%50));
                shape.y((4 * (color[0] - 'A')) % 250);
                shape.shapesize(30);
                writer->write(&shape);
                fflush(stdout);
            }

            // Workaround missing operation wait_timeout = next_send_time - last_send_time
            Duration_t d1 = next_send_time;
            Duration_t d2 = last_send_time;

            wait_timeout = d1 - d2;
        }
    }

    // clean up
    ShapeTypeConfigurator::destroy_participant( participant );
    fprintf(stderr, "Done...\n");
    return 0;
}

void print_usage( const char * name )
{
  printf( "Usage:  %s\n", name );
  printf( "    [-pub <pubTopic>]                :  default: '%s'\n", DEFAULT_PUBLISH_TOPIC );
  printf( "    [-sub <subTopic>]                :  default: '%s'\n", DEFAULT_SUBSCRIBE_TOPIC );
  printf( "    [-domain <domainId>]             :  default: '%d'\n", DEFAULT_DOMAIN_ID );
  printf( "    [-color <colorName>]             :  default: '%s'\n", DEFAULT_COLOR );
  printf( "    [-governance <governanceFile>]   :  default: '%s'\n", DEFAULT_GOVERNANCE_FILE );
  printf( "    [-permissions <permissionsFile>] :  default: '%s'\n", DEFAULT_PERMISSIONS_FILE );
  printf( "    [-partition <partitionStr>]      :  default: '%s'\n", "<empty>" );
  printf( "    [-livelinessPeriod <float>]      :  default: %f\n",   DEFAULT_LIVELINESS_PERIOD );
  printf( "    [-disableSecurity]               :  default: %s\n",   "false" );
  printf( "    [-logging]                       :  default: %s\n",   "false" );
}

int main(int argc, char *argv[])
{
    int domain_id   = -1;
    const char *published_topic = NULL;
    const char *subscribed_topic = NULL;
    const char *color_name = NULL;
    const char *governance_file = NULL;
    const char *permissions_file = NULL;
    const char *partition = NULL;
    float livelinessPeriod = DEFAULT_LIVELINESS_PERIOD;
    bool        use_security = true;
    bool        enable_logging = false;

    Log::SetVerbosity(Log::Kind::Error);

    for (int i=1; i<argc; ++i) {
        if ( strcmp(argv[i], "-domain") == 0 ) {
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

    if ( domain_id == -1 ) {
        domain_id = DEFAULT_DOMAIN_ID;
        fprintf(stderr, "Info: \"-domain\" unspecified. Default to: %d\n", domain_id);
    }

    setup_signal_handler();

    if (published_topic)
      printf("Publishing:  '%s'\n", published_topic);
    if (subscribed_topic)
      printf("Subscribing: '%s'\n", subscribed_topic);

    return run(domain_id, use_security, published_topic, subscribed_topic, color_name,
               governance_file, permissions_file,
               partition, livelinessPeriod, enable_logging);
}

