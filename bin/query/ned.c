/*********************************************************************
Settings for NED.
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
#include <string.h>

#include <gnuastro-internal/checkset.h>

#include "main.h"

#include "ui.h"
#include "tap.h"





static void
ned_sanity_checks(struct queryparams *p)
{
  /* Set the summarized names. */
  if(p->datasetstr)
    {
      if( !strcmp(p->datasetstr, "objdir") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("NEDTAP.objdir", &p->datasetstr);
        }
    }

  /* Currently NED only has a single table for TAP access, so warn the
     users about this if they ask for any other table. */
  if( p->datasetstr==NULL || strcmp(p->datasetstr, "NEDTAP.objdir") )
    error(EXIT_FAILURE, 0, "NED currently only supports a single "
          "dataset with the TAP protocol called 'NEDTAP.objdir' "
          "(which you can also call in Query with '--dataset=objdir'). "
          "TAP access to more datasets/tables will be provided in "
          "the future. To see all the column information and select "
          "the columns you want for your work, please run this command:\n\n"
          "    astquery %s --dataset=objdir --info", p->databasestr);
}





void
ned_prepare(struct queryparams *p)
{
  /* NED-specific. */
  ned_sanity_checks(p);

  /* Set the URLs, note that this is a simply-linked list, so we need to
     reverse it in the end (with 'gal_list_str_reverse') to have the same
     order here. */
  gal_list_str_add(&p->urls,
                   "https://ned.ipac.caltech.edu/tap/sync", 0);

  /* Name of default RA Dec columns. */
  if(p->ra_name==NULL)  p->ra_name="ra";
  if(p->dec_name==NULL) p->dec_name="dec";

  /* Basic sanity checks. */
  tap_sanity_checks(p);
}
