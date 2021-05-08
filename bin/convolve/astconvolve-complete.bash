# Bash autocompletion to Gnuastro's Convolve program. See the comments
# above 'bin/completion.bash.in' for more.
#
# Original author:
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Contributing author(s):
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
# with Gnuastro. If not, see <http://www.gnu.org/





# For debugging
#
# See the description in 'bin/completion.bash.in'.
#source /PATH/TO/GNUASTRO/SRC/bin/completion.bash.in
#source /PATH/TO/GNUASTRO/BUILD/bin/completion.bash.built





#######################################################################
############        Only for Convolve (this program)       ############
#######################################################################

# Dealing with arguments: Convolve only takes one argument/file (either an
# image or a table). So if an image/table file has already been given on
# the command-line only suggest option names.
_gnuastro_autocomplete_astconvolve_arguments(){
    local given_file=""
    if _gnuastro_autocomplete_first_in_arguments image \
       || _gnuastro_autocomplete_first_in_arguments table; then
        _gnuastro_autocomplete_compreply_options_all ""
    else
        _gnuastro_autocomplete_compreply_files_certain image "$argument"
        _gnuastro_autocomplete_compreply_files_certain table "$argument"
    fi
}





# Fill option value (depends on option).
_gnuastro_autocomplete_astconvolve_option_value(){

    # Internal variables.
    local fits_file=""
    local given_hdu=""
    local given_file=""
    local convertt_colormaps=""

    # Keep this in the same order as the output of '--help', for options
    # with similar operations, keep the order within the '|'s.
    case "$option_name" in

        -c|--column)
            _gnuastro_autocomplete_given_file_and_hdu table "" --hdu
            _gnuastro_autocomplete_compreply_table_columns \
                "$given_file" "$given_hdu" "$current"
            ;;

        # Since Convolve takes both tables and images, if we can't find an
        # already given image file, we'll search for a table.
        --hdu|--khdu)
            # Set the option name to check for the file.
            local fileoption=""
            if [ "$option_name" = "--khdu" ]; then fileoption="--kernel"; fi

            # Find the matching HDUs.
            _gnuastro_autocomplete_given_file image "$fileoption"
            if [ x"$given_file" = x ]; then
                _gnuastro_autocomplete_given_file table "$fileoption"
                _gnuastro_autocomplete_compreply_hdus \
                    table "$given_file" "$current"
            else
                _gnuastro_autocomplete_compreply_hdus \
                    image "$given_file" "$current"
            fi
            ;;

        --kernelcolumn)
            _gnuastro_autocomplete_given_file_and_hdu table --kernel --khdu
            _gnuastro_autocomplete_compreply_table_columns \
                "$given_file" "$given_hdu" "$current"
            ;;

        -k|--kernel)
            _gnuastro_autocomplete_compreply_files_certain image "$current"
            _gnuastro_autocomplete_compreply_files_certain table "$current"
            ;;

        --searchin)
            _gnuastro_autocomplete_compreply_searchin "$current"
            ;;

        --interpmetric)
            _gnuastro_autocomplete_compreply_interpmetric "$current"
            ;;

        --type)
            _gnuastro_autocomplete_compreply_numbertype "$current"
            ;;

        --wcslinearmatrix)
            _gnuastro_autocomplete_compreply_from_string "cd pc" "$current"
            ;;

        -d|--domain)
            _gnuastro_autocomplete_compreply_from_string \
                "spatial frequency" "$current"
            ;;

        --numthreads)
            _gnuastro_autocomplete_compreply_numthreads
            ;;


    esac
}





_gnuastro_autocomplete_astconvolve(){

    # The installation directory of Gnuastro. The '@PREFIX@' part will be
    # replaced automatically during 'make install', with the user's given
    # requested installation directory. Ff you are debugging, please
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
        _gnuastro_autocomplete_astconvolve_option_value

    # When 'option_name' is not empty (and not yet complete), we are busy
    # filling in the option name.
    elif [ x$option_name != x ]; then
        _gnuastro_autocomplete_compreply_options_all "$option_name"

    # In the case of "none-of-the-above", it is an argument.
    else
        _gnuastro_autocomplete_astconvolve_arguments
    fi
}





# Define the completion specification, or COMPSPEC: -o bashdefault: Use
# Bash default completions if nothing is found.  -F function: Use this
# 'function' to generate the given program's completion.
complete -o bashdefault -F _gnuastro_autocomplete_astconvolve astconvolve
