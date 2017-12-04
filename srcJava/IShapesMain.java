import java.util.Collection;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Condition;
import org.omg.dds.core.Duration;
import org.omg.dds.core.GuardCondition;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.Time;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.policy.Liveliness;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.Topic;


public class IShapesMain
{

   private static final String DEFAULT_GOVERNANCE_FILE     = "TESTONLY_governance_signed.p7s";
   private static final String DEFAULT_PERMISSIONS_FILE    = "TESTONLY_permissions_signed.p7s";
   private static final String DEFAULT_PUBLISH_TOPIC       = "OD_OA_OM_OD";
   private static final String DEFAULT_SUBSCRIBE_TOPIC     = "OD_OA_OM_OD";
   private static final String DEFAULT_COLOR               = "BLACK";
   private static final float DEFAULT_LIVELINESS_PERIOD    = 0.0F;
   private static final int   DEFAULT_DOMAIN_ID            = 0;

   private ShapeTypeConfigurator shape_type_configurator;
   private ServiceEnvironment env;

   // Keeps main thread iterating until a signal changes its value to false
   private boolean exit_application = false;


   public IShapesMain(ShapeTypeConfigurator shape_type_configurator)
   {
      this.shape_type_configurator = shape_type_configurator;
      this.env = shape_type_configurator.getServiceEnvironment();
   }


   private Topic<ShapeType> create_topic(DomainParticipant participant, String topic_name)
   {
      return participant.createTopic(topic_name, ShapeType.class);
   }

   private DataWriter<ShapeType> create_writer(
         DomainParticipant participant,
         Topic<ShapeType> topic,
         String partition,
         float livelinessPeriod)
   {
      PublisherQos pub_qos = participant.getDefaultPublisherQos();

      if (partition != null)
      {
         Partition p = PolicyFactory.getPolicyFactory(env).Partition().withName(partition);
         pub_qos = pub_qos.withPolicy(p);
      }

      Publisher publisher = participant.createPublisher(pub_qos);

      DataWriterQos dw_qos = publisher.getDefaultDataWriterQos();
      if (livelinessPeriod > 0.0F)
      {
         Liveliness l = PolicyFactory.getPolicyFactory(env).Liveliness().withLeaseDuration(
               (long)(livelinessPeriod*1000000000), TimeUnit.NANOSECONDS);
         dw_qos = dw_qos.withPolicies(l);
      }

      return publisher.createDataWriter(topic, dw_qos);
   }

   private DataReader<ShapeType> create_reader(
         DomainParticipant participant,
         Topic<ShapeType> topic,
         String partition,
         float livelinessPeriod)
   {
      SubscriberQos sub_qos = participant.getDefaultSubscriberQos();

      if (partition != null)
      {
         Partition p = PolicyFactory.getPolicyFactory(env).Partition().withName(partition);
         sub_qos = sub_qos.withPolicy(p);
      }

      Subscriber subscriber = participant.createSubscriber(sub_qos);

      DataReaderQos dr_qos = subscriber.getDefaultDataReaderQos();
      if (livelinessPeriod > 0.0F)
      {
         Liveliness l = PolicyFactory.getPolicyFactory(env).Liveliness().withLeaseDuration(
               (long)(livelinessPeriod*1000000000), TimeUnit.NANOSECONDS);
         dr_qos = dr_qos.withPolicies(l);
      }

      return subscriber.createDataReader(topic, dr_qos);
   }


