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

    if [ $prev = "--lineatz" ]; then
        # Show all sub options in "lineatz"
        COMPREPLY=($(compgen -W "$(astcosmiccal --listlines | \
                           awk '!/^#/ {print $2}') " \
                           -- "$word"));
    else
        # Show all options in CosmicCalculator:
        COMPREPLY=($(compgen -W "$(astcosmiccal --help | \
                             awk 'match($0, /--[a-z]+/) {print substr($0, RSTART, RLENGTH)}') " \
                             -- "$word"));
    fi;
}

complete -F _astcosmiccal_completions astcosmiccal
