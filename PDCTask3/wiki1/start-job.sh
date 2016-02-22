#!/bin/sh

hdfs dfs -rm -r ./out_wiki
hadoop jar ./wordcount.jar pdccourse.wiki.WordCount -D mapreduce.job.reduces=1 /pub/ruwiki.xml ./out_wiki



    
    
