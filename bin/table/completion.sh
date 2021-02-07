#!/usr/bin/env bash

# TODO: GNU Copyright ...
# Original Author:
# Pedram Ashofteh Ardakani <pedramardakani@pm.me>
# Contributing authors:
# Mohammad Akhlaghi <mohammad@akhlaghi.org>

# A thought on short options. I thing they should not be covered by the
# autocompletion. Because only the advanced users may use them. And it is
# possible to mix them up. So, only those will use the short options who
# know what they are doing. Hence, they will not need the autocompletion
# feature binded to the short options.  However, the short options are
# taken into consideration for suggesting the upcoming commands.

# A though on reporing errors, e.g. invalid filenames. The autocompletion
# feature should not suggest anything in case there is an error in the
# commandline. No errors or messages should be shown as the program in
# charge will handle that. This autocompletion feature is only here to help
# with preventing unintentional mistakes. So, in case there is an invalid
# command on the current commandline, there should be no completion
# suggestions.

# A thought on portability and obeying POSIX standards. This autocomplete
# script should be compatible to bash 3.0 (2004) and newer. That is because
# the MacOS standard is still around bash 3.2. Some commands such as
# '[[ =~', 'complete', arrays, etc. are not POSIX compatible. But they are bash
# built-in.

# TODO: There should be some way to keep autocomplete from crashing in
# older systems.

# TIP: Run the command below to initialize the bash completion feature for
# this specific program (i.e. astcosmiccal):
# $ source astcosmiccal-completion.bash

# GLOBAL VARIABLES

PREFIX="/usr/local/bin";
ASTFITS="$PREFIX/astfits";
ASTTABLE="$PREFIX/asttable";
db=0 # Set 0 for printing debug messages, else set to 1

# Use extended globs in the case statements if needed
# https://mywiki.wooledge.org/BashGuide/Patterns#Extended_Globs
# shopt -s extglob

#  astquery gaia --dataset=edr3 --center=24,25 --radius=0.1 --output=gaia.fits --column=ra,dec,parallax --quiet -i | awk '/[0-9]+/ {print $2}'

_gnuastro_autocomplete_get_fits_hdu(){
    # Accepts a fits filename as input and echoes its headers
    if [ -f "$1" ]; then
        $ASTFITS --quiet "$1" | awk '{print $2}'
    fi
}

