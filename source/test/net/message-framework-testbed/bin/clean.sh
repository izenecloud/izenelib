#!/bin/sh

ps  ax |grep ControllerProcess | awk '{print $1}' |xargs kill -9  
ps  ax |grep IndexProcess | awk '{print $1}' |xargs kill -9
ps  ax |grep MainProcess | awk '{print $1}' |xargs kill -9
ps  ax |grep DocumentProcess | awk '{print $1}' |xargs kill -9
ps  ax |grep LAProcess | awk '{print $1}' |xargs kill -9

#rm *.txt *.dat *.sdb gmon.out index-data/* *Process* t_CheckDuration logfiles/*
