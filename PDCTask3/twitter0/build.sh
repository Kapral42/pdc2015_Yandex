#!/bin/sh

JARS=`yarn classpath`

javac -classpath $JARS -d classes TwitterStat.java
jar -cvf twitterstat.jar -C classes .

#javac -classpath `hadoop classpath` WordCount.java 
#jar -cvf wordcount.jar .