_gnuastro_autocomplete_compgen(){
    # Accept either an array or a string '$1' split by normal bash
    # conventions, check if second argument '$2' is present in the
    # suggestions, if so, put it in suggestions, continue otherwise. Note,
    # in case the second agument is not passed, no filteration will be
    # applied.
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

_gnuastro_autocomplete_list_fits_hdu(){
    # Checks for the current fits file and puts its headers into
    # completion suggestions
    if [ -f "$1"  ]; then
        list=("$(_gnuastro_autocomplete_get_fits_hdu "$1")")
        # A custom enhancement for the 'compgen' command. This version will
        # have no problem with the dash sign '-'. Because sometimes the
        # 'hdu' names might contain dash symbols in them. This ensures that
        # all of them are suggested.
        _gnuastro_autocomplete_compgen "${list[@]}" "$word"
        unset list
    fi
}

_gnuastro_autocomplete_list_fits_names(){
    # 'Append' all 'FITS' files in current directory to suggestions. Case
    # insensitive.  The -X option and its filter pattern are explained on
    # bash programmable completion info page: $ info bash programmable
    # 'Appending' seems a good idea because the program might accept
    # multiple input types.

    # For example the 'asttable' program can either accept a fits file or
    # various short/long options as its first argument. In this case,
    # autocompletion suggests both.

    # The completion can not suggest filenames that contain white space in
    # them for the time being.
    COMPREPLY+=($(compgen -f -X "!*.[fF][iI][tT][sS]" -- "$word"))
}

_gnuastro_autocomplete_expect_number(){
    # Prompt the user that this option only accepts a number
    echo "Pass"
}

_gnuastro_autocomplete_get_fits_name(){
    # Iterate and echo the last fits file among the command line
    # TODO: How about all other fits file extensions?
    for w in "${COMP_WORDS[@]}"
    do
        temp_name="$(echo "$w" | awk '/[.][fF][iI][tT][sS]$/')"
        [ -f "$temp_name" ] && file_name="$temp_name"
    done
    unset temp_name

    if [ -f "$file_name" ]; then
        # Check if file_name is actually an existing fits file. This
        # prevents other functions from failing and producing obscure error
        # messages
        echo "$file_name"
        # Note that should not be an 'else' statement with 'exit' error
        # code. Because this function is checking the presence of a fits
        # file everytime bash completion is provoked. Then it will return
        # error if there is no fits name and break functionality.
    fi
    unset file_name
}

_gnuastro_autocomplete_get_fits_columns(){
    # Checks if the argument contains a valid file. Does not check for its
    # extension. Then, reads the column names using the asttable program
    # and echoes the resulting STR.
    if [ -f "$1" ]; then
        # Force 'awk' to read after the second line of 'asttable' output,
        # because the second line contains the filename. The filename might
        # start with numbers. If so, there will be an unwanted '(hdu:'
        # printed in the results. Here, 'awk' will print the second column
        # in lines that start with a number.
        "$ASTTABLE" --information "$1" | awk 'NR>2' | awk '/^[0-9]/ {print $2}'
    fi
}

_gnuastro_autocomplete_list_fits_columns(){
    # Accept a fits file name as the first argument ($1). Read and suggest
    # its column names. If the file does not exist, pass.
    if [ -f "$1" ]; then
        list=("$(_gnuastro_autocomplete_get_fits_columns "$1")")
        _gnuastro_autocomplete_compgen "${list[@]}"
        unset list
    fi
}

_gnuastro_autocomplete_get_file(){
    # The last file name (.txt/.fits) in COMP_LINE
    echo "Pass"
}

# just find the short commands
# astconvolve --help | awk -v pattern="^ *-([a-z]|[A-Z])" 'match($0, pattern) {print $0}'

_gnuastro_autocomplete_list_options(){
    # Accept the command name and its absolute path, run the --help option
    # and 'append' all long options to the current suggestions. 'Appending'
    # seems a good idea because the program might accept multiple input
    # types. For example the 'asttable' program can either accept a fits
    # file or various short/long options as its first argument. In this
    # case, autocompletion suggests both.
    list=("$("$1" --help | awk -v regex=" --+[a-zA-Z0-9]*=?" 'match($0, regex) {print substr($0, RSTART, RLENGTH)}')")
    _gnuastro_autocomplete_compgen "${list[@]}" "$word"
    unset list
}

_gnuastro_asttable_completions(){

    # TODO: @@
    PROG_NAME="asttable";

    PROG_ADDRESS="$PREFIX/$PROG_NAME";

    # Initialize the completion response with null
    COMPREPLY=();

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

    # A quick check to see if there is already a fits file name invoked in
    # the current commandline. This means the order of commands does matter
    # in this bash completion. If we do not want this, we should implement
    # another method for suggesting completions.
    fits_name="$(_gnuastro_autocomplete_get_fits_name)"

    # TODO: Prettify the code syntax, shorter ones on top
    case "$prev" in
        asttable)
            _gnuastro_autocomplete_list_fits_names
            _gnuastro_autocomplete_list_options $PROG_ADDRESS
            ;;
        -i|--information)
            if [ -f "$fits_name" ]; then
                # The user has entered a valid fits file name. So keep on
                # with suggesting all other options at hand.
                _gnuastro_autocomplete_list_options $PROG_ADDRESS
            else
                # Check if the user has already specified a fits file. If
                # the _gnuastro_autocomplete_get_file_name echoes an empty
                # response, it means no fits files were specified.
                _gnuastro_autocomplete_list_fits_names
            fi
            ;;
        -L|--catcolumnfile|-w|--wcsfile)
            # Only suggest a fits filename
            _gnuastro_autocomplete_list_fits_names
            ;;
        -c|--column|-r|--range|-s|--sort|-C|--catcolumns)
            # The function below returns the columns inside the last fits
            # file specified in the commandline. If no fits files were
            # detected, there will be no response from autocompletion. This
            # might alert the user that something is going wrong.
            _gnuastro_autocomplete_list_fits_columns "$fits_name"
            ;;
        -W|--wcshdu)
            # Description is same as the '--column' option.
            _gnuastro_autocomplete_list_fits_hdu "$fits_name"
            ;;
        -o|--output)
            # Do not suggest anything.
            ;;
        -b|--noblank) ;;
        -h|--hdu) ;;
        *) _gnuastro_autocomplete_list_options $PROG_ADDRESS ;;
    esac

    if [[ ! "${COMPREPLY[@]}" =~ "=" ]]; then
        # '[[' and 'compopt' work for bash 3+ so it should be portable to
        # systems younger than 2004.
        # https://mywiki.wooledge.org/BashFAQ/061
        compopt +o nospace
    fi

    # Debugging purpose:
    if [ $db -eq 0 ]; then
        cat <<EOF

************ DEBUG ************
>>> prev: '$prev' -- \$3: '$3'
>>> word: '$word' -- \$2: '$2'
>>> fits_name: '$fits_name'
>>> COMP_WORDS: '${COMP_WORDS[@]}'
>>> COMPREPLY: '${COMPREPLY[@]}'
*******************************
EOF
        # Printf should stay outside the 'heredoc' to prevent an extra
        # newline. As a result, the cursor stays on the same line.
        printf ">>> Line: %s" "$COMP_LINE"
    fi

}

complete -F _gnuastro_asttable_completions -o nospace asttable
