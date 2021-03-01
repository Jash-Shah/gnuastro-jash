#!/bin/sh

# Add bash autocompletion to Gnuastro. This shell script is intended to
# load itself automatically from the '~/.bashrc' file, modified during
# installation. For more details, see the 'autocomplete feature' under the
# 'developing' section of Gnuastro's manual and the comments below.
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







# In the developing phase, call the command below to try autocompletion in
# action:
#
# $ source completion.sh






# GLOBAL VARIABLES
<<<<<<< HEAD
db=1
PREFIX="/usr/local/bin";
ASTFITS="$PREFIX/astfits";
ASTTABLE="$PREFIX/asttable";
db=0 # Set 0 for printing debug messages, else set to 1
=======
_gnuastro_completion_debug=1
_gnuastro_prefix="/usr/local/bin";
_gnuastro_astfits="$_gnuastro_prefix/astfits";
_gnuastro_asttable="$_gnuastro_prefix/asttable";
>>>>>>> e18d46e4 (bin/table/completion.bash: improvements to find good table name)





# Accepts a FITS filename as input and echoes its headers.
_gnuastro_autocomplete_get_fits_hdu(){
    if [ -f "$1" ]; then
        $_gnuastro_astfits --quiet "$1" | awk '{print $2}'
    fi
}





# Accept either an array or a string '$1' split by normal bash conventions,
# check if second argument '$2' is present in the suggestions, if so, put
# it in suggestions, continue otherwise. Note, in case the second agument
# is not passed, no filteration will be applied.
_gnuastro_autocomplete_compgen(){
    for w in $1
    do
        # The right-hand side of '[[ ... =~ ... ]]' is considered a regex
        # and should not be quoted or it will be taken as a literal.
        # TODO: Let 'sed' or 'awk' do the regex matching if it screws up
        # the portability.
        [[ "$w" =~ $2 ]] && COMPREPLY+=( "$w" )
    done

    # Uncomment if you want to accept user's current word as a completion
    # [[ -z "${COMPREPLY[*]}" ]] && COMPREPLY+=( "$2" )
}





# Checks for the current fits file and puts its headers into completion
# suggestions
_gnuastro_autocomplete_list_fits_hdu(){
    if [ -f "$1"  ]; then
        list=("$(_gnuastro_autocomplete_get_fits_hdu "$1")")
        # A custom enhancement for the 'compgen' command. This version will
        # have no problem with the dash sign '-'. Because sometimes the
        # 'hdu' names might contain dash symbols in them. This ensures that
        # all of them are suggested.
        _gnuastro_autocomplete_compgen "${list[@]}" "$word"
        # TODO: If using `local` is allowed, remove the `unset` commands
        # and declare variables as `local` instead
        unset list
    fi
}





# 'Append' all 'FITS' files in current directory to suggestions. Case
# insensitive.  The -X option and its filter pattern are explained on bash
# programmable completion info page: $ info bash programmable 'Appending'
# seems a good idea because the program might accept multiple input types.
#
# For example the 'asttable' program can either accept a fits file or
# various short/long options as its first argument. In this case,
# autocompletion suggests both.
#
# The completion can not suggest filenames that contain white space in them
# for the time being.
_gnuastro_autocomplete_list_fits_names(){
    # List all files in the current directory. Filter out those that start
    # with the current word in the command line '$word'. Note that the grep
    # option '--color=never' has to be there to prevent passing special
    # characters into '$COMPREPLY'.
    local files=($(ls | grep -e "^$word" --color=never))
    for f in ${files[*]} ; do
        if astfits $f &> /dev/null; then COMPREPLY+=("$f"); fi
    done
}




# Prompt the user that this option only accepts a number
_gnuastro_autocomplete_expect_number(){
    echo "Pass"
}





# Check if the given file is a FITS file (that can actually be
# opened). Note that FITS files have many possible extensions (see the
# 'gal_fits_name_is_fits' function in 'lib/fits.c').
_gnuastro_autocomplete_file_is_fits(){
    if astfits $1 -h0 &> /dev/null; then return 0; else return 1; fi
}





