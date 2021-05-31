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

#include <gnuastro-internal/checkset.h>

#include "main.h"

#include "ui.h"





/* Basic sanity checks necessary in all TAP-based databases. */
void
tap_sanity_checks(struct queryparams *p)
{
  /* Checks in case a raw query isn't given. */
  if(p->query==NULL)
    {
      /* If '--center' is given, '--radius' is also necessary. */
      if(p->center || p->overlapwith)
        {
          /* Make sure the radius is given, and that it isn't zero. */
          if(p->center && p->radius==NULL && p->width==NULL)
            error(EXIT_FAILURE, 0, "the '--radius' ('-r') or "
                  "'--width' ('-w') options are necessary with "
                  "the '--center' ('-C') option");
        }

      /* If no dataset is explicitly given, let the user know that a
         catalog reference is necessary. */
      if( p->information==0 && p->datasetstr==NULL )
        error(EXIT_FAILURE, 0, "no '--dataset' specified! To get the "
              "list of available datasets (tables) in this database, "
              "please run with '--information' (or '-i'). Note that some "
              "databases (like VizieR) have (tens of) thousands of "
              "datasets. Hence, for a fast result, its best to limit the "
              "downloaded and displayed information list by also adding "
              "--limitinfo=\"SEARCH\" (where 'SEARCH' can be any string "
              "in the description of the dataset, usually project or "
              "author names) for example:\n\n"
              "    astquery %s -i --limitinfo=\"SEARCH_STRING\"\n\n"
              "For more, see the documentation of 'astquery' and the "
              "\"Available databases\" section of the book for more:\n\n"
              "    info astquery\n"
              "    info gnuastro \"Available databases\"\n",
              p->databasestr);
    }
}





/* The database string needs to be quoted if it contains a slash. */
static char *
tap_dataset_quote_if_necessary(struct queryparams *p)
{
  int quote=0;
  char *c, *quoted;

  /* Parse the string for bad characters */
  for(c=p->datasetstr; *c!='\0'; ++c)
    if(*c=='/') { quote=1; break; }

  /* Add quotes around the database string. */
  if(quote)
    {
      /* Allocate the new string with quotes. */
      if( asprintf(&quoted, "\"%s\"", p->datasetstr)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation ('quoted')",
              __func__);
    }
  else quoted=p->datasetstr;

  /* Return the possibly quoted string. */
  return quoted;
}





/* Construct the query for metadata download. */
static char *
tap_query_construct_meta(struct queryparams *p)
{
  char *querystr;

  /* If a dataset is given, build the query to download the metadata of
     that dataset. Otherwise, get the metadata of the full database. */
  if(p->datasetstr)
    {
      if( asprintf(&querystr,  "\"SELECT * FROM TAP_SCHEMA.columns "
                   "WHERE table_name = '%s'\"", p->datasetstr)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation ('querystr')",
              __func__);
    }
  else
    {
      if(p->limitinfo)
        {
          if( asprintf(&querystr,  "\"SELECT * FROM TAP_SCHEMA.tables "
                       "WHERE description LIKE '%%%s%%'\"", p->limitinfo)<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf allocation ('querystr')",
                  __func__);
        }
      else
        gal_checkset_allocate_copy("\"SELECT * FROM TAP_SCHEMA.tables\"",
                                   &querystr);
    }

  /* Clean up and return. */
  return querystr;
}





/* Construct the spatial-constraints criteria if necessary. */
static char *
tap_query_construct_spatial(struct queryparams *p)
{
  size_t ndim;
  double width2, *center, *darray;
  char *regionstr=NULL, *spatialstr=NULL;
  double *ocenter=NULL, *owidth=NULL, *omin=NULL, *omax=NULL;

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

  /* Build the final spatial constraint query string. Note on the
     quotations: the final query is surrounded by single-quotes
     ('). However, we need the single quotes around 'ICRS' in this command
     (both in the final string below and the ones above). So just before
     the first 'ICRS', we end the single-quote and start a double quote and
     keep it until the end. Finally, we add a single quote again so all
     other components of the query can assume that the single-quote
     environment is active.*/
  if( asprintf(&spatialstr,
               "1=CONTAINS( POINT('\"'ICRS', %s, %s), %s )\"'",
               p->ra_name, p->dec_name, regionstr)<0 )
    error(EXIT_FAILURE, 0, "%s: asprintf allocation ('querystr')",
          __func__);


  /* Clean up and return. */
  free(regionstr);
  if(p->overlapwith)
    { free(ocenter); free(owidth); free(omin); free(omax); }
  return spatialstr;
}





