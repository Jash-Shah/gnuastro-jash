#/usr/bin/env bash

# TIP: Run the command below to initialize the bash completion feature for
# this specific program (i.e. astcosmiccal):
# $ source astcosmiccal-completion.bash

# GLOBAL VARIABLES

PREFIX="/usr/local/bin";
ASTFITS="$PREFIX/astfits";
ASTTABLE="$PREFIX/asttable";

#  astquery gaia --dataset=edr3 --center=24,25 --radius=0.1 --output=gaia.fits --column=ra,dec,parallax --quiet -i | awk '/[0-9]+/ {print $2}'

_gnuastro_autocomplete_fits_hdu_read(){
    # Accepts a fits filename as input and suggests its headers
    COMPREPLY=($("$ASTFITS --quiet $1" | awk '{print $2}'))
}

_gnuastro_autocomplete_fits_list_files(){
    # Suggest all 'FITS' files in current directory. Case insensitive.
    COMPREPLY+=($(compgen -f -X "!*.[fF][iI][tT][sS]"));
}

_gnuastro_autocomplete_column_read(){
    echo "Pass"
}

_gnuastro_autocomplete_expect_number(){
    # Prompt the user that this option only accepts a number
    echo "Pass"
}

_gnuastro_fits_last_occurance(){
    # The last FITS file in COMP_LINE
    echo "Pass"
}

_gnuastro_file_last_occurance(){
    # The last file name (.txt/.fits) in COMP_LINE
    echo "Pass"
}

# just find the short commands
# astconvolve --help | awk -v pattern="^ *-([a-z]|[A-Z])" 'match($0, pattern) {print $0}'

_gnuastro_autocomplete_list_all_options(){
    COMPREPLY=($($1 --help | awk -v regex=" --+([a-z]|[A-Z]|[0-9])*" 'match($0, regex) {print substr($0, RSTART, RLENGTH)}'))
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

    case "$word" in
        -i|--information)
            _gnuastro_autocomplete_fits_list_files
        ;;
        -b|--noblank) ;;
        -h|--hdu) ;;
        # default case
        *) _gnuastro_autocomplete_list_all_options $PROG_NAME ;;
    esac

}

complete -F _gnuastro_asttable_completions asttable
