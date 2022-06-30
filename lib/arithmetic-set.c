/*********************************************************************
Arithmetic operations on data structures.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
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
#include <stdlib.h>
#include <string.h>

#include <gnuastro/list.h>

#include <gnuastro-internal/checkset.h>
#include <gnuastro-internal/arithmetic-set.h>





/* Remove a name from the list of names and return the dataset it points
   to. */
static gal_data_t *
arithmetic_set_remove_name(struct gal_arithmetic_set_params *p,
                           char *name)
{
  gal_data_t *tmp, *removed=NULL, *prev=NULL;

  /* Go over all the given names. */
  for(tmp=p->named;tmp!=NULL;tmp=tmp->next)
    {
      if( !strcmp(tmp->name, name) )
        {
          removed=tmp;
          if(prev) prev->next = tmp->next;
          else     p->named   = tmp->next;
        }

      /* Set this node as the 'prev' pointer. */
      prev=tmp;
    }

  /* A small sanity check. */
  if(removed==NULL)
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
          "fix the problem. 'removed' must not be NULL at this point",
          __func__, PACKAGE_BUGREPORT);

  /* Nothing in the list points to it now. So we can safely modify and
     return it. */
  free(removed->name);
  removed->next=NULL;
  removed->name=NULL;
  return removed;
}





/* Pop a dataset and keep it in the 'named' list for later use. */
void
gal_arithmetic_set_name(struct gal_arithmetic_set_params *p, char *token)
{
  gal_data_t *tmp, *tofree;
  char *varname=&token[ GAL_ARITHMETIC_SET_PREFIX_LENGTH ];

  /* If a dataset with this name already exists, it will be removed/deleted
     so we can use the name for the newly designated dataset. */
  for(tmp=p->named; tmp!=NULL; tmp=tmp->next)
    if( !strcmp(varname, tmp->name) )
      {
        tofree=arithmetic_set_remove_name(p, varname);
        gal_data_free(tofree);

        /* IMPORTANT: we MUST break here! 'tmp' does't point to the right
           place any more. We can define a 'prev' node and modify it on
           every attempt, but since there is only one dataset with a given
           name, that is redundant and will just make the program slow. */
        break;
      }

  /* Pop the top operand, then add it to the list of named datasets, but
     only if it is used in later tokens. If it isn't, free the popped
     dataset. The latter case (to define a name, but not use it), is
     obviously a redundant operation, but that is upto the user and may
     happen in scripts where the operands and operators list is
     automatically generated. We should just have everything in place, so
     no crashes occur or no extra memory is consumed. */
  if( p->used_later(p, varname) )
    {
      /* Add the top popped operand to the list of names. */
      gal_list_data_add(&p->named, p->pop(p));

      /* Write the requested name into this dataset. But note that 'name'
         MUST be already empty. So to be safe, we'll do a sanity check. */
      if(p->named->name)
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
              "fix the problem. The 'name' element should be NULL at "
              "this point, but it isn't", __func__, PACKAGE_BUGREPORT);
      if(p->named->unit)
        { free(p->named->unit); p->named->unit=NULL; }
      if(p->named->comment)
        { free(p->named->comment); p->named->comment=NULL; }
      gal_checkset_allocate_copy(varname, &p->named->name);
    }
  else
    {
      /* Pop the top operand, then free it: for example the user has ran
         'set-i', but forgot to actually use it (happens a lot due to human
         error!). */
      tmp=p->pop(p);
      gal_data_free(tmp);
    }
}





/* See if a given token is the name of a variable. */
int
gal_arithmetic_set_is_name(gal_data_t *named, char *token)
{
  gal_data_t *tmp;

  /* Make sure the variable name hasn't been set before. */
  for(tmp=named; tmp!=NULL; tmp=tmp->next)
    if( !strcmp(token, tmp->name) )
      return 1;

  /* If control reaches here, then there was no match*/
  return 0;
}





/* Return a copy of the named dataset. */
gal_data_t *
gal_arithmetic_set_copy_named(struct gal_arithmetic_set_params *p,
                              char *name)
{
  gal_data_t *out=NULL, *tmp;

  /* Find the proper named element to use. */
  for(tmp=p->named;tmp!=NULL;tmp=tmp->next)
    {
    if( !strcmp(tmp->name, name) )
      {
        /* If the named operand is used later, then copy it into the
           output. */
        if( p->used_later(p, name) )
          {
            out=gal_data_copy(tmp);
            out->next=NULL;
            if(out->name)    { free(out->name);    out->name=NULL;    }
            if(out->unit)    { free(out->unit);    out->unit=NULL;    }
            if(out->comment) { free(out->comment); out->comment=NULL; }
          }

        /* The named operand is not used any more. Remove it from the list
           of named datasets and continue. */
        else out=arithmetic_set_remove_name(p, name);
      }
    }

  /* A small sanity check. */
  if(out==NULL)
    error(EXIT_FAILURE, 0, "%s: a bug! please contact us at %s to fix the "
          "problem. The requested name '%s' couldn't be found in the list",
          __func__, PACKAGE_BUGREPORT, name);

  /* Return. */
  return out;
}
