#/usr/bin/env bash

# TIP: Run the command below to initialize the bash completion feature for
# this specific program (i.e. astcosmiccal):
# $ source astcosmiccal-completion.bash

_astcosmiccal_completions(){

    # PATH_TO_COMMAND_BIN, i.e. the directory which the astcosmiccal
    # command sits in.
    local PATH_TO_COMMAND_BIN=".";

    # Use "args.h" file to filter out all valid options of the command and
    # put them all in the variable "commandoptions".

    local commandoptions=$(sed -n -E '/^\s+\"[a-z]+/p' \
                               "$PATH_TO_COMMAND_BIN/args.h" | \
                               # remove newline and space characters
                               tr -d '\n" ' | \
                               # replace commas with spaces
                               tr ',' ' ');

    # Initialize the completion response with null
    COMPREPLY=();

    # Variable "word", is the current word being completed
    local word="${COMP_WORDS[COMP_CWORD]}";

    # Variable "prev" is the word just before the current word
    local prev="${COMP_WORDS[COMP_CWORD-1]}";

    if [ $prev = "lineatz" ]; then
        # Show options related to "lineatz"
        COMPREPLY=($(compgen -W "$(astcosmiccal --listlines | \
                           awk '!/^#/ {print $2}') " \
                           -- "$word"));
    else
        COMPREPLY=($(compgen -W "$commandoptions" -- "$word"));
    fi;
}

complete -F _astcosmiccal_completions astcosmiccal
