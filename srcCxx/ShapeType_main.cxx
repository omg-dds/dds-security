/*
 * This application can publish, subscribe, or publish and subscribe a single Topic
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(RTI_CONNEXT_DDS)
#include "rti_connext_dds/ShapeType_configurator_rti_connext_dds.h"
#elif defined(TWINOAKS_COREDX)
#include "toc_coredx_dds/ShapeType_configurator_toc_coredx_dds.h"
#else
#include "ShapeType_configurator_default.h"
#endif

using namespace DDS;

const char *DEFAULT_GOVERNANCE_FILE     = "TESTONLY_governance_signed.p7s";
const char *DEFAULT_PERMISSIONS_FILE    = "TESTONLY_permissions_signed.p7s";
const char *DEFAULT_PUBLISH_TOPIC       = "OD_OA_OM_OD";
const char *DEFAULT_SUBSCRIBE_TOPIC     = "OD_OA_OM_OD";
const char *DEFAULT_COLOR     = "BLACK";
const int DAFAULT_DOMAIN_ID   = 0;

// Keeps main thread iterating until a signal changes its value to false
bool exit_application = false;
GuardCondition *exit_guard = NULL;

// Signal handler changes the sets exit_service to true.
void signal_handler(int signal)
{
    fprintf(stderr, "Stopping...\n");
    exit_application = true;
    if ( exit_guard != NULL ) {
        exit_guard->set_trigger_value(BOOLEAN_TRUE);
    }
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

Topic *create_topic(DomainParticipant *participant, const char *topic_name)
{
    /* Register the type before creating the topic */
    const char *type_name = ShapeTypeTypeSupport::get_type_name();
    ReturnCode_t retcode = ShapeTypeTypeSupport::register_type(
        participant, type_name);
    if (retcode != RETCODE_OK) {
        printf("register_type error %d\n", (int)retcode);
        return NULL;
    }

    Topic *topic = participant->create_topic(topic_name, ShapeTypeTypeSupport::get_type_name(),
            TOPIC_QOS_DEFAULT, NULL /* listener */, STATUS_MASK_NONE);
    if ( topic == NULL ) {
        printf("create_topic error for topic '%s'\n", topic_name);
        return NULL;
    }

    return topic;
}

ShapeTypeDataWriter *create_writer(DomainParticipant *participant, Topic *topic)

{
    Publisher *publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT,
            NULL /* listener */, STATUS_MASK_NONE);
    if (publisher == NULL) {
        printf("create_publisher error\n");
        return NULL;
    }
    DataWriter *writer = publisher->create_datawriter(
            topic, DATAWRITER_QOS_DEFAULT, NULL /* listener */, STATUS_MASK_NONE);
    if (writer == NULL) {
        printf("create_datawriter error\n");
        return NULL;
    }
    ShapeTypeDataWriter *shape_writer = ShapeTypeDataWriter::narrow(writer);
    if (shape_writer == NULL) {
        printf("ShapeTypeDataWriter narrow error\n");
        return NULL;
    }
    return shape_writer;
}

ShapeTypeDataReader *create_reader(DomainParticipant *participant, Topic *topic)

{
    Subscriber *subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
            NULL /* listener */, STATUS_MASK_NONE);
    if (subscriber == NULL) {
        printf("create_publisher error\n");
        return NULL;
    }
    DataReader *reader = subscriber->create_datareader(
            topic, DATAREADER_QOS_DEFAULT, NULL /* listener */, STATUS_MASK_NONE);
    if (reader == NULL) {
        printf("create_datareader error\n");
        return NULL;
    }
    ShapeTypeDataReader *shape_reader = ShapeTypeDataReader::narrow(reader);
    if (shape_reader == NULL) {
        printf("ShapeTypeDataReader narrow error\n");
        return NULL;
    }
    return shape_reader;
}


