# Bash autocompletion to Gnuastro's ConvertType. See the comments above
# 'bin/completion.bash.in' for more.
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
############      Only for ConvertType (this program)      ############
#######################################################################

# Dealing with arguments: ConvertType can take multiple images as arguments
# (so unlike other programs, there is no option to print options). One way
# would be to calculate how many color channels there are in each input and
# look at the desired output format and its necessary color channels. But
# that is too complicated for now.
_gnuastro_autocomplete_astconvertt_arguments(){
    _gnuastro_autocomplete_compreply_images_all "$argument"
}





# Fill option value (depends on option).
_gnuastro_autocomplete_astconvertt_option_value(){

    # Internal variables.
    local fits_file=""
    local given_hdu=""
    local given_file=""
    local convertt_colormaps=""

    # Keep this in the same order as the output of '--help', for options
    # with similar operations, keep the order within the '|'s.
    case "$option_name" in

        -h|--hdu|-g|--globalhdu)
            _gnuastro_autocomplete_given_file image ""
            _gnuastro_autocomplete_compreply_hdus \
                image "$given_file" "$current"
            ;;

        --colormap)
            _gnuastro_autocomplete_compreply_convertt_colormap
            _gnuastro_autocomplete_compreply_from_string \
                "$convertt_colormaps" "$current"
            ;;

        -u|--quality)
            _gnuastro_autocomplete_compreply_from_string \
                "$(seq 100)" "$current"
            ;;

        --wcslinearmatrix)
            _gnuastro_autocomplete_compreply_wcslinearmatrix "$current"
            ;;

        --numthreads)
            _gnuastro_autocomplete_compreply_numthreads "$current"
            ;;


    esac
}





_gnuastro_autocomplete_astconvertt(){

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
        _gnuastro_autocomplete_astconvertt_option_value

    # When 'option_name' is not empty (and not yet complete), we are busy
    # filling in the option name.
    elif [ x$option_name != x ]; then
        _gnuastro_autocomplete_compreply_options_all "$option_name"

    # In the case of "none-of-the-above", it is an argument.
    else
        _gnuastro_autocomplete_astconvertt_arguments
    fi
}





# Define the completion specification, or COMPSPEC: -o bashdefault: Use
# Bash default completions if nothing is found.  -F function: Use this
# 'function' to generate the given program's completion.
complete -o bashdefault -F _gnuastro_autocomplete_astconvertt astconvertt
