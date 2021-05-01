#!/bin/bash

# Bash autocompletion to Gnuastro. This shell script is intended to load
# itself automatically from the '~/.bashrc' file, modified during
# installation. For more details, see the 'autocomplete feature' under the
# 'developing' chapter of Gnuastro's manual and the comments below.
#
# To debug/test this script, you can simply 'source' it into your running
# terminal.
#
# Original author:
#     Pedram Ashofteh Ardakani <pedramardakani@pm.me>
# Contributing author(s):
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Copyright (C) 2021 Free Software Foundation, Inc.
#
# Gnuastro is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# Gnuastro is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with Gnuastro. If not, see <http://www.gnu.org/licenses/>.




#######################################################################
############      Options and general operating mode       ############
#######################################################################

# GLOBAL VARIABLES
_gnuastro_prefix="/usr/local/bin";




# Basic initialization.
_gnuastro_autocomplete_initialize(){

    # Initialize the completion response with null
    COMPREPLY=();

    # Variable "current", is the current word being completed. "$2" is the
    # default value for the current word in completion scripts. But we are
    # using the longer form: "${COMP_WORDS[COMP_CWORD]}" for clarity.
    current="${COMP_WORDS[COMP_CWORD]}"
    if [ "$current" = "=" ]; then

        # The equal sign '=' raises complexities when filling suggestions
        # for long options. Things will work out fine when they are simply
        # ignored.
        current=""

    fi

    # Variable "prev", is one word before the one being completed. By
    # default, this is set as "$3" in completion scripts. But we are using
    # the longer form: "${COMP_WORDS[COMP_CWORD-1]}" to avoid confusions
    # with the arguments of our internal functions. Note that Bash will
    # return the '=' sign as a separate component of the line, so in that
    # case, we want the second-last word.
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    if [ "$prev" = "=" ]; then

        # While a user is writing a long option's argument, the previous
        # word will be the equal sign '='. This is not helpful at all. But
        # looking at a word just before '=' helps us understand which long
        # option is being called upon.
        prev="${COMP_WORDS[COMP_CWORD-2]}"

    fi
}





# List all the options of the given program (an '=' is kept for the options
# that need a value). In the output of '--help', option lines have these
# properties:
#
#   - The full line starts with two empty spaces.
#   - The first non-white character is a '-'.
#   - It contains a long option formated like '--XXXX'.
#
# But some options have a short version (which we ignore in
# autocompletion), so if the first word ends with a ',' the option name we
# want to show is the second word.
_gnuastro_autocomplete_option_list(){
    options_all=$("${COMP_WORDS[0]}" --help \
                      | awk '/^  / && $1 ~ /^-/ && /--+[a-zA-Z0-9]*/ { \
                              if($1 ~ /,$/) name=$2; \
                              else          name=$1; \
                              print name}' \
                      | sed -e's|=.*|=|');
}





# Return successfully if the previous token (specified as first argument)
# is an option that requires a value and store the option name if it is.
#
#   INPUT:
#     1) [as argument] String to search for.
#     *) [as variable] 'options_all' (in case the list of options is
#                      already read)
_gnuastro_autocomplete_string_is_valued_option(){

    # For easy reading.
    local string=$1

    # If the first character of the string isn't a '-', then its not an
    # option and there is no need to do any futher checks (and slow down
    # the output) so we can safely return failure.
    if [ ${string:0:1} != "-" ]; then return 1; fi

    # List of option names (with an '=' after those that need a value.
    if [ x"$options_all" = x ]; then
        _gnuastro_autocomplete_option_list
    fi

    # Go over the option list and see if they match the '$1'.
    for option in $options_all; do
        if [[ $option = $string=* ]]; then return 0; fi
    done

    # If control reaches here, then return failure (1).
    return 1
}





