#!/bin/sh

hdfs dfs -rm -r ./out
hadoop jar ./twittermeans.jar pdccourse.twitter.TwitterMeans -D mapred.reduce.tasks=1 ./twitter_in_stat ./out
    
    
