#!/bin/sh

JARS=`yarn classpath`

javac -classpath $JARS -d classes TwitterMeans.java
jar -cvf twittermeans.jar -C classes .

#javac -classpath `hadoop classpath` WordCount.java 
#jar -cvf wordcount.jar .

