# Bash autocompletion to Gnuastro's Fits program. See the comments above
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
############          Only for Fits (this program)         ############
#######################################################################

# Dealing with arguments: Fits can take any number of images, so don't
# suggest options
_gnuastro_autocomplete_astfits_arguments(){
    _gnuastro_autocomplete_compreply_files_certain fits "$argument"
}





# Fill option value (depends on option).
_gnuastro_autocomplete_astfits_option_value(){

    # Internal variables.
    local fits_file=""
    local given_hdu=""
    local given_file=""
    local wcs_coordsys=""

    # Keep this in the same order as the output of '--help', for options
    # with similar operations, keep the order within the '|'s.
    case "$option_name" in

        --hdu|--copy|--cut|--remove)
            _gnuastro_autocomplete_given_file fits ""
            _gnuastro_autocomplete_compreply_hdus \
                all "$given_file" "$current"
            ;;

        --outhdu)
            _gnuastro_autocomplete_given_file fits "--output"
            _gnuastro_autocomplete_compreply_hdus \
                all "$given_file" "$current"
            ;;

        --output)
            _gnuastro_autocomplete_compreply_files_certain fits "$current"
            ;;

        --tableformat)
            _gnuastro_autocomplete_compreply_tableformat "$current"
            ;;

        --delete|--datetosec)
            _gnuastro_autocomplete_given_file_and_hdu fits "" "--hdu"
            _gnuastro_autocomplete_compreply_keys "$given_file" \
                                                  "$given_hdu" "$current"
            ;;

        --keyvalue)
            _gnuastro_autocomplete_given_file_and_hdu fits "" "--hdu"
            _gnuastro_autocomplete_compreply_keys "$given_file" \
                                                  "$given_hdu" "$current" \
                                                  "yes"
            ;;

        --rename|--update)
            # Find the match.
            _gnuastro_autocomplete_given_file_and_hdu fits "" "--hdu"
            _gnuastro_autocomplete_compreply_keys "$given_file" \
                                                  "$given_hdu" "$current"

            # If there is the only one suggestion, then add a ',' (to let
            # the user easily type-in the new name.
            if [        x"${COMPREPLY[0]}" != x ] \
                   && [ x"${COMPREPLY[1]}"  = x ]; then
                COMPREPLY[0]="${COMPREPLY[0]},";
                compopt -o nospace
            fi
            ;;

        --wcscoordsys)
            _gnuastro_autocomplete_compreply_wcs_coordsys
            _gnuastro_autocomplete_compreply_from_string \
                "$wcs_coordsys" "$current"
            ;;

        --wcsdistortion)
            _gnuastro_autocomplete_compreply_from_string \
                "SIP TPV" "$current"
            ;;
    esac
}





_gnuastro_autocomplete_astfits(){

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
        _gnuastro_autocomplete_astfits_option_value

    # When 'option_name' is not empty (and not yet complete), we are busy
    # filling in the option name.
    elif [ x$option_name != x ]; then
        _gnuastro_autocomplete_compreply_options_all "$option_name"

    # In the case of "none-of-the-above", it is an argument.
    else
        _gnuastro_autocomplete_astfits_arguments
    fi
}





# Define the completion specification, or COMPSPEC: -o bashdefault: Use
# Bash default completions if nothing is found.  -F function: Use this
# 'function' to generate the given program's completion.
complete -o bashdefault -F _gnuastro_autocomplete_astfits astfits
