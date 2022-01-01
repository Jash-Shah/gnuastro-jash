/*********************************************************************
Settings for ESA's Gaia.
Query is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2020-2022 Free Software Foundation, Inc.

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





/* Gaia-specific: use simpler names for the commonly used Gaia
   datasets. This is relevant anytime that '--dataset' has been called. */
static void
gaia_sanity_checks(struct queryparams *p)
{
  /* FIRST CHECK (BEFORE SETTING DEFAULT DATASET): Gaia datasets are large
     and it doesn't allow downloading of the full dataset anonymously! You
     need to contact them for that. So if no constraints are given for the
     rows, a warning will be printed. */
  if(p->information==0
     && p->datasetstr
     && p->query==NULL
     && p->center==NULL
     && p->range==NULL
     && p->overlapwith==NULL )
    error(EXIT_FAILURE, 0, "no constraints specified! "
          "In other words, you are asking for all the rows within this "
          "dataset! Gaia datasets have billions of rows, therefore it "
          "has a limit on the number of rows downloaded anonymously. "
          "For bulk download access, you should contact "
          "'gaia-helpdesk@cosmos.esa.int'. Alternatively, you can "
          "constrain your search to a certain spatial region with "
          "'--center=RA,DEC' (supplemented by '--radius' or '--width' in "
          "degrees) or use '--overlapwith' to only download rows that "
          "overlap with the provided image. See the documentation for "
          "more with this command: 'info astquery' (press 'q' to return "
          "to the command-line).");

  /* Fix the summarized names. */
  if(p->datasetstr)
    {
      if( !strcmp(p->datasetstr, "edr3") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("gaiaedr3.gaia_source", &p->datasetstr);
        }
      else if( !strcmp(p->datasetstr, "dr2") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("gaiadr2.gaia_source", &p->datasetstr);
        }
      else if( !strcmp(p->datasetstr, "dr1") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("gaiadr1.gaia_source", &p->datasetstr);
        }
      else if( !strcmp(p->datasetstr, "hipparcos") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("public.hipparcos", &p->datasetstr);
        }
      else if( !strcmp(p->datasetstr, "tycho2") )
        {
          free(p->datasetstr);
          gal_checkset_allocate_copy("public.tycho2", &p->datasetstr);
        }
    }

  /* Currently we assume GAIA only uses TAP. */
  p->usetap=1;
}





void
gaia_prepare(struct queryparams *p)
{
  /* Gaia-specific settings. */
  gaia_sanity_checks(p);

  /* Set the URLs, note that this is a simply-linked list, so we need to
     reverse it in the end (with 'gal_list_str_reverse') to have the same
     order here. */
  gal_list_str_add(&p->urls,
                   "https://gea.esac.esa.int/tap-server/tap/sync", 0);

  /* Name of default RA Dec columns. */
  if(p->ra_name==NULL)  p->ra_name="ra";
  if(p->dec_name==NULL) p->dec_name="dec";

  /* Basic sanity checks. */
  tap_sanity_checks(p);
}