# See if the current word is an argument or option value.
_gnuastro_autocomplete_mode_arg_optval(){

    # If the previous token is the first token, then this is an
    # argument, no need for any further checks.
    if [ $prev = "${COMP_WORDS[0]}" ]; then
        argument=$current

    # If the previous token is an option that needs a value, then this is
    # an option value, this function will set 'option_name' and
    # 'option_name_complete' if necessary.
    elif _gnuastro_autocomplete_string_is_valued_option $prev; then
        option_name=$prev
        option_value=$current
        option_name_complete=1

    # The previous token wasn't an option that required a value, so this is
    # an argument.
    else
        argument=$current
    fi
}





# See if this is an argument, option name or option value. This function
# will fill 'argument', 'option_name' and 'option_value' (they should be
# initialized to an empty string before it).
#
#  option_value=FULL:            We are busy completing an option value.
#  option_name=FULL:             Can mean different meanings.
#    {
#      option_name_complete==0:  We are still filling the option name.
#      option_name_complete==1:  We are starting the option values.
#    }
#  argument=FULL:                We are busy filling an argument.
#  argument=EMPTY:               We haven't started writing an argument yet.
_gnuastro_autocomplete_mode(){

    # Local variable necessary only in this function.
    local namevalue=""

    # If the current word is empty, it may be an argument, or value of an
    # option (in case a value-required option is given before).
    if [ x$current = x ]; then
        _gnuastro_autocomplete_mode_arg_optval

    # The current word isn't empty.
    else

        # If the current word starts with a '-', it is an option name or
        # 'name=value' pair.
        if [ ${current:0:1} = "-" ]; then

            # If there is an equal sign, then we should separate the option
            # name from the value and keep
            if [[ $current = *=* ]]; then

                # By setting the "internal field separator" (IFS) to '='
                # and using 'read', we can separate the strings before and
                # after the equal sign.
                IFS="=" read -ra namevalue <<< $current
                option_name=${namevalue[0]}
                option_name_complete=1

                # If the value isn't written yet, (for example '--hdu='),
                # then the second string will just be an '='. But no value
                # is given yet, so 'option_value' should be empty.
                option_value=${namevalue[1]}
                if [ x$option_value = x"\=" ]; then option_value=""; fi
            else
                option_name=$current
                option_name_complete=0
            fi

        # The current word didn't start with a '-', so it may be an
        # argument or option value.
        else
            # Bash may separate the '=' in 'name=value' tokens. In this
            # scenario, when the user only gives 'name=' and presses TAB,
            # then 'current' will be '='. In this case, we should just set
            # it to empty.
            if [ $current = "=" ]; then current=""; fi

            # Check to see if its an argument or option value.
            _gnuastro_autocomplete_mode_arg_optval
        fi

    fi
}




# Given a certain option (as first argument), find the value that the user
# has given for it.
#
# OUTPUT:
#   read_option_value
_gnuastro_autocomplete_read_option_value(){

    # Inputs:
    local read_option_name=$1

    # Initialize the output (defined as 'local' before this).
    read_option_value=""

    # Parse through the given command-line and find the value.
    local option_found=0
    for word in ${COMP_WORDS[*]}; do

        # Ignore the program name (first word), current (last) word and any
        # '=' signs.
        if [ x$word = x${COMP_WORDS[0]} ] \
               || [ x$word = x$current ] \
               || [ x$word = x"=" ]; then
            local just_a_place_holder=1
        else
            # If the 'option_found' flag is set, this is the answer, set it
            # and return (this has to be *before* the place that we set
            # 'option_found').
            if [ $option_found = 1 ]; then
                read_option_value="$word";
                return 0
            fi

            # If this word is the desired option, set the 'option_found'
            # flag so the next word is taken as the value.
            if [ x$word = x$read_option_name ]; then option_found=1; fi
        fi
    done
}




















#######################################################################
############                     Files                     ############
#######################################################################

