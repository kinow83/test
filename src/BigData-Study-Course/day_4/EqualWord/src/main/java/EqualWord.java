import java.io.IOException;
import java.util.*;
        
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapreduce.*;
import org.apache.hadoop.mapreduce.lib.input.*;
import org.apache.hadoop.mapreduce.lib.output.*;
        
public class EqualWord {
        
 public static class MyMapper extends Mapper<LongWritable, Text, Text, LongWritable> {
    private final static LongWritable one = new LongWritable(1);
    private Text word = new Text();
    private int A_INT = Integer.valueOf('a');
    private int Z_INT = Integer.valueOf('z');
        
    public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
        String line = value.toString();
        StringTokenizer tokenizer = new StringTokenizer(line, "\t\r\n\f |,.()<>`~!@#$%^&*()_-+=\\{}[]:;\"'<>/?");
        char h;
        int h_int;
        while (tokenizer.hasMoreTokens()) {
        	h = tokenizer.nextToken().toLowerCase().charAt(0);
        	h_int = Integer.valueOf(h+"");
        	if (h_int >= A_INT && h_int <= Z_INT) {
	            word.set(h+"");
	            context.write(word, one);
//	            System.out.println(".."+word.toString()+", "+one);
        	}
        }
    }
 } 

 public static class MyReducer extends Reducer<Text, LongWritable, Text, LongWritable> {
    private LongWritable sumWritable = new LongWritable();

    public void reduce(Text key, Iterable<LongWritable> values, Context context) 
      throws IOException, InterruptedException {
        long sum = 0;
        for (LongWritable val : values) {
            sum += val.get();
        }
        sumWritable.set(sum);
        context.write(key, sumWritable);
    }
 }
        
 public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    Job job = new Job(conf, "EqualWord");

    job.setJarByClass(EqualWord.class);
    job.setMapperClass(MyMapper.class);
    job.setReducerClass(MyReducer.class);
    
    // if mapper outputs are different, call setMapOutputKeyClass and setMapOutputValueClass
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(LongWritable.class);
        
    // An InputFormat for plain text files. Files are broken into lines. Either linefeed or carriage-return are used to signal end of line.
    // Keys are the position in the file, and values are the line of text..        
    job.setInputFormatClass(TextInputFormat.class);
    job.setOutputFormatClass(TextOutputFormat.class);
        
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
        
    
    
    job.waitForCompletion(true);
 }
        
}
