package pdccourse.wiki;

import java.io.IOException;
import java.util.StringTokenizer;
import java.util.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

import org.apache.hadoop.mapreduce.Counter;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.RunningJob;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapreduce.Cluster;

public class WordCount {

    static enum Counters {PAGE_COUNT};

    public static class TokenizerMapper
       extends Mapper<Text, Text, Text, Text> {

    private Text word = new Text();
    private Text resVal = new Text();
    String token;

    public void map(Text key, Text value, Context context) throws IOException, InterruptedException {
        StringTokenizer itr = new StringTokenizer(value.toString(), " \n\t\r,:?!|=()[]{}$%&*#\"<>;_.@\\/^\'+№~`-—“„–");
        Integer wordInPage = 0;
        Map<String, Integer> rez = new LinkedHashMap<String, Integer>();
        while (itr.hasMoreTokens()) {
            token = itr.nextToken();
            rez.put(token, rez.get(token) == null? 1 : rez.get(token) + 1);
            wordInPage++;
        }
        for(Map.Entry<String, Integer> e : rez.entrySet()) {
            word.set(e.getKey());
            float tf = (float)e.getValue() / (float)wordInPage;
            resVal.set(key.toString() + "@" + tf);
            context.write(word, resVal);
        }
        context.getCounter(Counters.PAGE_COUNT).increment(1);
    }
  }

  public static class IntSumReducer
       extends Reducer<Text, Text, Text, Text> {
    private long mapperCounter;

    @Override
    public void setup(Context context) throws IOException, InterruptedException{
         Configuration conf = context.getConfiguration();
         Cluster cluster = new Cluster(conf);
         Job currentJob = cluster.getJob(context.getJobID());
         mapperCounter = currentJob.getCounters().findCounter(Counters.PAGE_COUNT).getValue();
    }

    public void reduce(Text key, Iterable<Text> values,
                       Context context
                       ) throws IOException, InterruptedException {
        for (Text val : values) {
            context.write(key, new Text(val.toString() + "@" + Long.toString(mapperCounter)));
        }
    }
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
    if (otherArgs.length != 2) {
      System.err.println("Usage: wordcount <in> <out>");
      System.exit(2);
    }
    conf.set("mapreduce.input.keyvaluelinerecordreader.key.value.separator", " ");
    conf.setBoolean("exact.match.only", true);
    conf.set("io.serializations",
             "org.apache.hadoop.io.serializer.JavaSerialization,"
             + "org.apache.hadoop.io.serializer.WritableSerialization");

    Job job = new Job(conf, "word count wiki");
    job.setInputFormatClass(XmlInputFormat.class);
    job.setJarByClass(WordCount.class);
    job.setMapperClass(TokenizerMapper.class);
    job.setReducerClass(IntSumReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
    FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}
