/*********************************************************************
Access VizieR servers for query.
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





void
vizier_prepare(struct queryparams *p)
{
  /* Make sure that atleast one type of constraint is specified. */
  if(p->query==NULL && p->center==NULL && p->overlapwith==NULL)
    error(EXIT_FAILURE, 0, "no '--query', '--center' or '--overlapwith' "
          "specified. At least one of these options are necessary in the "
          "Gaia dataset");

  /* If '--center' is given, '--radius' is also necessary. */
  if(p->center || p->overlapwith)
    {
      /* Make sure the radius is given, and that it isn't zero. */
      if(p->overlapwith==NULL && p->radius==NULL && p->width==NULL)
        error(EXIT_FAILURE, 0, "the '--radius' ('-r') or '--width' ('-w') "
              "options are necessary with the '--center' ('-C') option");

      /* If no dataset is explicitly given, let the user know that a
         catalog reference is necessary. */
      if( p->datasetstr==NULL)
        error(EXIT_FAILURE, 0, "no '--dataset' specified. You can "
              "use the URL below to search existing datasets "
              "(catalogs):\n\n"
              "   http://cdsarc.u-strasbg.fr/viz-bin/cat");
    }

  /* Set the URLs, note that this is a simply-linked list, so we need to
     reverse it in the end to have the same order here. */
  gal_list_str_add(&p->urls, "http://tapvizier.u-strasbg.fr/TAPVizieR/tap/sync", 0);

  /* These don't seem to be working as of January 2021.
  gal_list_str_add(&p->urls, "https://vizier.cfa.harvard.edu/TAPVizieR/tap/sync", 0);
  gal_list_str_add(&p->urls, "http://vizier.hia.nrc.ca/TAPVizieR/tap/sync", 0);
  gal_list_str_add(&p->urls, "http://vizier.nao.ac.jp/TAPVizieR/tap/sync", 0);
  gal_list_str_add(&p->urls, "http://data.bao.ac.cn/TAPVizieR/tap/sync", 0);
  gal_list_str_add(&p->urls, "http://vizier.ast.cam.ac.uk/TAPVizieR/tap/sync", 0);
  gal_list_str_add(&p->urls, "http://www.ukirt.jach.hawaii.edu/TAPVizieR/tap/sync", 0);
  gal_list_str_add(&p->urls, "http://vizier.inasan.ru/TAPVizieR/tap/sync", 0);
  */
  gal_list_str_reverse(&p->urls);

  /* Name of RA Dec columns to use in Gaia. */
  p->ra_name="RAJ2000";
  p->dec_name="DEJ2000";
}