int run(DomainId_t domain_id, const char *pub_topic_name, const char *sub_topic_name, const char *color,
        const char *governance_file, const char *permissions_file)
{
    ShapeTypeDataWriter *writer = NULL;
    ShapeTypeDataReader *reader = NULL;
    Topic *pub_topic = NULL;
    Topic *sub_topic = NULL;
    ReturnCode_t retcode;
    
    DomainParticipant *participant = ShapeTypeConfigurator::create_participant(domain_id, governance_file, permissions_file);

    if ( participant == NULL ) { return -1; }

#if defined(TWINOAKS_COREDX)
    Duration_t send_period(1, 0);
    Time_t current_time(0, 0);
#else
    Duration_t send_period = {1, 0};
    Time_t current_time = {0, 0};
#endif

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
        writer = create_writer(participant, pub_topic);
        if ( writer == NULL ) { return -1; }
    }
    if ( sub_topic != NULL ) {
        reader = create_reader(participant, sub_topic);
        if ( reader == NULL ) { return -1; }
        retcode = wait_set->attach_condition(
                reader->create_readcondition(ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE));
        if ( retcode != RETCODE_OK ) {
            printf("attach_condition error. Retcode: %d\n", (int)retcode);
            return -1;
        }
    }

    participant->get_current_time(current_time);
#if defined(TWINOAKS_COREDX)
    Time_t last_send_time(0, 0);
    Time_t send_time(send_period.sec, send_period.nanosec);
#else
    Time_t last_send_time    = {0, 0};
    Time_t send_time = { send_period.sec, send_period.nanosec};
#endif
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
        printf("\nLoop: wait count = %d, sent count = %d, "
	       "received count = %d\n", 
	       wait_count++, sent_count, recv_count);
        wait_set->wait(active_cond, wait_timeout);

        if ( reader != NULL ) {
#if defined(TWINOAKS_COREDX)
            while ( reader->take_next_sample(&shape, &info) == DDS_RETCODE_OK ) 
#else
            while ( reader->take_next_sample(shape, info) == DDS_RETCODE_OK ) 
#endif
              {
                printf("Received sample...\n");
                if ( info.valid_data ) {
                    recv_count++;
#if defined(TWINOAKS_COREDX)
                    ShapeType::print(stdout, &shape);
#else
                    ShapeTypeTypeSupport::print_data(&shape);
#endif
                }
            }
        }

        if ( writer != NULL )  {
            participant->get_current_time(current_time);
            if ( current_time >= next_send_time ) {
                last_send_time = current_time;
                next_send_time = last_send_time + send_time;

                printf("Time: %d:%u -- Sending sample: count = %d\n", 
		       current_time.sec, current_time.nanosec, sent_count++);
                strcpy(shape.color, color);
                shape.x = 5*(sent_count%50);
                shape.y = (4 * (color[0] - 'A')) % 250;
                shape.shapesize = 30;
#if defined(TWINOAKS_COREDX)
                writer->write(&shape, DDS_HANDLE_NIL);
#else
                writer->write(shape, DDS_HANDLE_NIL);
#endif
            }

            // Workaround missing operation wait_timeout = next_send_time - last_send_time
#if defined(TWINOAKS_COREDX)
            Duration_t d1(next_send_time.sec,  next_send_time.nanosec);
            Duration_t d2(last_send_time.sec, last_send_time.nanosec);
#else
            Duration_t d1 = {next_send_time.sec,  next_send_time.nanosec};
            Duration_t d2 = {last_send_time.sec, last_send_time.nanosec};
#endif
            wait_timeout = d1 - d2;
        }
    }

    // clean up 
    ShapeTypeConfigurator::destroy_participant( participant );
    fprintf(stderr, "Done...\n");

    return 0;
}

int main(int argc, char *argv[])
{
    DomainId_t domain_id   = -1;
    const char *published_topic = NULL;
    const char *subscribed_topic = NULL;
    const char *color_name = NULL;
    const char *governance_file = NULL;
    const char *permissions_file = NULL;

    if ( argc != 5 ) {
        fprintf(stderr, "Usage:  %s [-pub <pubTopic>] [-sub <subTopic>] [-domain <domainId>] [-color <colorName>]\n"
                "\t\t [-governance <governanceFile>]  [-permissions <permissionsFile>]\n",
                argv[0]);
    }
    
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
        else if ( strcmp(argv[i], "-help") == 0 ) {
            return -1;
        }
        else {
           fprintf(stderr, "Error: unrecognized option: \"%s\"\n", argv[i]);
           return -1;
        }
    }


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
        domain_id = DAFAULT_DOMAIN_ID;
        fprintf(stderr, "Info: \"-domain\" unspecified. Default to: %d\n", domain_id);
    }
 
    setup_signal_handler();
    
    if (published_topic)
      printf("Publishing:  '%s'\n", published_topic);
    if (subscribed_topic)
      printf("Subscribing: '%s'\n", subscribed_topic);
    
    return run(domain_id, published_topic, subscribed_topic, color_name, governance_file, permissions_file);
}
