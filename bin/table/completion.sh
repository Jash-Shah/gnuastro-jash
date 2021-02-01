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
    $("$ASTFITS --quiet $1" | awk '{print $2}')
}

_gnuastro_autocomplete_fits_list(){

}

_gnuastro_autocomplete_column_read(){

}

_gnuastro_autocomplete_expect_number(){
    # Prompt the user that this option only accepts a number
}

_gnuastro_fits_last_occurance(){
    # The last FITS file in COMP_LINE

}

_gnuastro_file_last_occurance(){
    # The last file name (.txt/.fits) in COMP_LINE
}

_gnuastro_asttable_completions(){



    # TODO: @@
    PROG_NAME="asttable";

    PROG_ADDRESS="$PREFIX/$PROG_NAME";

    # Initialize the completion response with null
    COMPREPLY=();

    # Variable "word", is the current word being completed
    local word="${COMP_WORDS[COMP_CWORD]}";

    # Variable "prev" is the word just before the current word
    local prev="${COMP_WORDS[COMP_CWORD-1]}";

    # Create the array of options that need a fits file as input
    local infits=($($PROG_ADDRESS --help | \
                                   awk 'match($0, /--([A-Z]|[a-z]|[0-9])*=FITS/) {print substr($0, RSTART, RLENGTH-4)}'));

#    case "$1" in
#        -h|--hdu)
#        # default case
#        *) ;;
#    esac

    # Add completion suggestions for special options
    if [ $prev = "--lineatz" ]; then
        # Show all sub options in "lineatz"
        COMPREPLY+=($(compgen -W "$($PREFIX/$PROG_NAME --listlines | \
                             awk '!/^#/ {print $2}') " \
                              -- "$word"));
    fi

    # Add completion suggestions for general options
    if [[ "${infits[@]}" =~ "$prev" ]]; then

        # Check if the previous word exists in the "infits" array
        # Look into 'gal_fits_name_is_fits' function for acceptable suffixes
        COMPREPLY+=($(compgen -f -X "!*.[fF][iI][tT][sS]"));
    else
        # Show all options in CosmicCalculator:
        COMPREPLY+=($(compgen -W "$(astcosmiccal --help | \
                             awk 'match($0, /--([A-Z]|[a-z]|[0-9])*/) {print substr($0, RSTART, RLENGTH)}') " \
                             -- "$word"));
    fi;
}

complete -F _asttable_completions asttable
