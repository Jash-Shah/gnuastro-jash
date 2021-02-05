#/usr/bin/env bash

# TODO: GNU Copyright ...
# Original Author:
# Pedram Ashofteh Ardakani <pedramardakani@pm.me>
# Contributing authors:
# Mohammad Akhlaghi <mohammad@akhlaghi.org>

# A thought on short options. I thing they should not be covered by the
# autocompletion. Because only the advanced users may use them. And it is
# possible to mix them up. So, only those will use the short options who
# know what they are doing. Hence, they will not need the autocompletion
# feature binded to the short options.

# TIP: Run the command below to initialize the bash completion feature for
# this specific program (i.e. astcosmiccal):
# $ source astcosmiccal-completion.bash

# GLOBAL VARIABLES

PREFIX="/usr/local/bin";
ASTFITS="$PREFIX/astfits";
ASTTABLE="$PREFIX/asttable";

# Use extended globs in the case statements if needed
# https://mywiki.wooledge.org/BashGuide/Patterns#Extended_Globs
# shopt -s extglob

#  astquery gaia --dataset=edr3 --center=24,25 --radius=0.1 --output=gaia.fits --column=ra,dec,parallax --quiet -i | awk '/[0-9]+/ {print $2}'

_gnuastro_autocomplete_list_fits_hdu(){
    # Accepts a fits filename as input and suggests its headers
    if [[ -f "$1"  ]]; then
        local list="$($ASTFITS --quiet $1 | awk '{print $2}')"
        COMPREPLY=($(compgen -W "$list"))
    fi
}

_gnuastro_autocomplete_list_fits_names(){
    # Suggest all 'FITS' files in current directory. Case insensitive.  The
    # -X option and its filter pattern are explained on bash programmable
    # completion info page:  $ info bash programmable
    COMPREPLY=($(compgen -f -X "!*.[fF][iI][tT][sS]" -- "$word"))
}

_gnuastro_autocomplete_expect_number(){
    # Prompt the user that this option only accepts a number
    echo "Pass"
}

_gnuastro_autocomplete_get_fits_name(){
    # Get the first fits file among the command line and put it into the
    # $comp_fits_name variable
    # TODO: How about all other fits file extensions?
    local file_name="$(echo ${COMP_WORDS[@]} | awk -v regex="[a-zA-Z]*.[fF][iI][tT][sS]" 'match($0, regex) {print substr($0, RSTART, RLENGTH)}')"
    if [ -f "$file_name" ]; then
        # Check if file_name is actually an existing fits file. This
        # prevents other functions from failing and producing obscure error
        # messages
        echo "$file_name"
    fi
}

_gnuastro_autocomplete_get_fits_columns(){
    # Checks if the argument contains a valid file. Does not check for its
    # extension. Then, reads the column names using the asttable program
    # and assigns the result to the $comp_fits_columns array.
    if [ -f "$1" ]; then
        echo "$($ASTTABLE --information $1 | awk -v regex="^[0-9]+" 'match($0, regex) {print $2}')"
    fi
}

_gnuastro_autocomplete_list_fits_columns(){
    # Accept a fits file name as the first argument ($1). Read and suggest
    # its column names
    if [ -f "$1" ]; then
        local list="$(_gnuastro_autocomplete_get_fits_columns $1)"
        COMPREPLY=($(compgen -W "$list"))
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
    # and print all long options available.
    local list="$($1 --help | awk -v regex=" --+[a-zA-Z0-9]*=?" 'match($0, regex) {print substr($0, RSTART, RLENGTH)}')"
    COMPREPLY=($(compgen -W "$list" -- "$word"))
}

_gnuastro_asttable_completions(){

    # TODO: @@
    local PROG_NAME="asttable";

    local PROG_ADDRESS="$PREFIX/$PROG_NAME";

    # Initialize the completion response with null
    COMPREPLY=();

    # Variable "word", is the current word being completed
    local word="${COMP_WORDS[COMP_CWORD]}";

    # Variable "prev" is the word just before the current word
    local prev="${COMP_WORDS[COMP_CWORD-1]}";

    # A quick check to see if there is already a fits file name invoked in
    # the current commandline. This means the order of commands does matter
    # in this bash completion. If we do not want this, we should implement
    # another method for suggesting completions.
    local fits_name="$(_gnuastro_autocomplete_get_fits_name)"

    # TODO: Prettify the code syntax, shorter ones on top
    case "$prev" in
        asttable) _gnuastro_autocomplete_list_fits_names ;;
        -i|--information|-w|--wcsfile)
            if [ -z _gnuastro_autocomplete_get_fits_name ]; then
                # Check if the user has already specified a fits file. If
                # the _gnuastro_autocomplete_get_file_name echoes an empty
                # response, it means no fits files were specified.
                _gnuastro_autocomplete_list_fits_names
            else
                # The user has entered a valid fits file name. So keep on
                # with suggesting all other options at hand.
                _gnuastro_autocomplete_list_options $PROG_NAME
            fi
            ;;
        -c|--column)
            # The function below, checks if the user has specified a fits
            # file in the current commandline. If not, there will be no
            # response from autocompletion. This might alert the user that
            # something is going wrong.
            _gnuastro_autocomplete_list_fits_columns "$fits_name"
            ;;
        -W|--wcshdu)
            # Description is same as the '--column' option.
            _gnuastro_autocomplete_list_fits_hdu "$fits_name"
            ;;
        -b|--noblank) ;;
        -h|--hdu) ;;
        *) _gnuastro_autocomplete_list_options $PROG_NAME ;;
    esac

    # Debugging purpose:
    # printf "\n*** DEBUG ***\n>>> prev: $prev \t \$3: $3\n>>> word: $word \t \$2: $2\n>>> line: ${COMP_LINE[@]}"
}

complete -F _gnuastro_asttable_completions -o nospace asttable
