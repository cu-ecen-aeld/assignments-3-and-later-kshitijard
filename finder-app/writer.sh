#!/bin/bash


# file: writer.sh
# This script creates a file at the specified location with the specified text

#Input arguments
writefile=$1
writestr=$2

if [ $# -ne 2 ]
then
	echo "Invalid number of arguments. Total number of arguments shoudl be 2"
	exit 1
fi

directory=` dirname ${writefile}`

if [ ! -d "$directory" ]
then
	echo "directory does not exist."
	mkdir -p ${directory}
fi

if touch ${writefile}
then
	echo ${writestr} > ${writefile}
else
	echo "file could not be created"
	exit 1
fi
