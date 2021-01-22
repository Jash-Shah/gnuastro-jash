/*********************************************************************
Query - Retreive data from a remote data server.
Query is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2020-2021, Free Software Foundation, Inc.

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
#include <string.h>

#include <gnuastro/wcs.h>
#include <gnuastro/pointer.h>

#include <gnuastro-internal/checkset.h>

#include "tap.h"
#include "ned.h"
#include "gaia.h"
#include "query.h"
#include "astron.h"
#include "vizier.h"





/* Go over the columns and see if a given column exists. */
static int
query_output_meta_col(gal_list_str_t **cols, gal_data_t *allcols,
                      size_t numcols, char *string)
{
  size_t i;
  for(i=0; i<numcols; ++i)
    if( !strcmp(allcols[i].name, string) )
      {
        gal_list_str_add(cols, string, 0);
        return 1;
      }
  return 0;
}





/* Read the downloaded metadata for all the tables (datasets) within a
   database and print them in an easy to read format. */
static void
query_output_meta_database(struct queryparams *p)
{
  int tableformat;
  size_t i, *size=NULL;
  size_t numcols, numrows;
  gal_list_str_t *cols=NULL;
  char **name, **type, **desc;
  gal_data_t *table, *allcols, *scol=NULL;

  /* Get the downloaded metadata column information so we can ask for the
     proper columns. */
  allcols=gal_table_info(p->downloadname, "1", NULL, &numcols,
                         &numrows, &tableformat);

  /* Parse the column information to set the necessary columns. */
  if( query_output_meta_col(&cols, allcols, numcols, "table_name") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'table_name' found, but this "
            "is required by the IVOA TAP standard"); return; }
  if( query_output_meta_col(&cols, allcols, numcols, "description") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'description' found, but this "
            "is required by the IVOA TAP standard"); return; }
  if( query_output_meta_col(&cols, allcols, numcols, "table_type") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'table_type' found, but this "
            "is required by the IVOA TAP standard"); return; }
  query_output_meta_col(&cols, allcols, numcols, "size");

  /* Read the necessary columns in the desired order, we just need to
     reverse the list first since it is last-in-first-out. */
  gal_list_str_reverse(&cols);
  table=gal_table_read(p->downloadname, "1", NULL, cols,
                       GAL_TABLE_SEARCH_NAME, 1, p->cp.minmapsize,
                       p->cp.quietmmap, NULL);

  /* Set the basic columns for easy reading. */
  name=table->array;
  desc=table->next->array;
  type=table->next->next->array;
  if(table->next->next->next)
    {
      scol=gal_data_copy_to_new_type_free(table->next->next->next,
                                          GAL_TYPE_SIZE_T);
      table->next->next->next=scol;
      size=scol->array;
    }

  /* Print all the information for those tables that have a type of
     'table'. If I understood the TAP standard properly, the 'view' ones
     aren't relevant for non-webpage users. */
  printf("\nRETRIEVED DATASET INFORMATION\n"
         "=============================\n");
  printf("Database: %s (URL: %s)\n", p->databasestr, p->urls->v);
  if(p->limitinfo)
    printf("Only datasets containing string below in description "
           "(case sensitive): '%s'\n", p->limitinfo);
  if(table->size)
    {
      for(i=0;i<table->size;++i)
        if( !strcmp(type[i],"table") )
          {
            printf("\n%zu of %zu\n"
                   "==================\n"
                   "DATASET NAME: %s\n"
                   "------------------\n", i+1, table->size, name[i]);
            if(size)
              printf("DATASET SIZE (number of rows): %zu\n"
                     "------------------\n", size[i]);
            printf("DATASET DESCRIPTION:\n%s\n"
                   "==================\n", desc[i]);
          }
    }
  else printf("\nNO DATASET FOUND!\n");

  /* Clean up and return. */
  gal_list_data_free(table);
  gal_data_array_free(allcols, numcols, 0);
}





