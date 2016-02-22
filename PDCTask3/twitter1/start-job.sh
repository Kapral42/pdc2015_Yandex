#!/bin/sh

hdfs dfs -rm -r ./out
hadoop jar ./twitterdistrib.jar pdccourse.twitter.TwitterDistrib -D mapred.reduce.tasks=1 ./twitter_in_stat ./out

    
    