   public void run(int domain_id,
         boolean use_security,
         boolean logging,
         String pub_topic_name,
         String sub_topic_name,
         String color,
         String governance_file,
         String permissions_file,
         String partition,
         float livelinessPeriod)
   {
      DataWriter<ShapeType> writer = null;
      DataReader<ShapeType> reader = null;
      Topic<ShapeType> pub_topic = null;
      Topic<ShapeType> sub_topic = null;


      DomainParticipant participant = 
            shape_type_configurator.create_participant(domain_id, use_security, logging, governance_file, permissions_file);

      Duration send_period = Duration.newDuration(1, TimeUnit.SECONDS, env);

      WaitSet wait_set = env.getSPI().newWaitSet();
      GuardCondition exit_guard = GuardCondition.newGuardCondition(env);
      wait_set.attachCondition(exit_guard);

      Collection<Condition> active_cond;

      if (pub_topic_name != null)
      {
         pub_topic = create_topic(participant, pub_topic_name);
      }

      if (sub_topic_name != null)
      {
         if (sub_topic_name.equals(pub_topic_name))
         {
            sub_topic = pub_topic;
         }
         else
         {
            sub_topic = create_topic(participant, sub_topic_name);
         }
      }

      if (pub_topic != null)
      {
         writer = create_writer(participant, pub_topic, partition, livelinessPeriod);
      }

      if (sub_topic != null)
      {
         reader = create_reader(participant, sub_topic, partition, livelinessPeriod);
         wait_set.attachCondition(
               reader.createReadCondition(
                     reader.getParent().createDataState()
                     .withAnySampleState().withAnyViewState().withAnyInstanceState()));
      }


      Time current_time = participant.getCurrentTime();
      Time last_send_time = Time.newTime(0, TimeUnit.SECONDS, env);
      Time send_time = Time.newTime(send_period.getDuration(TimeUnit.NANOSECONDS), TimeUnit.NANOSECONDS, env);

      Time next_send_time = addTime(current_time, send_time);
      Duration wait_timeout = writer == null ? Duration.infiniteDuration(env) : send_period;


      int wait_count = 0;
      int sent_count = 0;
      int recv_count = 0;
      while  ( exit_application == false ) {
         System.out.format("\nLoop: wait count = %d, sent count = %d, "+
               "received count = %d\n",
               wait_count++, sent_count, recv_count);
         try
         {
            active_cond = wait_set.waitForConditions(wait_timeout);
         }
         catch (TimeoutException e)
         {
            // ignore
         }

         if (reader != null)
         {
            Iterator<ShapeType> it = reader.take();
            while (it.hasNext())
            {
               System.out.println("Received sample...");
               Sample<ShapeType> sample = it.next();
               if (sample.getData() != null)
               {
                  recv_count++;
                  shape_type_configurator.print_data(sample.getData());
               }
            }
         }

         if (writer != null)
         {
            current_time = participant.getCurrentTime();
            if (current_time.getTime(TimeUnit.NANOSECONDS) >= next_send_time.getTime(TimeUnit.NANOSECONDS))
            {
               last_send_time = current_time;
               next_send_time = addTime(last_send_time, send_time);

               System.out.format("Time: %d:%d -- Sending sample: count = %d\n",
                     current_time.getTime(TimeUnit.SECONDS),
                     current_time.getRemainder(TimeUnit.SECONDS, TimeUnit.NANOSECONDS),
                     sent_count++);
               ShapeType shape = new ShapeType(
                     color,
                     5*(sent_count%50),
                     (4 * (color.charAt(0) - 'A')) % 250,
                     30);
               try
               {
                  writer.write(shape);
               }
               catch (TimeoutException e)
               {
                  System.err.println("TimeoutException writing shape");
               }
            }

            wait_timeout = subTime(next_send_time, last_send_time);
         }
      }

      // clean up
      participant.close();
      System.err.println("Done...");
   }

   private Time addTime(Time t1, Time t2)
   {
      return Time.newTime(
            t1.getTime(TimeUnit.NANOSECONDS) + t2.getTime(TimeUnit.NANOSECONDS),
            TimeUnit.NANOSECONDS,
            env);
   }

   private Duration subTime(Time t1, Time t2)
   {
      return Duration.newDuration(
            t1.getTime(TimeUnit.NANOSECONDS) - t2.getTime(TimeUnit.NANOSECONDS),
            TimeUnit.NANOSECONDS,
            env);
   }


   public static void print_usage(String name)
   {
      System.out.println( "Usage:  java "+ name );
      System.out.println( "    [-pub <pubTopic>]                :  default: "+ DEFAULT_PUBLISH_TOPIC );
      System.out.println( "    [-sub <subTopic>]                :  default: "+ DEFAULT_SUBSCRIBE_TOPIC );
      System.out.println( "    [-domain <domainId>]             :  default: "+ DEFAULT_DOMAIN_ID );
      System.out.println( "    [-color <colorName>]             :  default: "+ DEFAULT_COLOR );
      System.out.println( "    [-governance <governanceFile>]   :  default: "+ DEFAULT_GOVERNANCE_FILE );
      System.out.println( "    [-permissions <permissionsFile>] :  default: "+ DEFAULT_PERMISSIONS_FILE );
      System.out.println( "    [-partition <partitionStr>]      :  default: "+ "<empty>" );
      System.out.println( "    [-livelinessPeriod <float>]      :  default: "+ DEFAULT_LIVELINESS_PERIOD );
      System.out.println( "    [-disableSecurity]               :  default: "+ "false" );
      System.out.println( "    [-logging]                       :  default: "+ "false" );
   }



