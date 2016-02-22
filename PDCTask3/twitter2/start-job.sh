#!/bin/sh

hdfs dfs -rm -r ./out
hadoop jar ./twittertop.jar pdccourse.twitter.TwitterTop -D mapred.reduce.tasks=1 ./twitter_in_stat ./out
    
    