static uint8_t
query_type_from_tap(char *typestr)
{
  uint8_t type;
  if(      !strcmp(typestr,"BOOLEAN")  ) type=GAL_TYPE_UINT8;
  else if( !strcmp(typestr,"BIGINT")   ) type=GAL_TYPE_INT64;
  else if( !strcmp(typestr,"REAL")     ) type=GAL_TYPE_FLOAT32;
  else if( !strcmp(typestr,"DOUBLE")   ) type=GAL_TYPE_FLOAT64;
  else if( !strcmp(typestr,"SMALLINT")
           || !strcmp(typestr,"INTEGER") ) type=GAL_TYPE_INT32;
  else if( !strcmp(typestr,"VARCHAR")
           || !strcmp(typestr,"STRING")
           || !strncmp(typestr, "CHAR", 4) ) type=GAL_TYPE_STRING;
  else type=GAL_TYPE_INVALID;
  return type;
}




/* Read the downloaded metadata for all the columns within a table
   (dataset) and print them in an easy to read format. */
static void
query_output_meta_dataset(struct queryparams *p)
{
  size_t i;
  int tableformat;
  size_t numcols, numrows;
  gal_list_str_t *cols=NULL;
  gal_data_t *table, *allcols;
  char **name, **type, **unit, **desc;

  /* Get the downloaded metadata column information so we can ask for the
     proper columns. */
  allcols=gal_table_info(p->downloadname, "1", NULL, &numcols,
                         &numrows, &tableformat);

  /* Parse the column information to set the necessary columns. */
  if( query_output_meta_col(&cols, allcols, numcols, "column_name") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'column_name' found, but this "
            "is required by the IVOA TAP standard"); return; }
  if( query_output_meta_col(&cols, allcols, numcols, "datatype") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'datatype' found, but this "
            "is required by the IVOA TAP standard"); return; }
  if( query_output_meta_col(&cols, allcols, numcols, "description") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'description' found, but this "
            "is required by the IVOA TAP standard"); return; }
  if( query_output_meta_col(&cols, allcols, numcols, "unit") == 0 )
    { error(EXIT_SUCCESS, 0, "no 'unit' found, but this "
            "is required by the IVOA TAP standard"); return; }

  /* Read the necessary columns in the desired order, we just need to
     reverse the list first since it is last-in-first-out. */
  gal_list_str_reverse(&cols);
  table=gal_table_read(p->downloadname, "1", NULL, cols,
                       GAL_TABLE_SEARCH_NAME, 1, p->cp.minmapsize,
                       p->cp.quietmmap, NULL);

  /* It may happen that the required dataset name isn't recognized by the
     database. In this case, 'table' will have 0 rows. */
  if(table->size)
    {
      /* Free the initial 'allcols' and set the array pointers.  */
      name=table->array;
      type=table->next->array;
      desc=table->next->next->array;
      unit=table->next->next->next->array;
      gal_data_array_free(allcols, numcols, 0);

      /* Allocate the new 'allcols' and fill it with the metadata. */
      numcols=table->size;
      allcols=gal_data_array_calloc(numcols);
      for(i=0;i<numcols;++i)
        {
          allcols[i].type=query_type_from_tap(type[i]);
          gal_checkset_allocate_copy(name[i], &allcols[i].name);
          gal_checkset_allocate_copy(desc[i], &allcols[i].comment);
          if( !strcmp(unit[i]," ") ) allcols[i].unit=NULL;
          else gal_checkset_allocate_copy(unit[i], &allcols[i].unit);
        }

      /* Print the basic information. */
      printf("\n--------\ndatabase: %s (URL: %s)\ndataset: %s\n",
             p->databasestr, p->urls->v, p->datasetstr);
      gal_table_print_info(allcols, numcols, GAL_BLANK_SIZE_T);
    }
  else
    {
      /* We are using 'error' to have the progam name at the start, and so
         it goes to 'stderr'. But we aren't exiting with 'EXIT_FAILURE',
         because Query still has work to do (for example deleting the
         temporarily downloaded file). */
      printf("\n");
      error(EXIT_SUCCESS, 0, "no '%s' dataset found in the '%s' database. "
            "For the list of datasets within this database, please run the "
            "command below (put any search word or phrase in 'SEARCH' to "
            "find your dataset more easily):\n\n"
            "   astquery %s --information --limitinfo=\"SEARCH\"\n\n",
            p->datasetstr, p->databasestr, p->databasestr);
    }

  /* Clean up and return. */
  gal_list_data_free(table);
  gal_data_array_free(allcols, numcols, 0);
}





