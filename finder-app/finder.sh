#!/bin/sh
#Store arguments in local variables
filesdir=$1
serachstr=$2

#Check for valid arguments
if [ $# -ne 2 ] 
then
	echo " Invalid arguments"
	echo " Argument 1 <File path>"
	echo " Argument 2 String to be searched "
	exit 1
	
else 
	if [ ! -d "$filesdir" ] 
	then
		echo "Directory Not Found- $filesdir "
		exit 1

	else
	
	#Calculate total files in which string occured
	total_files=$(find "$filesdir" -type f | wc -l)
	
	#Calculate total lines in which string is matched
	total_lines=$(grep -r "$serachstr" "$filesdir" | wc -l)
	fi

#echo total files and total lines in which input string is matched
echo "The number of files are $total_files and the number of matching lines are $total_lines"

fi
