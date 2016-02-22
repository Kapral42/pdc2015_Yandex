package pdccourse.twitter;

import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.io.WritableComparator;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.util.GenericOptionsParser;

public class TwitterTop {

    static class ReverseComparator extends WritableComparator {
        private static final IntWritable.Comparator TEXT_COMPARATOR = new IntWritable.Comparator();

        public ReverseComparator() {
            super(IntWritable.class);
        }

        @Override
        public int compare(byte[] b1, int s1, int l1, byte[] b2, int s2, int l2) {
                return (-1)* TEXT_COMPARATOR
                        .compare(b1, s1, l1, b2, s2, l2);
        }

        @Override
        public int compare(WritableComparable a, WritableComparable b) {
            if (a instanceof IntWritable && b instanceof IntWritable) {
                return (-1)*(((IntWritable) a).compareTo((IntWritable) b));
            }
            return super.compare(a, b);
        }
    }

  public static class TokenizerMapper 
       extends Mapper<Object, Text, IntWritable, IntWritable> {
    
    private IntWritable id = new IntWritable();
    private IntWritable followers = new IntWritable();
      
    public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
        StringTokenizer itr = new StringTokenizer(value.toString());
        id.set(Integer.parseInt(itr.nextToken()));
        followers.set(Integer.parseInt(itr.nextToken()));
        context.write(followers, id);
    } 
    
  }
      

  public static class TopReducer 
       extends Reducer<IntWritable,IntWritable,IntWritable,IntWritable> {
    
    private IntWritable id = new IntWritable();
    private IntWritable followers = new IntWritable();
    private int count = 0;

    public void reduce(IntWritable key, Iterable<IntWritable> values, Context context) 
                        throws IOException, InterruptedException {
        followers = key;
        for (IntWritable val : values) {
            id = val;
            count++;
            if (count > 50)
                return;
            context.write(id, followers);
        }
    }
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
    if (otherArgs.length != 2) {
      System.err.println("Usage: twitter top <in> <out>");
      System.exit(2);
    }
    Job job = new Job(conf, "twitter top");
    job.setJarByClass(TwitterTop.class);
    job.setMapperClass(TokenizerMapper.class);
    job.setCombinerClass(TopReducer.class);
    job.setReducerClass(TopReducer.class);
    job.setSortComparatorClass(ReverseComparator.class);
    job.setOutputKeyClass(IntWritable.class);
    job.setOutputValueClass(IntWritable.class);
    FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
    FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}