/* Read the raw download data, and write them into the output file with
   Gnuastro's own library. */
static void
query_output_data(struct queryparams *p)
{
  gal_data_t *table;

  /* Read the table and write it into a clean output (in case the
     downloaded table is compressed in any special FITS way). */
  table=gal_table_read(p->downloadname, "1", NULL, NULL,
                       GAL_TABLE_SEARCH_NAME, 1, p->cp.minmapsize,
                       p->cp.quietmmap, NULL);
  gal_table_write(table, NULL, NULL, p->cp.tableformat,
                  p->cp.output ? p->cp.output : p->cp.output,
                  "QUERY", 0);

  /* Get basic information about the table and free it. */
  p->outtableinfo[0]=table->size;
  p->outtableinfo[1]=gal_list_data_number(table);
  gal_list_data_free(table);
}





void
query_check_download(struct queryparams *p)
{
  size_t len;
  int status=0;
  char *logname;
  fitsfile *fptr;

  /* Open the FITS file and if the status value is still zero, it means
     everything worked properly. */
  fits_open_file(&fptr, p->downloadname, READONLY, &status);
  if(status==0)
    {
      /* Close the FITS file pointer. */
      fits_close_file(fptr, &status);

      /* Prepare the output dataset. */
      if(p->information)
        {
          if(p->datasetstr)  query_output_meta_dataset(p);
          else               query_output_meta_database(p);
        }
      else               query_output_data(p);

      /* Delete the raw downloaded file if necessary. */
      if(p->keeprawdownload==0) remove(p->downloadname);
    }
  else
    {
      /* Add a '.log' suffix to the output filename. */
      len=strlen(p->downloadname);
      logname=gal_pointer_allocate(GAL_TYPE_STRING, len+10, 1,
                                   __func__, "logname");
      sprintf(logname, "%s.log", p->downloadname);

      /* Rename the output file to the logname file and let the user
         know. */
      rename(p->downloadname, logname);
      if(p->cp.quiet==0) printf("\n");
      error(EXIT_FAILURE, 0, "the requested dataset could not be "
            "retrieved! For more, please see '%s'", logname);
    }

  /* Add the query keywords to the first extension (if the output was a
     FITS file). */
  if( p->information==0 && gal_fits_name_is_fits(p->cp.output) )
    {
      gal_fits_key_list_title_add_end(&p->cp.okeys,
                                      "Constructed query command", 0);
      gal_fits_key_list_fullcomment_add_end(&p->cp.okeys,
                                            p->finalcommand, 1);
      gal_fits_key_write_config(&p->cp.okeys, "Query settings",
                                "QUERY-CONFIG", p->cp.output, "0");
    }
}





void
query(struct queryparams *p)
{
  /* Download the dataset. */
  switch(p->database)
    {
    case QUERY_DATABASE_ASTRON: astron_prepare(p); break;
    case QUERY_DATABASE_GAIA:   gaia_prepare(p);   break;
    case QUERY_DATABASE_NED:    ned_prepare(p);    break;
    case QUERY_DATABASE_VIZIER: vizier_prepare(p); break;
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
            "address the problem. '%d' is not a recognized database "
            "code", __func__, PACKAGE_BUGREPORT, p->database);
    }

  /* Download the requested query. */
  tap_download(p);

  /* Make sure that the result is a readable FITS file, otherwise, abort
     with an error. */
  if(p->dryrun==0)
    query_check_download(p);

  /* Let the user know that things went well. */
  if(p->dryrun==0 && p->cp.quiet==0)
    {
      if(p->information==0)
        printf("\nQuery resulted in %zu rows and %zu columns.\n",
               p->outtableinfo[0], p->outtableinfo[1]);
      if(p->keeprawdownload)
        {
          if(p->information) printf("\n");
          printf("Query's raw downloaded file: %s\n", p->downloadname);
        }
      if(p->information==0)
        {
          printf("Query's final output: %s\n", p->cp.output);
          printf("TIP: use the command below for more on the "
                 "downloaded table:\n"
                 "   asttable %s --info\n", p->cp.output);
        }
    }

  /* Clean up. */
  gal_list_str_free(p->urls, 0);
}