# This function is given the long and short formats of an option (as first
# and second arguments). If the option has been called until now, and has
# been given a value, its value will be returned. Note that if it is called
# multiple times, only the last occurance is used. It should be used like
# this:
#
#    _gnuastro_autocomplete_option_value --hdu -h
_gnuastro_autocomplete_option_value(){
    option_value=""
    local longname=$1
    local shortname=$2
    for i in $(seq 1 ${#COMP_WORDS[@]}); do

        # To keep things readable, put this token into a variable.
        local token=${COMP_WORDS[i]}

        # First, the easy long-format scenario.
        if [ x$token = x$longname ]; then
            if [ x${COMP_WORDS[i+1]} = x"=" ]; then
                option_value="${COMP_WORDS[i+2]}"
            else
                option_value="${COMP_WORDS[i+1]}"
            fi

        # The short format. In the short format, the option is defined by
        # its first two characters (the first is a '-' and the second can
        # be any ASCII character. We don't have any '=' sign, however, the
        # value may be touching the option (into one token), for example
        # '-c1' and '-c 1' are the same.
        #
        # TODO: We still have to account for cases where there are short
        # options with no values in the middle. For example assume that we
        # also have option '-a' that is just an on-off option. In this
        # case, according to the GNU Coding Standards, it is fine to say
        # something like this: '-ac1'. This isn't too common, so in this
        # first implementation, we are ignoring it due to lack of
        # time. We'll add it later.
        else
            local first=${token:0:2}
            if [ x"$first" = x"$shortname" ]; then
                if [ x"$token" =  x"$first" ]; then
                    option_value="${COMP_WORDS[i+1]}"
                else
                    option_value=$(echo $token | sed 's|'${token:0:2}'||')
                fi
            fi
        fi
    done
}





# Return 1 if the given non-FITS file is a table.
_gnuastro_autocomplete_plaintext_is_table(){

    # For easy reading.
    local inputfile="$1"

    # If the file is not plain-text, it will contain an 'executable' or
    # 'binary' in the output of the 'file' command.
    if file $inputfile \
            | grep 'executable\|binary' &> /dev/null; then
        return 1
    else
        # The file is plain-text. Extract the first non-commented or empty
        # line and feed it to 'asttable' to see if it can be interpretted
        # properly. We don't want to bother with the other lines, because
        # we don't want to waste computational power here.
        if awk '!/^#/ && NF>0 {print; exit 0}' $inputfile \
                | asttable &> /dev/null; then
            return 0
        else
            return 1
        fi
    fi
}





# Reverse the list of existing strings on the command-line (so the last
# word becomes the first), then check if it is an acceptable Table file in
# there.
_gnuastro_autocomplete_last_table(){

    # Output variables.
    last_table=""
    last_table_hdu=""
    last_table_hdu_auto=0

    # Internal variables.
    local token=""
    local option_value=""

    # Parse the tokens in reverse.
    for token in $(echo "${COMP_WORDS[@]}" \
                    | awk '{for(i=NF;i>1;--i) printf "%s ", $i}')
    do
        # First, make sure it is an existing file.
        if [ -f $token ]; then

            # It is a FITS file.
            if _gnuastro_autocomplete_file_is_fits $token; then

                # See if a HDU has been given or not. If it hasn't been
                # given, its safe to assume the first HDU. In this case,
                # we'll set the 'last_table_hdu_auto' variable to 1 (to
                # help in debugging in later steps).
                _gnuastro_autocomplete_option_value --hdu -h
                if [ x"$option_value" = x ]; then
                    last_table_hdu=1
                    last_table_hdu_auto=1
                else
                    last_table_hdu="$option_value"
                fi

                # If this extension of this file is actually a FITS table
                # (not an image), then set the 'last_table' variable and
                # break out of the loop that parses the tokens.
                if asttable $token -h$last_table_hdu -i &> /dev/null; then
                    last_table="$token"
                    break;
                fi

            # Not a FITS file.
            else
                if _gnuastro_autocomplete_plaintext_is_table $token; then
                    last_table="$token"
                    break;
                fi
            fi
        fi
    done
}





# Checks if the argument contains a valid file. Does not check for its
# extension. Then, reads the column names using the asttable program and
# echoes the resulting STR.
_gnuastro_autocomplete_get_fits_columns(){
    if [ -f "$1" ]; then
        # Force 'awk' to read after the second line of 'asttable' output,
        # because the second line contains the filename. The filename might
        # start with numbers. If so, there will be an unwanted '(hdu:'
        # printed in the results. Here, 'awk' will print the second column
        # in lines that start with a number.
        "$_gnuastro_asttable" --information "$1" \
            | awk 'NR>2' \
            | awk '/^[0-9]/ {print $2}'
    fi
}




# Accept a fits file name as the first argument ($1). Read and suggest its
# column names. If the file does not exist, pass.
_gnuastro_autocomplete_list_fits_columns(){
    if [ -f "$1" ]; then
        list=("$(_gnuastro_autocomplete_get_fits_columns "$1")")
        _gnuastro_autocomplete_compgen "${list[@]}"
        unset list
    fi
}





# The last file name (.txt/.fits) in COMP_LINE
_gnuastro_autocomplete_get_file(){
    echo "Pass"
}




# Accept the command name and its absolute path, run the --help option and
# 'append' all long options to the current suggestions. 'Appending' seems a
# good idea because the program might accept multiple input types. For
# example the 'asttable' program can either accept a fits file or various
# short/long options as its first argument. In this case, autocompletion
# suggests both.
_gnuastro_autocomplete_list_options(){
    list=("$("$1" --help \
                  | awk -v regex=" --+[a-zA-Z0-9]*=?" \
                        'match($0, regex) \
                           {print substr($0, RSTART, RLENGTH)}')")
    _gnuastro_autocomplete_compgen "${list[@]}" "$word"
    unset list
}





# Prints the message taken as $1 and asks for user action. Then reprints
# the former prompt, which lives in $COMP_LINE.
#
# TODO: The newly printed prompt was taken from here:
# https://stackoverflow.com/questions/22322879/how-to-print-current-bash-prompt
# This is only available since Bash 4.4 (September 2016). We should find
# A more low-level/basic operation.
_gnuastro_autocomplete_print_message(){
    if ! [ x"$1" = x ]; then
        printf "\n$1\n"
        printf "${PS1@P} %s" "$COMP_LINE"
    fi
}




_gnuastro_asttable_completions(){
    # Basic definitions.
    PROG_NAME=$1

    # Initialize the completion response with null
    COMPREPLY=();

    # Strings used in multiple places.
    local infowarning="With '--information' (or '-i') all other options are disabled, you can press ENTER now."

    # Variable "word", is the current word being completed. "$2" is the
    # default value for the current word in completion scripts. But we are
    # using the longer form: "${COMP_WORDS[COMP_CWORD]}" for clarity.
    word="${COMP_WORDS[COMP_CWORD]}"
    if [ "$word" = "=" ]; then
        # The equal sign '=' raises complexities when filling suggestions
        # for long options. Things will work out fine when they are simply
        # ignored.
        word=""
    fi

    # Variable "prev", is one word before the one being completed. By
    # default, this is set as "$3" in completion scripts. But we are using
    # the longer form: "${COMP_WORDS[COMP_CWORD-1]}" for clarity.
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    if [ "$prev" = "=" ]; then
        # While a user is writing a long option's argument, the previous
        # word will be the equal sign '='. This is not helpful at all. But
        # looking at a word just before '=' helps us understand which long
        # option is being called upon.
        prev="${COMP_WORDS[COMP_CWORD-2]}"
    fi

    # If a table has been called until this stage, extract it. The table
    # name itself will be put in 'last_table' and its possible HDU (if its
    # was a FITS table), will be put in 'last_table_hdu'.
    _gnuastro_autocomplete_last_table


    case "$prev" in
        asttable)
            _gnuastro_autocomplete_list_fits_names
            _gnuastro_autocomplete_list_options $PROG_NAME
            ;;
        -i|--information)
            # when a file has been given before this, and the
            # '--information' option is called, we should tell the user to
            # avoid trying new options and just press ENTER. Otherwise, we
            # should let the user choose a FITS table (to view its
            # information).
            if [ x"$last_table" = x ]; then
                _gnuastro_autocomplete_list_fits_names
            else
                _gnuastro_autocomplete_print_message "$infowarning"
                COMPREPLY=()
            fi
            ;;
        -L|--catcolumnfile|-w|--wcsfile)
            # Only suggest a fits filename
            _gnuastro_autocomplete_list_fits_names
            ;;
        -c|--column|-r|--range|-s|--sort|-C|--catcolumns| \
            -m|--colmetadata|--inpolygon|--outpolygon| \
            -e|--equal|-n|--notequal|-b|--noblank|--searchin)
            # The function below returns the columns inside the last fits
            # file specified in the commandline. If no fits files were
            # detected, there will be no response from autocompletion. This
            # might alert the user that something is going wrong.
            _gnuastro_autocomplete_list_fits_columns "$last_table"
            ;;
        -W|--wcshdu|-u|--catcolumnhdu|-h|--hdu)
            # Description is same as the '--column' option.
            _gnuastro_autocomplete_list_fits_hdu "$last_table"
            ;;
        -o|--output|--polygon|-H|--head|-t|--tail| \
            --onlyversion|-N|--numthreads|--minmapsize)
            # Do not suggest anything.
            ;;
        --config)
            # Suggest config files
            COMPREPLY=($(compgen -f -X "!*.[cC][oO][nN][fF]" -- "$word"))
            ;;
        *)
            # Check the entire prompt line $COMP_LINE, if the '-i' or
            # '--information' option is passed and there is a valid fits
            # file present in the command line, prompt user that they can
            # safely press ENTER since this configuration disables all
            # other options. Otherwise, just print all available options.
            if echo "$COMP_LINE" \
                    | grep -e ' --information' -e ' -i' &> /dev/null \
                    &&  [ -f "$last_table" ]; then
                _gnuastro_autocomplete_print_message "$infowarning"
                COMPREPLY=()
            else
                _gnuastro_autocomplete_list_options $PROG_NAME
            fi
            ;;
    esac

    if [[ "${COMPREPLY[@]}" =~ "=" ]]; then
        # Do not append 'space' character to the end of line in case there
        # is a long option present in the suggestions. Please note that
        # long options mostly have a '=' suffix, for example '--hdu=1'.
        compopt -o nospace
    fi

    # Be verbose in debugging mode, where $db is set to '0'.
    if [ $_gnuastro_completion_debug = 0 ]; then
        cat <<EOF

