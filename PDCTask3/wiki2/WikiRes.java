package pdccourse.wiki;

import java.io.IOException;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.*;
import org.apache.commons.collections.IteratorUtils;

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

public class WikiRes {

  public static class TokenizerMapper 
       extends Mapper<Object, Text, Text, Text> {
    
    private Text first = new Text();
    private Text second = new Text();
          
    public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
        StringTokenizer itr = new StringTokenizer(value.toString());
        first.set(itr.nextToken());//word
        second.set(itr.nextToken());//id@TF@page_count
        context.write(first, second);
    }
  }
  
     public static class IntSumReducer 
       extends Reducer<Text,Text,Text,Text> {
   
    private Integer count = new Integer(0);
    private Integer page_count = new Integer(0);
    private Text id = new Text();
    private Text result = new Text();
    boolean flg = false;
     
    public void reduce(Text key, Iterable<Text> values, 
                       Context context
                       ) throws IOException, InterruptedException {
        int page_w_count = 0;
        float TF;
        float TF_IDF;
        TreeMap top = new TreeMap<Float, Vector>();

        for (Text val : values) {
            page_w_count++;
            StringTokenizer itr = new StringTokenizer(val.toString(), "@");
            id.set(itr.nextToken());
            TF = Float.parseFloat(itr.nextToken());
            if (!flg) {// read page_count
                page_count = Integer.parseInt(itr.nextToken());
                flg = true;
            }
            Vector res = (Vector)top.get(TF);
            if (res == null) {
                res = new Vector();
            }
            if (res.size() <= 10) {
                res.add(id.toString());
                top.put(TF, res);
            }
            if (top.size() > 10) {
                top.remove(top.firstKey());
            }
        }
        float IDF = (float) Math.log((float)page_count / (float)page_w_count);
        String str = "";
        Map.Entry e;
        int out_count = 0;
        while (top.size() > 0 && out_count < 10){
            e = top.lastEntry();
            if (e == null) break;
            Vector v = (Vector)e.getValue();
            for (int i = 0; i < v.size() && out_count < 10; ++i) {
                TF_IDF = (float)e.getKey() * IDF;
                str = str + v.get(i) + "@" + TF_IDF + " ";
                out_count++;
            }
            top.remove(top.lastKey());
        }
        context.write(key, new Text(str));
        
    } 
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
    if (otherArgs.length != 2) {
      System.err.println("Usage: page count <in> <out>");
      System.exit(2);
    }
    Job job = new Job(conf, "wiki page count");
    job.setJarByClass(WikiRes.class);
    job.setMapperClass(TokenizerMapper.class);
    job.setReducerClass(IntSumReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
    FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}