# Check if the given file is a FITS file (that can actually be
# opened). Note that FITS files have many possible extensions (see the
# 'gal_fits_name_is_fits' function in 'lib/fits.c').
_gnuastro_autocomplete_is_fits(){
    if "$_gnuastro_prefix"/astfits "$1" -h0 &> /dev/null; then return 0;
    else                                                       return 1;
    fi
}





# Return successfully if argument (a FITS file) has a image HDU.
_gnuastro_autocomplete_fits_has_image(){
    if _gnuastro_autocomplete_is_fits "$1"; then
        if [ $("$_gnuastro_prefix"/astfits "$1" --hasimagehdu) = 1 ]; then
            return 0
        fi
    fi
    return 1
}





# Return successfully if argument (a FITS file) has a table HDU.
_gnuastro_autocomplete_fits_has_table(){
    if _gnuastro_autocomplete_is_fits "$1"; then
        if [ $("$_gnuastro_prefix"/astfits "$1" --hastablehdu) = 1 ]; then
            return 0
        fi
    fi
    return 1
}





# Return successfully if first argument is plain-text (not binary).
_gnuastro_autocomplete_is_plaintext(){
    if file "$1" | grep 'executable\|binary' &> /dev/null; then return 1;
    else                                                        return 0;
    fi
}





# Return successfully (with 0) if the given non-FITS file is a table.
_gnuastro_autocomplete_is_plaintext_table(){

    # Only do the check if the file exists.
    if [ -f $1 ]; then

        # If the file is not plain-text, it will contain an 'executable' or
        # 'binary' in the output of the 'file' command.
        if _gnuastro_autocomplete_is_plaintext "$1"; then

            # The file is plain-text. Extract the first non-commented or
            # empty line and feed it to 'asttable' to see if it can be
            # interpretted properly. We don't want to bother with the other
            # lines, because we don't want to waste computational power
            # here.
            if awk '!/^#/ && NF>0 {print; exit 0}' "$1" \
                    | "$_gnuastro_prefix"/asttable &> /dev/null; then
                return 0
            else
                return 1
            fi

            # The file was binary
        else return 1
        fi

    # The file didn't exist.
    else return 1
    fi
}





# Return successfully if the first argument is a table.
_gnuastro_autocomplete_is_table(){
    if   _gnuastro_autocomplete_fits_has_table     $1; then return 0
    elif _gnuastro_autocomplete_is_plaintext_table $1; then return 0
    else                                                    return 1
    fi
}





# If a table is already given in the previous tokens, return successfully
# (with zero), and will put its name in 'given_file'. Otherwise, return a
# failure (1) and 'given_file' will be untouched.
_gnuastro_autocomplete_first_in_arguments(){

    # Inputs
    local mode=$1

    # Local variables (that are only for this function).
    local word=""
    local previous=""

    # Initialize outputs
    given_file=""

    # Go over all the words/tokens given until now.
    for word in ${COMP_WORDS[*]}; do

        # Ignore the program name (first word), current (last) word, any
        # directories or '=', or any word that starts with a '-' (which is
        # an option).
        if [ x$word = x${COMP_WORDS[0]} ] \
               || [ x$word = x$current ] \
               || [ ${word:0:1} = "-" ] \
               || [ x$word = x"=" ] \
               || [ -d $word ]; then
            local just_a_place_holder=1
        else
            # If the previous word is a valued option, then it shouldn't be
            # checked.
            if _gnuastro_autocomplete_string_is_valued_option $previous; then
                local just_a_place_holder=1
            else

                # Based on the mode, do the proper check.
                if [ $mode = table ]; then
                    if _gnuastro_autocomplete_is_table $word; then
                        given_file=$word
                        return 0;
                    fi
                else
                    if _gnuastro_autocomplete_fits_has_image $word; then
                        given_file=$word
                        return 0;
                    fi
                fi
            fi
        fi

        # If this word isn't an '=', put it in 'previous' and go onto the
        # next word.
        if [ $word != "=" ]; then
            previous=$word
        fi
    done

    # If control reached here, then there weren't any tables on the
    # command-line until now.
    return 1;
}





