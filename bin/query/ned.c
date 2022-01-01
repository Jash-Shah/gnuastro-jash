/*********************************************************************
Settings for NED.
Query is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2021-2022 Free Software Foundation, Inc.

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

#include <gnuastro-internal/checkset.h>

#include "main.h"

#include "ui.h"
#include "tap.h"




/* Basic sanity checks. */
static void
ned_sanity_checks(struct queryparams *p)
{
  /* Set the summarized names. */
  if(p->datasetstr)
    {
      /* Correct the dataset name if 'objdir' is given. */
      if( !strcmp(p->datasetstr, "objdir") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("NEDTAP.objdir", &p->datasetstr);
        }

      /* Database-specific checks. For example, if we should use TAP or
         not. Note that the user may give 'NEDTAP.objdir', so we can't use
         the 'if' above (for expanding summarized names). */
      if( !strcmp(p->datasetstr, "NEDTAP.objdir") )
        p->usetap=1;
      else if( !strcmp(p->datasetstr, "extinction") )
        {
          /* Crash for options that are not compatible with extinction. */
          if( p->radius || p->width || p->range || p->noblank || p->columns
              || p->head!=GAL_BLANK_SIZE_T || p->sort )
            error(EXIT_FAILURE, 0, "NED's extinction calculator returns "
                  "the galactic extinction for a single point (in multiple "
                  "filters), therefore the following options are not "
                  "acceptable with it: '--radius', '--width', '--range', "
                  "'--noblank', '--column', '--head' and '--sort'");

          /* Make sure that '--center' is given. */
          if(p->center==NULL)
            error(EXIT_FAILURE, 0, "no coordinate specified! Please use "
                  "'--center' to specify the RA and Dec (in J2000) of "
                  "your desired coordinate, for example "
                  "--center=10.68458,41.269166");
        }
    }
  else
    error(EXIT_FAILURE, 0, "no dataset specified! Query only recognizes "
          "two datasets for NED: 'objdir' and 'extinction'. 'objdir' "
          "is in the IVOA Table Access Protocol (TAP) format, so you "
          "can see its available columns before downloading the actual "
          "data (to only download the small sub-set you need) with this "
          "command: 'astquery %s --dataset=objdir --info'. However, the "
          "'extinction' catalog isn't TAP-based, so the '--info' option "
          "isn't supported (but by its nature, the size of the "
          "extinction catalog is very small)", p->databasestr);

  /* Currently NED only has a single table for TAP access, so warn the
     users about this if they ask for any other table. */
  if( p->usetap
      && ( p->datasetstr==NULL
           || strcmp(p->datasetstr, "NEDTAP.objdir") ) )
    error(EXIT_FAILURE, 0, "NED currently only supports a single "
          "dataset with the TAP protocol called 'NEDTAP.objdir' "
          "(which you can also call in Query with '--dataset=objdir'). "
          "TAP access to more datasets/tables will be provided in "
          "the future. To see all the column information and select "
          "the columns you want for your work, please run this command:\n\n"
          "    astquery %s --dataset=objdir --info", p->databasestr);
}





/* Extinction with NED */
void
ned_extinction(struct queryparams *p)
{
  double *darr;
  char *command;

  /* If the user wants information, we'll specify a (0,0) center coordinate
     and continue. In the end, instead of saving the file, we'll just
     report the metadata. */
  if(p->information)
    error(EXIT_FAILURE, 0, "'--information' is not yet supported for "
          "NED's extinction calculator");

  /* Build the calling command. Note that the query quotes are
     included by the function building it. */
  darr=p->center->array;
  if( asprintf(&command, "curl%s -o%s 'https://ned.ipac.caltech.edu/cgi-bin/calc?in_csys=Equatorial&out_csys=Equatorial&in_equinox=J2000.0&out_equinox=J2000.0&obs_epoch=2000.0&lon=%fd&lat=%fd&of=xml_main&ext=1'", p->cp.quiet ? " -s" : "",
               p->downloadname, darr[0], darr[1])<0 )
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

  /* Run the command if '--dryrun' isn't called: if the command succeeds
     'system' returns 'EXIT_SUCCESS'. */
  if(p->dryrun==0)
    {
      if(system(command)!=EXIT_SUCCESS)
        error(EXIT_FAILURE, 0, "the query download command %sfailed%s\n",
              p->cp.quiet==0 ? "printed above " : "",
              p->cp.quiet==0 ? "" : " (the command can be printed "
              "if you don't use the option '--quiet', or '-q')");
    }
}





/* For NED's non-TAP queries. */
void
ned_non_tap(struct queryparams *p)
{
  if( !strcmp(p->datasetstr, "extinction") )
    ned_extinction(p);
}





void
ned_prepare(struct queryparams *p)
{
  /* NED-specific. */
  ned_sanity_checks(p);

  /* If we should use TAP, do the preparations. */
  if(p->usetap)
    {
      /* Set the URLs, note that this is a simply-linked list, so we need
         to reverse it in the end (with 'gal_list_str_reverse') to have the
         same order here. */
      gal_list_str_add(&p->urls,
                       "https://ned.ipac.caltech.edu/tap/sync", 0);

      /* Name of default RA Dec columns. */
      if(p->ra_name==NULL)  p->ra_name="ra";
      if(p->dec_name==NULL) p->dec_name="dec";

      /* Basic sanity checks. */
      tap_sanity_checks(p);
    }
  else
    ned_non_tap(p);
}
