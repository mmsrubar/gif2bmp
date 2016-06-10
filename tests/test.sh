#!/bin/sh
. /etc/init.d/functions # bring the action function

memcheck=$(whereis -b valgrind |awk '{print $2}')

#set -x
#set -e

gifs_dir=input
program=gif2bmp
valgrind_flag=false

this_will_fail()
{
    # Do some operations...
    return 1
}

this_will_succeed()
{
    # Do other operations...
    return 0
}

function clean_up {
  rm -rf tmp
  for i in $(find . -name "*.rgb*"); do rm -f $i; done
}

function usage { 
  echo "Usage: $0 [valgrind]"
  exit 1
}

function test_info {
  # Go through all gif files and try to read them. This will test if the gif was
  # successfully read to the memory which means we able to parse it corretly. If
  # the file was parsed corretly then the 0 is returned.

  printf "\nTEST: PARSING CORRECT GIF FILES\n===============================\n"

	if [ "$valgrind_flag" = false ];
	then
		for f in $(ls $gifs_dir/*.gif);
		do
			./$program -t < $f &> /dev/null
			if [ "$?" -eq "0" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	else
		echo "Now with valgrind on"
		for f in $(ls $gifs_dir/*.gif);
		do
			$memcheck --leak-check=full --error-exitcode=1 ./$program -t < $f 2>/dev/null
			if [ "$?" -eq "0" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	fi

  # Go through all incorect gif files and try to read them. Program should
  # return 1 because of wrong gif format.
  printf "\nTEST: PARSING WRONG GIF FILES\n===============================\n"

	if [ "$valgrind_flag" = false ];
	then
		for f in $(ls $gifs_dir/*.error);
		do
			./$program < $f &> /dev/null
			if [ "$?" -eq "1" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	else
		echo "Now with valgrind on"
		for f in $(ls $gifs_dir/*.error);
		do
			$memcheck --leak-check=full --error-exitcode=1 ./$program < $f 2> /dev/null
			if [ "$?" -eq "1" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	fi
}

function test_lzw {
  # Test if the decompression of gif's data compressed with LZW was successful.
  # My program will decompress gif's data and generate raw file which contains
  # rgb values of each pixel in the gif. Final raw file will have size 3 * width
  # * height.  Compare raw files generated from my program with raw outputs of
  # external utility called gif2rgb from the giflib library.

  printf "\nTEST: LZW DECOMPRESSION\n=======================\n"

	if [ "$valgrind_flag" = false ];
	then
		for f in $(ls $gifs_dir/*.gif);
		do
			# use external utility to get rgb files
			/usr/bin/gif2rgb $f > $f.rgb.ok
			# use my program to get rgb files
			./$program -i $f -o $f.rgb -r 2> /dev/null 1> /dev/null
			# compare results
			diff $f.rgb.ok $f.rgb
			if [ "$?" -eq "0" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	else
		echo "Now with valgrind on"
		for f in $(ls $gifs_dir/*.gif);
		do
			# use my program to get rgb files
			$memcheck --leak-check=full --error-exitcode=1 ./$program -i $f -o $f.rgb -r 2> /dev/null
			if [ "$?" -eq "0" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	fi
}

function test_sizes {
  # Test if the size of input GIF file is count correctly and also the size of
	# the output BMP file.

  printf "\nTEST: GIF AND BMP SIZES\n======================\n"

	for f in $(ls $gifs_dir/*.gif);
	do
		log_file="tmp/gif2bmp.log"
    bmp_file="tmp/tmp.bmp"

		# convert gif file to the bmp and save log
		./$program -i $f -o $bmp_file -l $log_file 2> /dev/null 1> /dev/null

		real_gif_size=$(ls -l $f | awk '{print $5}')
		counted_gif_size=$(cat $log_file|grep "^coded"|awk '{print $3}')
		real_bmp_size=$(ls -l $bmp_file | awk '{print $5}')
		counted_bmp_size=$(cat $log_file|grep "uncoded"|awk '{print $3}')

		if [ "$real_gif_size" = "$counted_gif_size" ] && [ "$real_bmp_size" = "$counted_bmp_size" ];
		then
			action "$(basename $f)" this_will_succeed
		else
			action "$(basename $f): $real_gif_size != $counted_gif_size OR $real_bmp_size != $counted_bmp_size" this_will_fail
		fi
	done
}

function test_gif2bmp {
  # Test if the comversion from GIF format to the BMP format was successful.
  # The bmp directory contains bmp files which was created by me and cheched in
  # GIMP.

  printf "\nTEST: GIF TO BMP CONVERSION\n=======================\n"

	if [ "$valgrind_flag" = false ];
	then
		for f in $(ls $gifs_dir/*.gif);
		do
			gif_file=$(basename $f)
			name="${gif_file%.*}"

			# use my program to convert gif file to the bmp
			./$program -i $f -o tmp/$name.bmp 2> /dev/null 1> /dev/null

			if [ -f "bmp/$name.bmp" ];
			then
				# compare results
				diff bmp/$name.bmp tmp/$name.bmp
				if [ "$?" -eq "0" ];
				then
				action "$(basename $f)" this_will_succeed
				else
				action "$(basename $f)" this_will_fail
				fi
			else
				printf " no correct BMP file to compare with ...\n"
				action "$(basename $f)" this_will_fail
			fi
		done
	else
		echo "Now with valgrind on"
		for f in $(ls $gifs_dir/*.gif);
		do
			gif_file=$(basename $f)
			name="${gif_file%.*}"

			# use my program to convert gif file to the bmp
			$memcheck --leak-check=full --error-exitcode=1 ./$program -i $f -o tmp/$name.bmp 2>/dev/null
			if [ "$?" -eq "0" ];
			then
				action "$(basename $f)" this_will_succeed
			else
				action "$(basename $f)" this_will_fail
			fi
		done
	fi
}

# check return value 0 is success and every other is failure
function check {
		if [ "$1" -eq "$2" ];
		then
			action "$3" this_will_succeed
		else
			action "$3" this_will_fail
		fi
}

function test_parameters {
  printf "\nTEST: PROGRAM'S INPUT PARAMETERS\n=============================\n"

	if [ "$valgrind_flag" = false ];
	then
		./$program -h > /dev/null
		check $? 0 "./$program -h"

		./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp 2>/dev/null
		check $? 0 "./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp"

		./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp -l tmp/log 2> /dev/null
		check $? 0 "./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp -l tmp/log"

		./$program -i
		check $? 1 "./$program -i"

		./$program -o
		check $? 1 "./$program -o"
	else
		echo "Now with valgrind on"
		$memcheck --leak-check=full --error-exitcode=1 ./$program -h 2> /dev/null
		check $? 0 "./$program -h"

		$memcheck --leak-check=full --error-exitcode=1 ./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp 2>/dev/null
		check $? 0 "./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp"

		$memcheck --leak-check=full --error-exitcode=1 ./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp -l tmp/log 2> /dev/null
		check $? 0 "./$program < $gifs_dir/jobs.gif > tmp/jobs.bmp -l tmp/log"

		$memcheck --leak-check=full --error-exitcode=1 ./$program -i 2> /dev/null
		check $? 1 "./$program -i"

		$memcheck --leak-check=full --error-exitcode=1 ./$program -o 2> /dev/null
		check $? 1 "./$program -o"
	fi
}

# prepare
#command -v gif2rgb >/dev/null 2>&1 || { echo >&2 "I require foo but it's not installed.  Aborting."; exit 1; }
command -v $memcheck >/dev/null 2>&1 || { echo >&2 "I require valgrind but it's not installed.  Aborting."; exit 1; }
clean_up
mkdir tmp
while [ "$1" != "" ];
do
  case "$1" in
		valgrind) valgrind_flag=true ;;
		bmp) test_gif2bmp; exit 0;;
		#test) test_info; exit 0;;
		test) valgrind_flag=true; test_info; exit 0;;
		lzw) valgrind_flag=true; test_lzw; exit 0;;
		sizes) test_sizes; exit 0 ;;
		p) test_parameters; exit 0;;
		#lzw) test_lzw; exit 0;;
		#bmp) valgrind_flag=true; test_gif2bmp; exit 0;;
    *)    		usage; exit 0;;
  esac

  shift
done

test_parameters
test_info
[ "`command -v gif2rgb`" = "" ] && echo "missing gif2rgb"|| test_lzw
[ "$valgrind_flag" = "false" ] && test_sizes
test_gif2bmp
clean_up
