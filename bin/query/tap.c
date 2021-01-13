/*********************************************************************
Table Access Protocol (TAP) based download of given query.
Query is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2021, Free Software Foundation, Inc.

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
#include <stdlib.h>

#include <gnuastro/wcs.h>

#include "main.h"

#include "ui.h"



/* The database string needs to be quoted if it contains a slash. */
static void
tap_database_quote_if_necessary(struct queryparams *p)
{
  int quote=0;
  char *c, *new;

  /* Parse the string. */
  for(c=p->datasetstr; *c!='\0'; ++c)
    if(*c=='/') { quote=1; break; }

  /* Add quotes around the database string. */
  if(quote)
    {
      /* Allocate the new string with quotes. */
      if( asprintf(&new, "\"%s\"", p->datasetstr)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation ('new')",
              __func__);

      /* Free the old one, and replace it. */
      free(p->datasetstr);
      p->datasetstr=new;
    }
}





void
tap_download(struct queryparams *p)
{
  size_t ndim;
  gal_data_t *tmp;
  gal_list_str_t *url;
  double width2, *center, *darray;
  char *tmpstr, *regionstr, *rangestr=NULL;
  char *command, *columns, allcols[]="*", *querystr;
  double *ocenter=NULL, *owidth=NULL, *omin=NULL, *omax=NULL;

  /* If the raw query has been given, use it. */
  if(p->query)
    querystr=p->query;
  else
    {
      /* If certain columns have been requested use them, otherwise
         download all existing columns.*/
      columns = p->columns ? ui_strlist_to_str(p->columns) : allcols;

      /* If the user wanted an overlap with an image, then calculate it. */
      if(p->overlapwith)
        {
          /* Calculate the Sky coverage of the overlap dataset. */
          gal_wcs_coverage(p->overlapwith, p->cp.hdu, &ndim, &ocenter,
                           &owidth, &omin, &omax);

          /* Make sure a WCS existed in the file. */
          if(owidth==NULL)
            error(EXIT_FAILURE, 0, "%s (hdu %s): contains no WCS to "
                  "derive the sky coverage", p->overlapwith, p->cp.hdu);
        }

      /* For easy reading. */
      center = p->overlapwith ? ocenter : p->center->array;

      /* Write the region. */
      if(p->radius)
        {
          darray=p->radius->array;
          if( asprintf(&regionstr, "CIRCLE('ICRS', %.8f, %.8f, %g)",
                       center[0], center[1], darray[0])<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf allocation ('regionstr')",
                  __func__);
        }
      else if(p->width || p->overlapwith)
        {
          darray = p->overlapwith ? owidth : p->width->array;
          width2 = ( (p->overlapwith || p->width->size==2)
                     ? darray[1] : darray[0] );
          if( asprintf( &regionstr, "BOX('ICRS', %.8f, %.8f, %.8f, %.8f)",
                        center[0], center[1], darray[0], width2 )<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf allocation ('regionstr')",
                  __func__);
        }

      /* Set the range criteria on the requested columns. */
      if(p->range)
        for(tmp=p->range; tmp!=NULL; tmp=tmp->next)
          {
            darray=tmp->array;
            if( asprintf(&tmpstr, "%s%sAND %s>=%g AND %s<=%g",
                         rangestr==NULL ? "" : rangestr,
                         rangestr==NULL ? "" : " ",
                         tmp->name, darray[0], tmp->name, darray[1]) < 0 )
              error(EXIT_FAILURE, 0, "%s: asprintf allocation ('tmpstr')",
                    __func__);
            free(rangestr);
            rangestr=tmpstr;
          }

      /* If the dataset has special characters (like a slash) it needs to
         be quoted. */
      tap_database_quote_if_necessary(p);

      /* Write the automatically generated query string. */
      if( asprintf(&querystr,  "SELECT %s "
                   "FROM %s "
                   "WHERE 1=CONTAINS( POINT('\"'ICRS', %s, %s), %s ) %s",
                   columns, p->datasetstr, p->ra_name, p->dec_name, regionstr,
                   rangestr ? rangestr : "")<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation ('querystr')",
              __func__);

      /* Clean up. */
      free(regionstr);
      if(columns!=allcols) free(columns);
      if(p->overlapwith)
        {free(ocenter); free(owidth); free(omin); free(omax);}
    }

  /* Go over the given URLs for this server. */
  for(url=p->urls; url!=NULL; url=url->next)
    {
      /* Build the calling command. Note the quotations around the value of
         QUERY: we start the values with a single quote, because in ADQL,
         we need double quotes to comment dataset names. However, inside
         'queryst', we finish the single quotes and switch to double quotes
         because we need the single quotes around 'IRCS'. So here, while we
         started it with a single quote, we finish with double quotes. */
      if( asprintf(&command, "curl -o%s --form LANG=ADQL "
                   "--form FORMAT=fits --form REQUEST=doQuery "
                   "--form QUERY='%s\" %s", p->downloadname, querystr,
                   url->v)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation ('command')",
              __func__);

      /* Print the calling command for the user to know. */
      if(p->cp.quiet==0)
        error(EXIT_SUCCESS, 0, "running: %s", command);

      /* Run the command: if it succeeds ('system' returns zero), then stop
         parsing with a break. If it fails, then see if this is the last
         URL or not. If its the last one ('url->next' is NULL), then exit
         with a failure, otherwise, just let the user know that this
         download failed, but continue with the next URLs. */
      if(system(command)==EXIT_SUCCESS) break;
      else
        error(url->next ? EXIT_SUCCESS : EXIT_FAILURE, 0,
              "the query download command %sfailed%s\n",
              p->cp.quiet==0 ? "printed above " : "",
              p->cp.quiet==0 ? "" : " (the command can be printed "
              "if you don't use the option '--quiet', or '-q')");
    }

  /* Clean up. */
  free(command);
}
