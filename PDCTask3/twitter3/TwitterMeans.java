package pdccourse.twitter;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

public class TwitterMeans {

  public static class TokenizerMapper
       extends Mapper<Object, Text, IntWritable, IntWritable> {

    private final static IntWritable one = new IntWritable(1);
    private Text word = new Text();
    private Integer followers = new Integer(0);
    private Integer sum = new Integer(0);
    private Integer count = new Integer(0);


    public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
      StringTokenizer itr = new StringTokenizer(value.toString());
      while (itr.hasMoreTokens()) {
        word.set(itr.nextToken());
        followers = Integer.parseInt(itr.nextToken());
        sum = sum + followers;
        count++;
      }
    }

     protected void cleanup(Context context) throws IOException, InterruptedException {
        IntWritable r_count = new IntWritable(count);
        IntWritable r_sum = new IntWritable(sum);
        context.write(r_sum, r_count);
     }

  }

     public static class IntSumReducer
       extends Reducer<IntWritable,IntWritable,DoubleWritable,NullWritable> {

    private Integer count = new Integer(0);
    private Integer sum = new Integer(0);

    public void reduce(IntWritable key, Iterable<IntWritable> values,
                       Context context
                       ) throws IOException, InterruptedException {
      for (IntWritable val : values) {
        sum += key.get();
        count += val.get();
      }
    }

    protected void cleanup(Context context) throws IOException, InterruptedException {
        context.write(new DoubleWritable((double)sum / (double)count), NullWritable.get());
     }

  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
    if (otherArgs.length != 2) {
      System.err.println("Usage: twitter count <in> <out>");
      System.exit(2);
    }
    Job job = new Job(conf, "twitter count");
    job.setJarByClass(TwitterMeans.class);
    job.setMapperClass(TokenizerMapper.class);
    job.setReducerClass(IntSumReducer.class);
    job.setOutputKeyClass(IntWritable.class);
    job.setOutputValueClass(IntWritable.class);
    FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
    FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}
