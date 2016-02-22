#!/bin/sh

JARS=`yarn classpath`

javac -classpath $JARS -d classes TwitterDistrib.java
jar -cvf twitterdistrib.jar -C classes .

#javac -classpath `hadoop classpath` WordCount.java 
#jar -cvf wordcount.jar .