# Find the requested table from the entered command-line.
#
# INPUT ARGUMENTS:
#    1) Mode of file ('table' or 'image').
#    2) Name of option containing file names.
#
# WRITTEN VARIABLES (should be defined before this function).
#     given_file: file name of given table.
_gnuastro_autocomplete_given_file(){

    # Set inputs (for each readability).
    local mode="$1"
    local name="$2"

    # If 'name' is emtpy, we should look in the arguments, otherwise, we
    # should loook into the options.
    if [ x"$name" = x ]; then
        if _gnuastro_autocomplete_first_in_arguments $mode; then
            # given_file is written by the function as a side-effect.
            local just_a_place_holder=1
        fi
    else
        _gnuastro_autocomplete_read_option_value "$name"
        given_file="$read_option_value"
    fi

}





# Find the requested table within the already entered command-line. This
# option needs two arguments:
#
# INPUT ARGUMENTS
#    1) Mode of file ('table' or 'image').
#    2) The filename option (if empty string, 1st argument).
#    3) The HDU option (only necessary if the file is FITS).
#
# WRITTEN VARIABLES (should be defined before this function).
#    given_file: file name of given table/image.
#    given_hdu:  HDU of given table/table.
_gnuastro_autocomplete_given_file_and_hdu(){

    # Set inputs (for each readability).
    local mode=$1
    local name=$2
    local hduoption=$3

    # Internal variables.
    local read_option_value=""

    # Initialize the outputs (defined before this).
    given_hdu=""
    given_file=""

    # First, confirm the table file name. If requested table is in an
    # argument, 'name' will be empty.
    _gnuastro_autocomplete_given_file $mode $name

    # If a file name existed and it is a FITS file, find the HDU given in
    # the option.
    if [ x"$given_file" != x ] \
           && _gnuastro_autocomplete_is_fits "$given_file"; then
        _gnuastro_autocomplete_read_option_value $hduoption
        given_hdu="$read_option_value"
    fi
}




















#######################################################################
############               Completion replies              ############
#######################################################################

# Add completion replies for the values to '--searchin'.
_gnuastro_autocomplete_compreply_searchin(){
    for v in name unit comment; do COMPREPLY+=("$v"); done
}





# Add completion replies for the values to '--searchin'.
_gnuastro_autocomplete_compreply_tableformat(){
    for v in fits-ascii fits-binary txt; do COMPREPLY+=("$v"); done
}





# Add matching options to the completion replies.
_gnuastro_autocomplete_compreply_options_all(){

    # Variable only necessary here.
    local options_match=""

    # Get the list of option names (with an '=' after those that need a
    # value (if 'options_all' isn't already set)
    if [ x"$options_all" = x ]; then
        _gnuastro_autocomplete_option_list
    fi

    # Limit the options to those that start with the already given portion.
    if [ x$1 = x ]; then
        options_match=$options_all
    else
        # We aren't using 'grep' because it can confuse the '--XXX' with
        # its own options on some systems (and placing a '--' before the
        # search string may not be portable).
        options_match=$(echo "$options_all" | awk '/^'$1'/')
    fi

    # Add the list of options.
    for f in $options_match; do COMPREPLY+=("$f"); done

    # Disable the extra space on the command-line after the match, only for
    # this run (only relevant when we have found the match).
    compopt -o nospace
}





# Add a file into the completion replies. The main point is to remove the
# fixed directory name prefix of the file (that is appended by 'ls').
#
# It takes two arguments:
#
#   1. Base string (that was fed into 'ls' to find the full string).
#      This string CAN BE EMPTY.
#   2. Full string.
#
# If the first is a full directory, then it will remove it from the full
# string before saving string (which is standard in autocomplete (the user
# has already given it and it is just annoying!).
_gnuastro_autocomplete_compreply_file(){
    if [ x$1 != x ] && [ -d $1 ]; then COMPREPLY+=("${2#$1}")
    else                               COMPREPLY+=("$2")
    fi
}





