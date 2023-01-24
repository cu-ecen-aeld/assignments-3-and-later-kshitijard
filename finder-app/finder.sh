#!/bin/bash

# file: finder.sh
# This script checks for a given string within a given filesystem


#Input arguments
filesdir=$1
searchstr=$2


# Checking for a valid number of arguments
if [ $# -ne 2 ]
then
	echo "Invalid number of arguments. Total number of arguments shoudl be 2"
	exit 1
fi

files_count=0
lines_count=0

# Checking whether the given file directory exists
if [ -d "$filesdir" ]
then 
	echo "success"
	files_count=` find ${filesdir} -type f | wc -l`
	lines_count=`grep -r ${searchstr} ${filesdir} | wc -l`
	echo "The number of files are ${files_count} and the number of matching lines are ${lines_count}"
	exit 0
# Error printed in case the path is not found 
else
	echo "error: file directory does not exist"
	exit 1
fi