************ DEBUG ************
>>> prev: '$prev' -- \$3: '$3'
>>> word: '$word' -- \$2: '$2'
>>> last_table: '$last_table'
>>> PROG_NAME: $PROG_NAME
>>> COMP_WORDS: '${COMP_WORDS[@]}'
>>> COMPREPLY: '${COMPREPLY[@]}'
*******************************
EOF
        # Printf should stay outside the 'heredoc' above to prevent an
        # extra newline. As a result, the cursor stays on the same line.
        printf ">>> Line: %s" "$COMP_LINE"
    fi

}




# Define the completion specification, or COMPSPEC: -o bashdefault: Use
# Bash default completions if nothing is found.  -F function: Use this
# 'function' to generate the given program's completion.
complete -o bashdefault -F _gnuastro_asttable_completions asttable








# A though on reporing errors, e.g. invalid filenames. The autocompletion
# feature should not suggest anything in case there is an error in the
# commandline. No errors or messages should be shown as the program in
# charge will handle that. This autocompletion feature is only here to help
# with preventing unintentional mistakes. So, in case there is an invalid
# command on the current commandline, there should be no completion
# suggestions.





# Use extended globs in the case statements if needed
# https://mywiki.wooledge.org/BashGuide/Patterns#Extended_Globs
# shopt -s extglob

#  astquery gaia --dataset=edr3 --center=24,25 --radius=0.1 --output=gaia.fits --column=ra,dec,parallax --quiet -i | awk '/[0-9]+/ {print $2}'