   public static void main(String[] args)
   {
      int domain_id   = -1;
      String published_topic = null;
      String subscribed_topic = null;
      String color_name = null;
      String governance_file = null;
      String permissions_file = null;
      String partition = null;
      float livelinessPeriod = DEFAULT_LIVELINESS_PERIOD;
      boolean use_security = true;
      boolean logging = false;

      for (int i = 0; i < args.length; i++)
      {
         if ( args[i].equals("-domain")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <domainId> after \"-domain\"\n");
               System.exit(-1);
            }
            domain_id = Integer.parseInt(args[i]);
         }
         else if ( args[i].equals("-pub")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <topicName> after \"-publish\"\n");
               System.exit(-1);
            }
            published_topic = args[i];
         }
         else if ( args[i].equals("-sub")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <topicName> after \"-subscribe\"\n");
               System.exit(-1);
            }
            subscribed_topic = args[i];
         }
         else if ( args[i].equals("-color")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <colorName> after \"-color\"\n");
               System.exit(-1);
            }
            color_name = args[i];
         }
         else if ( args[i].equals("-governance")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <governanceFile> after \"-governance\"\n");
               System.exit(-1);
            }
            governance_file = args[i];
         }
         else if ( args[i].equals("-permissions")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <permissionsFile> after \"-permissions\"\n");
               System.exit(-1);
            }
            permissions_file = args[i];
         }
         else if ( args[i].equals("-partition")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <partitionStr> after \"-parition\"\n");
               System.exit(-1);
            }
            partition = args[i];
         }
         else if ( args[i].equals("-livelinessPeriod")) {
            if ( ++i == args.length) {
               System.err.println("Error: missing <livelinessPeriod> after \"-livelinessPeriod\"\n");
               System.exit(-1);
            }
            livelinessPeriod = Float.parseFloat(args[i]);
         }
         else if ( args[i].equals("-disableSecurity")) {
            use_security = false;
         }
         else if ( args[i].equals("-logging")) {
            logging = true;
         }
         else if ( ( args[i].equals("-help"))  ||
               ( args[i].equals("-h"))  ) {
            print_usage( args[0] );
            System.exit(-1);
         }
         else {
            System.err.println("Error: unrecognized option: "+ args[i]);
            System.exit(-1);
         }         
      }

      if ( use_security ) {
         if ( governance_file == null ){
            governance_file  = DEFAULT_GOVERNANCE_FILE;
            System.err.println("Info: \"-governance\" unspecified. Default to " +
                  governance_file);
         }

         if ( permissions_file == null ){
            permissions_file  = DEFAULT_PERMISSIONS_FILE;
            System.err.println("Info: \"-permissions\" unspecified. Default to " +
                  permissions_file);
         }
      }

      if ( ( published_topic == null ) && ( subscribed_topic == null) ){
         published_topic  = DEFAULT_PUBLISH_TOPIC;
         subscribed_topic = DEFAULT_SUBSCRIBE_TOPIC;
         System.err.format("Info: Neither \"-pub\" nor \"-sub\" was specified. Default to publish \"%s\" AND subscribe \"%s\"\n",
               published_topic, subscribed_topic);
      }

      if ( color_name == null ) {
         color_name = DEFAULT_COLOR;
         System.err.println("Info: \"-color\" unspecified. Default to: " + color_name);
      }

      if ( (int)domain_id == -1 ) {
         domain_id = DEFAULT_DOMAIN_ID;
         System.err.println("Info: \"-domain\" unspecified. Default to: " + domain_id);
      }

      if (published_topic != null)
      {
         System.out.println("Publishing:  "+ published_topic);
      }
      if (subscribed_topic != null)
      {
         System.out.println("Subscribing: " + subscribed_topic);
      }

      
      IShapesMain main = new IShapesMain(new CafeConfigurator(use_security, logging));
      main.run(domain_id, use_security, logging, published_topic, subscribed_topic, color_name,
            governance_file, permissions_file,
            partition, livelinessPeriod);
   }

}