# Add all the FITS files that contain an image in the location pointed to
# by first argument into the completion replies.
_gnuastro_autocomplete_compreply_fits_images(){

    # Get list of matching files.
    #   with '-d' we are telling 'ls' to not go into sub-directories.
    local files=($(ls -d "$1"* 2> /dev/null))

    # Parse the list of files and add it when it is a directory or it can
    # be read as a table.
    for f in ${files[*]} ; do
        if [ -d "$f" ]; then COMPREPLY+=("${f#$1}/");
        elif _gnuastro_autocomplete_fits_has_image "$f"; then
            _gnuastro_autocomplete_compreply_file "$1" "$f"
        fi
    done
}





# Add all the HDUs that contain a table/image in the first argument (a FITS
# file) into the completion replies.
#
# INPUT ARGUMENTS
#    1) Mode of file ('table' or 'image').
#    2) Name of file.
_gnuastro_autocomplete_compreply_hdus(){
    if _gnuastro_autocomplete_is_fits "$2"; then
        for h in $("$_gnuastro_prefix"/astfits "$2" --list"$1"hdus); do
            COMPREPLY+=("$h")
        done
    fi
}





# Add all the tables in the location pointed to by first argument into the
# completion replies.
_gnuastro_autocomplete_compreply_tables(){

    # Get list of matching files.
    #   with '-d' we are telling 'ls' to not go into sub-directories.
    local files=($(ls -d "$1"* 2> /dev/null))

    # Parse the list of files and add it when it is a directory or it can
    # be read as a table.
    for f in ${files[*]} ; do
        if [ -d "$f" ]; then COMPREPLY+=("${f#$1}/");
        elif _gnuastro_autocomplete_is_table "$f"; then
            _gnuastro_autocomplete_compreply_file "$1" "$f"
        fi
    done
}





# Add all table columns in given file (possibly with HDU) that start with
# the given string into the completion replies.
_gnuastro_autocomplete_compreply_table_columns(){

    # Inputs
    local table_file="$1"
    local table_hdu="$2"
    local tomatch="$3"

    # Internal
    local columns=""
    local hdu_option=""

    # If a HDU is given, then add it to the options.
    if [ x"$table_hdu" != x ]; then hdu_option="--hdu=$table_hdu"; fi

    # Get the list of columns from the output of '--information': the
    # column names are the second column of the lines that start with a
    # number. If there is no column name, print the column number.
    #
    # We are forcing 'awk' to read after the second line of 'asttable'
    # output, because the second line contains the filename. The filename
    # might start with numbers. If so, there will be an unwanted '(hdu:'
    # printed in the results. Here, 'awk' will print the second column in
    # lines that start with a number.
    columns=$("$_gnuastro_prefix"/asttable --information \
                                 "$table_file" $hdu_option \
                  | awk 'NR>2 && /^[0-9]/ { \
                            if($2=="n/a") print $1; else print $2 \
                         }' \
                  | grep ^$tomatch)

    # Add the columns into the completion replies.
    for c in $columns; do COMPREPLY+=("$c"); done
}


















#######################################################################
############         Only for Table (this program)         ############
#######################################################################

# Dealing with arguments: Table only takes one argument/file. So if a table
# has been previously given on the command-line only print option names.
_gnuastro_autocomplete_asttable_arguments(){
    local given_file=""
    if _gnuastro_autocomplete_first_in_arguments table; then
        _gnuastro_autocomplete_compreply_options_all ""
    else
        _gnuastro_autocomplete_compreply_tables "$argument"
    fi
}