static void
tap_query_construct_noblank(struct queryparams *p, char **outstr)
{
  gal_list_str_t *tmp;
  char *noblankstr=NULL, *prevstr=*outstr;

  for(tmp=p->noblank; tmp!=NULL; tmp=tmp->next)
    {
      /* Write 'rangestr'. */
      if(prevstr)
        {
          if( asprintf(&noblankstr, "%s AND %s IS NOT NULL",
                       prevstr, tmp->v) < 0 )
            error(EXIT_FAILURE, 0,
                  "%s: asprintf allocation ('noblankstr', 1)", __func__);
          free(prevstr);
        }
      else
        if( asprintf(&noblankstr, "%s IS NOT NULL", tmp->v) < 0 )
          error(EXIT_FAILURE, 0,
                "%s: asprintf allocation ('noblankstr', 2)", __func__);

      /* Put the 'rangestr' in previous-range string for the next
         round.*/
      prevstr=noblankstr;
    }

  /* Set the final output pointer. */
  *outstr=noblankstr;
}





static void
tap_query_construct_range(struct queryparams *p, char **outstr)
{
  double *darray;
  gal_data_t *tmp;
  char *rangestr=NULL, *prevstr=*outstr;

  for(tmp=p->range; tmp!=NULL; tmp=tmp->next)
    {
      /* Write 'rangestr'. */
      darray=tmp->array;
      if(prevstr)
        {
          if( asprintf(&rangestr, "%s AND %s>=%g AND %s<=%g", prevstr,
                       tmp->name, darray[0], tmp->name, darray[1]) < 0 )
            error(EXIT_FAILURE, 0,
                  "%s: asprintf allocation ('rangestr', 1)", __func__);
          free(prevstr);
        }
      else
        if( asprintf(&rangestr, "%s>=%g AND %s<=%g",
                     tmp->name, darray[0], tmp->name, darray[1]) < 0 )
          error(EXIT_FAILURE, 0,
                "%s: asprintf allocation ('rangestr', 2)", __func__);

      /* Put the 'rangestr' in previous-range string for the next
         round.*/
      prevstr=rangestr;
    }

  /* Set the final output pointer. */
  *outstr=rangestr;
}





static char *
tap_query_construct_sort(struct queryparams *p)
{
  gal_list_str_t *tmp;
  char *sortstr=NULL, *prevstr=NULL;

  for(tmp=p->sort; tmp!=NULL; tmp=tmp->next)
    {
      /* Write 'rangestr'. */
      if(prevstr)
        {
          if( asprintf(&sortstr, "%s,%s", prevstr, tmp->v) < 0 )
            error(EXIT_FAILURE, 0,
                  "%s: asprintf allocation ('sortstr', 1)", __func__);
          free(prevstr);
        }
      else
        if( asprintf(&sortstr, "ORDER BY %s", tmp->v) < 0 )
          error(EXIT_FAILURE, 0,
                "%s: asprintf allocation ('sortstr', 2)", __func__);

      /* Put the 'rangestr' in previous-range string for the next
         round.*/
      prevstr=sortstr;
    }

  /* Set the final output pointer. */
  return sortstr;
}





