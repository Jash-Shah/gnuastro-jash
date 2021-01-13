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
#include "gaia.h"
#include "query.h"
#include "vizier.h"




void
query_check_download(struct queryparams *p)
{
  size_t len;
  int status=0;
  char *logname;
  fitsfile *fptr;
  gal_data_t *table;

  /* Open the FITS file and if the status value is still zero, it means
     everything worked properly. */
  fits_open_file(&fptr, p->downloadname, READONLY, &status);
  if(status==0)
    {
      /* Close the FITS file pointer. */
      fits_close_file(fptr, &status);

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
      error(EXIT_FAILURE, 0, "the requested dataset could not be retrieved! "
            "For more, please see '%s'", logname);
    }

  /* Add the query keywords to the first extension (if the output was a
     FITS file). */
  if( gal_fits_name_is_fits (p->cp.output) )
    gal_fits_key_write_config(&p->cp.okeys, "Query settings",
                              "QUERY-CONFIG", p->cp.output, "0");

}





void
query(struct queryparams *p)
{
  /* Download the dataset. */
  switch(p->database)
    {
    case QUERY_DATABASE_GAIA:   gaia_prepare(p);   break;
    case QUERY_DATABASE_VIZIER: vizier_prepare(p); break;
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to address "
            "the problem. '%d' is not a recognized database code", __func__,
            PACKAGE_BUGREPORT, p->database);
    }

  /* Download the requested query. */
  tap_download(p);

  /* Make sure that the result is a readable FITS file, otherwise, abort
     with an error. */
  query_check_download(p);

  /* Let the user know that things went well. */
  if(p->cp.quiet==0)
    {
      printf("\nQuery resulted in %zu rows and %zu columns.\n",
             p->outtableinfo[0], p->outtableinfo[1]);
      if(p->keeprawdownload)
        printf("Query's raw downloaded file: %s\n", p->downloadname);
      printf("Query's final output: %s\n", p->cp.output);
    }

  /* Clean up. */
  gal_list_str_free(p->urls, 0);
}
