# Bash autocompletion to Gnuastro's BuildProgram. See the comments above
# 'bin/completion.bash.in' for more.
#
# Original author:
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Contributing author(s):
# Copyright (C) 2021-2022 Free Software Foundation, Inc.
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
# with Gnuastro. If not, see <http://www.gnu.org/





# For debugging
#
# See the description in 'bin/completion.bash.in'.
#source /PATH/TO/GNUASTRO/SRC/bin/completion.bash.in
#source /PATH/TO/GNUASTRO/BUILD/bin/completion.bash.built





#######################################################################
############     Only for BuildProgram (this program)      ############
#######################################################################

# Fill the replies with available C compilers
_gnuastro_autocomplete_compreply_c_compiler(){
    for f in gcc clang cc $CC; do
        if which $f &> /dev/null; then COMPREPLY+=("$f"); fi
    done
}





# Dealing with arguments: BuildProgram currently only takes C source files.
_gnuastro_autocomplete_astbuildprog_arguments(){
    local given_file=""
    if _gnuastro_autocomplete_first_in_arguments source_c; then
        _gnuastro_autocomplete_compreply_options_all ""
    else
        _gnuastro_autocomplete_compreply_files_certain source_c "$argument"
    fi
}





# Fill option value (depends on option).
_gnuastro_autocomplete_astbuildprog_option_value(){

    # Internal variables.
    local junk=1
    local fits_file=""
    local given_hdu=""
    local given_file=""

    # Keep this in the same order as the output of '--help', for options
    # with similar operations, keep the order within the '|'s.
    case "$option_name" in

        -a|--la)
            _gnuastro_autocomplete_compreply_files_certain source_la "$current"
            ;;

        -c|--cc)
            _gnuastro_autocomplete_compreply_c_compiler
            ;;

        -I|--includedir|-L|--linkdir)
            _gnuastro_autocomplete_compreply_directories "$current"
            ;;

        -l|--linklib|-t|--tag|-W|--warning)
            # There is no easy way to guess which libraries the user wants
            # to link with, or the tag, or the warning level.
            junk=1
            ;;

        -O|--optimize)
            for f in $(printf "0\n1\n2\n3" | grep ^"$current"); do
                COMPREPLY+=("$f");
            done
            ;;

    esac
}





_gnuastro_autocomplete_astbuildprog(){

    # The installation directory of Gnuastro. The '@PREFIX@' part will be
    # replaced automatically during 'make install', with the user's given
    # requested installation directory. If you are debugging, please
    # correct it yourself (usually to '/usr/local/bin', but don't commit
    # this particular change).
    local gnuastro_prefix="@PREFIX@"

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
        _gnuastro_autocomplete_astbuildprog_option_value

    # When 'option_name' is not empty (and not yet complete), we are busy
    # filling in the option name.
    elif [ x$option_name != x ]; then
        _gnuastro_autocomplete_compreply_options_all "$option_name"

    # In the case of "none-of-the-above", it is an argument.
    else
        _gnuastro_autocomplete_astbuildprog_arguments
    fi
}





# Define the completion specification, or COMPSPEC: -o bashdefault: Use
# Bash default completions if nothing is found.  -F function: Use this
# 'function' to generate the given program's completion.
complete -o bashdefault -F _gnuastro_autocomplete_astbuildprog astbuildprog