/* Construct the query for data download. */
static char *
tap_query_construct_data(struct queryparams *p)
{
  char *sortstr=NULL;
  char *headstr=NULL, allcols[]="*";
  char *datasetstr, *valuelimitstr=NULL;
  char *querystr, *columns, *spatialstr=NULL;

  /* If the dataset has special characters (like a slash) it needs to
     be quoted. */
  datasetstr=tap_dataset_quote_if_necessary(p);

  /* If certain columns have been requested use them, otherwise
     download all existing columns.*/
  columns = p->columns ? ui_strlist_to_str(p->columns) : allcols;

  /* If the user only wants the top few rows, only return them. */
  if(p->head!=GAL_BLANK_SIZE_T)
    if( asprintf(&headstr, "TOP %zu", p->head)<0 )
      error(EXIT_FAILURE, 0, "%s: asprintf allocation ('head')",
            __func__);

  /* Build the 'noblank' and 'range' criteria. No blank goes first because
     it is easier to check (for the server), thus the more time-consuming
     range check can be done on fewer rows. */
  if(p->noblank) tap_query_construct_noblank(p, &valuelimitstr);
  if(p->range) tap_query_construct_range(p, &valuelimitstr);

  /* If the user has asked for a spatial constraint. */
  if(p->overlapwith || p->center)
    spatialstr=tap_query_construct_spatial(p);

  /* If the user has asked to sort the columns. */
  if(p->sort) sortstr=tap_query_construct_sort(p);

  /* Write the automatically generated query string.  */
  if( asprintf(&querystr,  "'SELECT %s %s FROM %s %s %s %s %s %s'",
               headstr ? headstr : "",
               columns,
               datasetstr,
               ( valuelimitstr || spatialstr ? "WHERE" : ""),
               valuelimitstr ? valuelimitstr : "",
               ( valuelimitstr && spatialstr ? "AND"   : "" ),
               spatialstr ? spatialstr : "",
               sortstr ? sortstr : "")<0 )
    error(EXIT_FAILURE, 0, "%s: asprintf allocation ('querystr')",
          __func__);

  /* Clean up and return. */
  if(datasetstr!=p->datasetstr) free(datasetstr);
  if(columns!=allcols) free(columns);
  return querystr;
}




void
tap_download(struct queryparams *p)
{
  gal_list_str_t *url;
  char *command, *querystr;

  /* If the raw query has been given, use it. */
  querystr = ( p->query
               ? p->query
               : ( p->information
                   ? tap_query_construct_meta(p)
                   : tap_query_construct_data(p) ) );

  /* Go over the given URLs for this server. */
  for(url=p->urls; url!=NULL; url=url->next)
    {
      /* Build the calling command. Note that the query quotes are
         included by the function building it. */
      if( asprintf(&command, "curl%s -o%s --form LANG=ADQL "
                   "--form FORMAT=fits --form REQUEST=doQuery "
                   "--form QUERY=%s %s", p->cp.quiet ? " -s" : "",
                   p->downloadname, querystr, url->v)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation ('command')",
              __func__);

      /* Print the calling command for the user to know. */
      if(p->dryrun==1 || p->cp.quiet==0)
        {
          if(p->dryrun==0) printf("\n");
          error(EXIT_SUCCESS, 0, "%s: %s",
                p->dryrun ? "would run" : "running", command);
          if(p->dryrun==0) printf("\nDownload status:\n");
        }

      /* Run the command: if it succeeds ('system' returns zero), then stop
         parsing with a break. If it fails, then see if this is the last
         URL or not. If its the last one ('url->next' is NULL), then exit
         with a failure, otherwise, just let the user know that this
         download failed, but continue with the next URLs. */
      if(p->dryrun) break;
      else
        {
          if(system(command)==EXIT_SUCCESS) break;
          else
            error(url->next ? EXIT_SUCCESS : EXIT_FAILURE, 0,
                  "the query download command %sfailed%s\n",
                  p->cp.quiet==0 ? "printed above " : "",
                  p->cp.quiet==0 ? "" : " (the command can be printed "
                  "if you don't use the option '--quiet', or '-q')");
        }
    }

  /* Keep the executed command (to put in the final file's meta-data). */
  p->finalcommand=command;
}
