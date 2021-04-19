/*********************************************************************
Functions to interface with DS9 files.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2015-2021, Free Software Foundation, Inc.

Gnuastro is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Gnuastro is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with Gnuastro. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

#include <gnuastro/ds9.h>
#include <gnuastro/data.h>

#include <gnuastro-internal/options.h>





/* Read the polygon specified in the given DS9 region file and parse it in
   the standard format. */
gal_data_t *
gal_ds9_reg_read_polygon(char *filename, int *coordmode)
{
  FILE *fp;
  char *polygonstr;
  gal_data_t *out=NULL;
  size_t commacounter=0;
  size_t plinelen, linesize=10, lineno=0;
  char *c, *line, *ds9regstart="# Region file format: DS9";
  char *polygonformaterr="It is expected for the line to have "
    "this format: 'polygon(AAA,BBB,...)'. Where 'AAA' and 'BBB' "
    "are numbers and '...' signifies that any number of points "
    "are possible";

  /* Initialize 'coordmode'. */
  *coordmode=GAL_DS9_COORD_MODE_INVALID;

  /* Allocate size to the lines on the file and check if it was sucessfull.
     The getline function reallocs the necessary memory. */
  errno=0;
  line = malloc(linesize * sizeof(*line));
  if(line == NULL)
    error(EXIT_FAILURE, errno, "%s: %zu bytes for line buffer",
          __func__, linesize * sizeof(*line));

  /* Open the file and checks if it's not null. */
  errno=0;
  fp = fopen(filename, "r");
  if(fp == NULL)
    error(EXIT_FAILURE, errno, "The polygon file is blank");

  /* Get the lines on the file. */
  while(getline(&line, &linesize, fp)!=-1)
    {
      /* To have line-counters starting from 1. */
      ++lineno;

      /* The first line should start with a fixed string. */
      if(lineno==1)
        {
          if( strncmp(line, ds9regstart, 25) )
            error(EXIT_FAILURE, 0, "%s: doesn't appear to be a DS9 "
                  "region file! We assume that DS9 region files begin "
                  "with this string in their first line: '%s'",
                  filename, ds9regstart);
          continue;
        }

      /* If we are on the coordinate mode line, then set the mode of Crop
         based on it. */
      if( !strcmp(line, "fk5\n") || !strcmp(line, "image\n") )
        {
          /* Make sure it hasn't been called more than once. */
          if(*coordmode!=GAL_DS9_COORD_MODE_INVALID)
            error_at_line(EXIT_FAILURE, 0, filename, lineno,
                          "more than one coordinate line defined");

          /* Set the proper mode. */
          if(!strcmp(line, "fk5\n")) *coordmode=GAL_DS9_COORD_MODE_WCS;
          else                       *coordmode=GAL_DS9_COORD_MODE_IMG;

          /* Stop parsing the file if the polygon has also been found by
             this point (we don't need any more information, no need to
             waste the user's CPU and time). */
          if(out) break;
        }

      /* The line containing the polygon information starts with
         'polygon('. */
      if( !strncmp(line, "polygon(", 8) )
        {
          /* Get the line length and check if it is indeed in the proper
             format. If so, remove the last parenthesis. */
          plinelen=strlen(line);
          if(line[plinelen-2]==')')
            line[plinelen-2]='\0';
          else
            error_at_line(EXIT_FAILURE, 0, filename, lineno,
                          "line with polygon vertices doesn't end "
                          "with ')'. %s", polygonformaterr);

          /* Convert the string to the expected format (with ':' separating
             each vertice). Note how we are ignoring the first 8 characters
             that contain 'polygon('. */
          polygonstr=&line[8];
          for(c=polygonstr; *c!='\0'; ++c)
            if(*c==',' && (++commacounter % 2) == 0 ) *c=':';

          /* Read the coordinates within the line. */
          out=gal_options_parse_colon_sep_csv_raw(polygonstr,
                                                  filename,
                                                  lineno);

          /* Stop parsing the file if the coordinate mode has also been
             found by this point (we don't need any more information, no
             need to waste the user's CPU and time). */
          if(*coordmode!=GAL_DS9_COORD_MODE_INVALID) break;
        }
    }

  /* If no coordinate mode was found in the file, print an error. */
  if(*coordmode==GAL_DS9_COORD_MODE_INVALID)
    error(EXIT_FAILURE, 0, "%s: no coordinate mode found! "
          "We expect one line to be either 'fk5' or 'image'",
          filename);

  /* If no polygon line was in the file, abort with an error. */
  if(out==NULL)
    error(EXIT_FAILURE, 0, "%s: no polygon statement found! We expect "
          "one line in the format of 'polygon(AAA,BBB,...)' in the "
          "file given to '--polygonfile' option. %s", filename,
          polygonformaterr);

  /* Clean up and return. */
  free(line);
  fclose(fp);
  return out;
}
