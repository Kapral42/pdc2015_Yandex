#!/bin/sh

hdfs dfs -rm -r ./outstat
hadoop jar ./twitterstat.jar pdccourse.twitter.TwitterStat -D mapred.reduce.tasks=1 /pub/followers.db ./outstat
    
    