# Fill option value (depends on option).
_gnuastro_autocomplete_asttable_option_value(){

    # Internal variables.
    local fits_file=""
    local given_hdu=""
    local given_file=""

    # Keep this in the same order as the output of '--help', for options
    # with similar operations, keep the order within the '|'s.
    case "$option_name" in

        # Options that need a column from the main argument.
        -b|--noblank|-c|--column|--inpolygon|--outpolygon)
            _gnuastro_autocomplete_given_file_and_hdu table "" --hdu
            _gnuastro_autocomplete_compreply_table_columns \
                "$given_file" "$given_hdu" "$current"
            ;;

        # Options that take the column name as first component of value.
        -m|--colmetadata|-e|--equal|-n|--notequal)

            # Get the main argument's name (and possible HDU).
            _gnuastro_autocomplete_given_file_and_hdu table "" --hdu
            _gnuastro_autocomplete_compreply_table_columns \
                "$given_file" "$given_hdu" "$current"

            # Since these options take a column name as first value and the
            # user should continue with other details, we need to disable
            # the extra space on the command-line after the successful
            # match.
            compopt -o nospace
            ;;

        -C|--catcolumns)
            _gnuastro_autocomplete_given_file_and_hdu \
                table --catcolumnfile --catcolumnhdu
            _gnuastro_autocomplete_compreply_table_columns \
                "$given_file" "$given_hdu" "$current"
            ;;

        -h|--hdu)
            _gnuastro_autocomplete_given_file table ""
            _gnuastro_autocomplete_compreply_hdus table "$given_file"
            ;;

        -L|--catcolumnfile)
            _gnuastro_autocomplete_compreply_tables "$current"
            ;;

        --searchin)
            _gnuastro_autocomplete_compreply_searchin
            ;;

        -u|--catcolumnhdu)
            _gnuastro_autocomplete_given_file table --catcolumnfile
            _gnuastro_autocomplete_compreply_hdus table "$given_file"
            ;;

        -w|--wcsfile)
            _gnuastro_autocomplete_compreply_fits_images "$current"
            ;;

        -W|--wcshdu)
            _gnuastro_autocomplete_given_file image --wcsfile
            _gnuastro_autocomplete_compreply_hdus image "$given_file"
            ;;

        --tableformat)
            _gnuastro_autocomplete_compreply_tableformat
            ;;
    esac
}





_gnuastro_autocomplete_asttable(){

    # Basic initialization. The variables we want to remain inside this
    # function are given a 'local' here and set inside the 'initialize'
    # function. The variables are defined above the function that gives
    # them a value.
    local prev=""
    local current=""
    local argument=""
    _gnuastro_autocomplete_initialize

    # For a check
    #echo
    #echo "prev:     $prev"
    #echo "current:  $current"
    #echo "argument: $argument"

    # Extract the current mode (if the user is giving an argument, option
    # name, or option value). See the description above this function on
    # how the mode is set.
    local options_all=""
    local option_name=""
    local option_value=""
    local option_name_complete=0
    _gnuastro_autocomplete_mode

    # For a check
    #echo
    #echo "argument:             $argument"
    #echo "option_name:          $option_name"
    #echo "option_name_complete: $option_name_complete"
    #echo "option_value:         $option_value"

    # If 'option_name_complete==1', then we are busy filling in the option
    # value.
    if [ $option_name_complete = 1 ]; then
        _gnuastro_autocomplete_asttable_option_value

    # When 'option_name' is not empty (and not yet complete), we are busy
    # filling in the option name.
    elif [ x$option_name != x ]; then
        _gnuastro_autocomplete_compreply_options_all "$option_name"

    # In the case of "none-of-the-above", it is an argument.
    else
        _gnuastro_autocomplete_asttable_arguments
    fi
}





# Define the completion specification, or COMPSPEC: -o bashdefault: Use
# Bash default completions if nothing is found.  -F function: Use this
# 'function' to generate the given program's completion.
complete -o bashdefault -F _gnuastro_autocomplete_asttable asttable
