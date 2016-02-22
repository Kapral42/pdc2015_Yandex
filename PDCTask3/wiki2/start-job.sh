#!/bin/sh

hdfs dfs -rm -r ./out_wiki
hadoop jar ./wikires.jar pdccourse.wiki.WikiRes -D mapred.reduce.tasks=4 ./wiki_2 ./out_wiki
    
    
