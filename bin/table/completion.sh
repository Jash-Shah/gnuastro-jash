#/usr/bin/env bash

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

_gnuastro_autocomplete_fits_hdu_read(){
    # Accepts a fits filename as input and suggests its headers
    COMPREPLY=($("$ASTFITS --quiet $1" | awk '{print $2}'))
}

_gnuastro_autocomplete_list_fits_files(){
    # Suggest all 'FITS' files in current directory. Case insensitive.
    COMPREPLY=($(compgen -f -X "!*.[fF][iI][tT][sS]" -- "$word"))
}

_gnuastro_autocomplete_expect_number(){
    # Prompt the user that this option only accepts a number
    echo "Pass"
}

_gnuastro_autocomplete_get_fits_file(){
    # Get the first fits file among the command line
    # TODO: Add all other fits file extensions
    comp_fits_file="$(echo ${COMP_WORDS[@]} | \
                           awk -v regex="([a-z]|[A-Z])*.[fF][iI][tT][sS]" \
                           'match($0, regex) \
                           {print substr($0, RSTART, RLENGTH)}')"
}

_gnuastro_autocomplete_list_fits_columns(){
    # Get the fits file name in current command line, put into
    # the $comp_fits_file variable
    _gnuastro_autocomplete_get_fits_file
    # If the fits file does exist, fetch its column names
    if [[ -f "$comp_fits_file" ]]; then
        # Set a global array named comp_fits_columns that contains all columns
        # inside the fits file specified in the first argument: $1.
        comp_fits_columns=("$($ASTTABLE --information $comp_fits_file | \
                                        awk -v regex="^[0-9]+" \
                                        'match($0, regex) \
                                        {print $2}')")
        COMPREPLY=($(compgen -W "${comp_fits_columns[@]}" -- "$word"))
    fi
}

_gnuastro_autocomplete_get_file(){
    # The last file name (.txt/.fits) in COMP_LINE
    echo "Pass"
}

# just find the short commands
# astconvolve --help | awk -v pattern="^ *-([a-z]|[A-Z])" 'match($0, pattern) {print $0}'

_gnuastro_autocomplete_list_all_options(){
    # The regex variable in the awk program contains the regular expression
    # pattern that matches all options provided in corresponding program
    COMPREPLY=($(compgen -W "$($1 --help | \
                         awk -v regex=" --+([a-z]|[A-Z]|[0-9])*" \
                         'match($0, regex) \
                         {print substr($0, RSTART, RLENGTH)}')" \
                         -- "$word"))
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

    # TODO: Prettify the code syntax, shorter ones on top
    case "$prev" in
        -i|--information) _gnuastro_autocomplete_list_fits_files ;;
        -c|--column) _gnuastro_autocomplete_list_fits_columns ;;
        -b|--noblank) ;;
        -h|--hdu) ;;
        *) _gnuastro_autocomplete_list_all_options $PROG_NAME ;;
    esac
}

complete -F _gnuastro_asttable_completions asttable
