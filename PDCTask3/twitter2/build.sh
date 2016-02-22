#!/bin/sh

JARS=`yarn classpath`

javac -classpath $JARS -d classes TwitterTop.java
jar -cvf twittertop.jar -C classes .

#javac -classpath `hadoop classpath` WordCount.java 
#jar -cvf wordcount.jar .

