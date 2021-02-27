#/usr/bin/env bash

# TIP: Run the command below to initialize the bash completion feature for
# this specific program (i.e. astcosmiccal):
# $ source astcosmiccal-completion.bash

_astcosmiccal_completions(){

    # Initialize the completion response with null
    COMPREPLY=();

    # Variable "word", is the current word being completed
    local word="${COMP_WORDS[COMP_CWORD]}";

    # Variable "prev" is the word just before the current word
    local prev="${COMP_WORDS[COMP_CWORD-1]}";

    # Create the array of options that need a fits file as input
    local infits=($(astcosmiccal --help | \
                                   awk 'match($0, /--([A-Z]|[a-z]|[0-9])*=FITS/) {print substr($0, RSTART, RLENGTH-4)}'));

    # Add completion suggestions for special options
    if [ $prev = "--lineatz" ]; then
        # Show all sub options in "lineatz"
        COMPREPLY+=($(compgen -W "$(astcosmiccal --listlines | \
                             awk '!/^#/ {print $2}') " \
                              -- "$word"));
    fi

    # Add completion suggestions for general options
    if [[ "${infits[@]}" =~ "$prev" ]]; then
        # Check if the previous word exists in the "infits" array
        COMPREPLY+=($(compgen -f -X "!*.[fF][iI][tT][sS]"));
    else
        # Show all options in CosmicCalculator:
        COMPREPLY+=($(compgen -W "$(astcosmiccal --help | \
                             awk 'match($0, /--([A-Z]|[a-z]|[0-9])*/) {print substr($0, RSTART, RLENGTH)}') " \
                             -- "$word"));
    fi;
}

complete -F _astcosmiccal_completions astcosmiccal
