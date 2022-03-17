/*********************************************************************
Table - View and manipulate a FITS table structures.
Table is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2019-2022 Free Software Foundation, Inc.

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

#include <gnuastro/wcs.h>
#include <gnuastro/type.h>

#include <gnuastro-internal/checkset.h>
#include <gnuastro-internal/arithmetic-set.h>

#include "main.h"
#include "arithmetic.h"





/*********************************************************************/
/********************       List operations      *********************/
/*********************************************************************/
static struct arithmetic_token *
arithmetic_add_new_to_end(struct arithmetic_token **list)
{
  struct arithmetic_token *tmp, *node;

  /* Allocate a new node. */
  errno=0;
  node=malloc(sizeof *node);
  if(node==NULL)
    error(EXIT_FAILURE, errno, "%s: couldn't allocate new node (%zu bytes)",
          __func__, sizeof *node);

  /* Initialize its elements. */
  node->next=NULL;
  node->name_def=NULL;
  node->name_use=NULL;
  node->constant=NULL;
  node->index=GAL_BLANK_SIZE_T;
  node->operator=GAL_ARITHMETIC_OP_INVALID;

  /* If the list already has elements, go to the last node in the list and
     add this node. */
  if(*list)
    {
      for(tmp=*list;tmp->next!=NULL;tmp=tmp->next) {}
      tmp->next=node;
    }
  else
    *list=node;

  /* Return a pointer to this node (to use temporarily). */
  return node;
}





void
arithmetic_token_free(struct arithmetic_token *list)
{
  struct arithmetic_token *tmp;
  while(list!=NULL)
    {
      /* Free allocated elements first. */
      if(list->name_def) free(list->name_def);
      if(list->name_use) free(list->name_use);

      /* Set the next list node, and free this one. */
      tmp=list->next;
      free(list);
      list=tmp;
    }
}


















/*********************************************************************/
/********************       User-interface       *********************/
/*********************************************************************/
static char *
arithmetic_operator_name(int operator)
{
  char *out=gal_arithmetic_operator_string(operator);

  /* If the operator wasn't in the library, see if it was defined here. */
  if(out==NULL)
    switch(operator)
      {
      case ARITHMETIC_TABLE_OP_SET: out="set"; break;
      case ARITHMETIC_TABLE_OP_WCSTOIMG: out="wcs-to-img"; break;
      case ARITHMETIC_TABLE_OP_IMGTOWCS: out="img-to-wcs"; break;
      case ARITHMETIC_TABLE_OP_DATETOSEC: out="date-to-sec"; break;
      case ARITHMETIC_TABLE_OP_DISTANCEFLAT: out="distance-flat"; break;
      case ARITHMETIC_TABLE_OP_DATETOMILLISEC: out="date-to-millisec"; break;
      case ARITHMETIC_TABLE_OP_DISTANCEONSPHERE: out="distance-on-sphere"; break;
      default:
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to fix "
              "the problem. %d is not a recognized operator code", __func__,
              PACKAGE_BUGREPORT, operator);
      }

  return out;
}





static void
arithmetic_init_wcs(struct tableparams *p, char *operator)
{
  /* If a WCS hasn't been read yet, read it.*/
  if(p->wcs==NULL)
    {
      /* A small sanity check. */
      if(p->wcsfile==NULL || p->wcshdu==NULL)
        error(EXIT_FAILURE, 0, "'--wcsfile' and '--wcshdu' are necessary "
              "for the '%s' operator", operator);

      /* Read the WCS. */
      p->wcs=gal_wcs_read(p->wcsfile, p->wcshdu, p->cp.wcslinearmatrix,
                          0, 0, &p->nwcs);
      if(p->wcs==NULL)
        error(EXIT_FAILURE, 0, "%s (hdu: %s): no WCS could be read by "
              "WCSLIB", p->wcsfile, p->wcshdu);
    }
}





/* Set the operator code from the given string. */
static int
arithmetic_set_operator(struct tableparams *p, char *string,
                        size_t *num_operands)
{
  int op=gal_arithmetic_set_operator(string, num_operands);;

  /* Set the operator and number of operands. */
  if( op==GAL_ARITHMETIC_OP_INVALID )
    {
      /* Simple operators. */
      if(      !strcmp(string, "wcs-to-img"))
        { op=ARITHMETIC_TABLE_OP_WCSTOIMG; *num_operands=0; }
      else if( !strcmp(string, "img-to-wcs"))
        { op=ARITHMETIC_TABLE_OP_IMGTOWCS; *num_operands=0; }
      else if( !strcmp(string, "date-to-sec"))
        { op=ARITHMETIC_TABLE_OP_DATETOSEC; *num_operands=0; }
      else if( !strcmp(string, "date-to-millisec"))
        { op=ARITHMETIC_TABLE_OP_DATETOMILLISEC; *num_operands=0; }
      else if( !strcmp(string, "distance-flat"))
        { op=ARITHMETIC_TABLE_OP_DISTANCEFLAT; *num_operands=0; }
      else if( !strcmp(string, "distance-on-sphere"))
        { op=ARITHMETIC_TABLE_OP_DISTANCEONSPHERE; *num_operands=0; }
      else
        { op=GAL_ARITHMETIC_OP_INVALID; *num_operands=GAL_BLANK_INT; }
    }

  /* Operator specific operations. */
  switch(op)
    {
    case ARITHMETIC_TABLE_OP_WCSTOIMG:
    case ARITHMETIC_TABLE_OP_IMGTOWCS:
      arithmetic_init_wcs(p, string);
      break;
    }

  /* Return the operator. */
  return op;
}





/* Initialize each column from an arithmetic operation. */
void
arithmetic_init(struct tableparams *p, struct arithmetic_token **arith,
                gal_list_str_t **toread, size_t *totcalled, char *expression)
{
  void *num;
  size_t one=1;
  uint8_t ntype;
  char *str, *delimiter=" \t";
  struct arithmetic_token *atmp, *node=NULL;
  char *token=NULL, *lasttoken=NULL, *saveptr;

  /* Parse all the given tokens. */
  token=strtok_r(expression, delimiter, &saveptr);
  while(token!=NULL)
    {
      /* Allocate and initialize this arithmetic token. */
      lasttoken=token;
      node=arithmetic_add_new_to_end(arith);

      /* See if the token is an operator, if not check other cases.  */
      node->operator=arithmetic_set_operator(p, token, &node->num_operands);
      if(node->operator==GAL_ARITHMETIC_OP_INVALID)
        {
          /* Token is a single number.*/
          if( (num=gal_type_string_to_number(token, &ntype)) )
            node->constant=gal_data_alloc(num, ntype, 1, &one, NULL, 0,
                                          -1, 1, NULL, NULL, NULL);

          /* This is a 'set-' operator. */
          else if( !strncmp(token, GAL_ARITHMETIC_SET_PREFIX,
                            GAL_ARITHMETIC_SET_PREFIX_LENGTH) )
            {
              node->num_operands=0;
              node->operator=ARITHMETIC_TABLE_OP_SET;
              gal_checkset_allocate_copy(token, &node->name_def);
            }

          /* Non-pre-defined situation.*/
          else
            {
              /* See if this is an already defined name. */
              for(atmp=*arith; atmp!=NULL; atmp=atmp->next)
                if( atmp->name_def
                    && strcmp(token,
                              ( atmp->name_def
                                + GAL_ARITHMETIC_SET_PREFIX_LENGTH) )==0 )
                  gal_checkset_allocate_copy(token, &node->name_use);

              /* If it wasn't found to be a pre-defined name, then its a
                 column operand (column number or name). */
              if(node->name_use==NULL)
                {
                  str = ( (token[0]=='$' && isdigit(token[1]))
                          ? &token[1]   /* Column number (starting with '$'). */
                          : token );    /* Column name, just add it.          */
                  gal_list_str_add(toread, str, 1);
                  node->index=*totcalled;
                  *totcalled+=1;
                }
            }
        }

      /* Go to the next token. */
      token=strtok_r(NULL, delimiter, &saveptr);
    }

  /* A small sanity check: the last added token must be an operator. */
  if( node==NULL || node->operator==GAL_ARITHMETIC_OP_INVALID )
    error(EXIT_FAILURE, 0, "last token in arithmetic column ('%s') is not a "
          "recognized operator", lasttoken);
}





/* Set the final index of each package of columns (possibly containing
   processing columns that will change in number and contents).  */
void
arithmetic_indexs_final(struct tableparams *p, size_t *colmatch)
{
  size_t startind;
  size_t i, numcols;
  struct column_pack *tmp;
  struct arithmetic_token *atmp;

  /* Set the column array that will allow removal of some read columns
     (where operations will be done). */
  p->colarray=gal_list_data_to_array_ptr(p->table, &p->numcolarray);

  /* go over each package of columns. */
  for(tmp=p->outcols;tmp!=NULL;tmp=tmp->next)
    {
      /* If we are on an arithmetic operation. */
      if(tmp->arith)
        {
          for(atmp=tmp->arith;atmp!=NULL;atmp=atmp->next)
            if(atmp->index!=GAL_BLANK_SIZE_T)
              {
                /* Small sanity check. */
                if(colmatch[atmp->index]!=1)
                  error(EXIT_FAILURE, 0, "arithmetic operands can (currently) "
                        "only correspond to a single column");

                /* Update the index in the full list of read columns. */
                numcols=0; for(i=0;i<atmp->index;++i) numcols+=colmatch[i];
                atmp->index=numcols;
              }
        }
      /* A simple column. */
      else
        {
          /* See where the starting column for this patch of simple columns
             is. */
          startind=0;
          for(i=0;i<tmp->start;++i) startind+=colmatch[i];

          /* How many of the read columns are associated with the this
             patch of columns. */
          numcols=0;
          for(i=0;i<tmp->numsimple;++i) numcols+=colmatch[tmp->start+i];

          /* Update the values. */
          tmp->start=startind;
          tmp->numsimple=numcols;
        }

    }
}




















/*********************************************************************/
/********************       Low-level tools      *********************/
/*********************************************************************/
static gal_data_t *
arithmetic_stack_pop(gal_data_t **stack, int operator, char *errormsg)
{
  gal_data_t *out=*stack;

  /* Update the stack. */
  if(*stack)
    *stack=(*stack)->next;
  else
    error(EXIT_FAILURE, 0, "not enough operands for '%s'%s",
          arithmetic_operator_name(operator), errormsg?errormsg:"");

  /* Arithmetic changes the contents of a dataset, so the old name (in the
     FITS 'EXTNAME' keyword) must not be used beyond this
     point. Furthermore, in Arithmetic, the 'name' element is used to
     identify variables (with the 'set-' operator). */
  if(out->name)    { free(out->name);    out->name=NULL;    }
  if(out->unit)    { free(out->unit);    out->unit=NULL;    }
  if(out->comment) { free(out->comment); out->comment=NULL; }

  /* Remove the 'next' element to break from the stack and return. */
  out->next=NULL;
  return out;
}





/* Wrapper function to pop operands within the 'set-' operator. */
static gal_data_t *
arithmetic_stack_pop_wrapper_set(void *in)
{
  struct gal_arithmetic_set_params *tprm
    = (struct gal_arithmetic_set_params *)in;
  gal_data_t **stack=(gal_data_t **)tprm->params;
  return arithmetic_stack_pop(stack, ARITHMETIC_TABLE_OP_SET, NULL);
}





/* For the 'set-' operator. */
static int
arithmetic_set_name_used_later(void *in, char *name)
{
  struct gal_arithmetic_set_params *p=(struct gal_arithmetic_set_params *)in;
  struct arithmetic_token *tokens = (struct arithmetic_token *)(p->tokens);
  struct arithmetic_token *token;

  size_t counter=0;

  /* If the name exists after the current token, then return 1. Note that
     we have already separated the usage of names set with 'set-' in the
     'name_use' element of the 'token' structure. */
  for(token=tokens;token!=NULL;token=token->next)
    {
      if( token->name_use
          && counter > p->tokencounter
          && strcmp(token->name_use, name)==0 )
        return 1;

      /* Increment the counter (has to be after the check above because it
         may not always get to the 'counter' variable). */
      ++counter;
    }

  /* If we get to this point, it means that the name doesn't exist. */
  return 0;
}





/* Set the converted column metadata. */
static void
arithmetic_update_metadata(gal_data_t *col, char *name, char *unit,
                           char *comment)
{
  if(col)
    {
      if(col->name)    free(col->name);
      if(col->unit)    free(col->unit);
      if(col->comment) free(col->comment);
      gal_checkset_allocate_copy(name, &col->name);
      gal_checkset_allocate_copy(unit, &col->unit);
      gal_checkset_allocate_copy(comment, &col->comment);
    }
}




















/*********************************************************************/
/********************          Operations        *********************/
/*********************************************************************/
static void
arithmetic_wcs(struct tableparams *p, gal_data_t **stack, int operator)
{
  gal_data_t *tmp;
  char errormsg[100];
  struct wcsprm *wcs=p->wcs;
  size_t i, ndim=p->wcs->naxis;
  gal_data_t *coord[3]={NULL, NULL, NULL};

  /* Prepare the (possibly necessaary!) error message for the number of
     popped operand. */
  sprintf(errormsg, " (input WCS has %zu dimensions)", ndim);

  /* Pop all the necessary datasets and make sure they are
     double-precision. NOTE: the top dataset on the stack is the
     highest-dimensional dataset. */
  for(i=0;i<ndim;++i)
    {
      tmp=arithmetic_stack_pop(stack, operator, errormsg);
      tmp=gal_data_copy_to_new_type_free(tmp, GAL_TYPE_FLOAT64);
      coord[ndim-i-1]=tmp;
    }

  /* Define the list of coordinates. */
  if(coord[1]) coord[0]->next=coord[1];
  if(coord[2]) coord[1]->next=coord[2];

  /* Final preparations. */
  if(operator==ARITHMETIC_TABLE_OP_WCSTOIMG)
    {
      /* Do the conversion. */
      gal_wcs_world_to_img(coord[0], wcs, 1);

      /* For image coordinates, we don't need much precision. */
      for(i=0;i<ndim;++i)
        coord[i]=gal_data_copy_to_new_type_free(coord[i], GAL_TYPE_FLOAT32);

      /* Set the names, units and comments for each dataset. */
      arithmetic_update_metadata(coord[0],"X","pixel","Converted from WCS");
      arithmetic_update_metadata(coord[1],"Y","pixel","Converted from WCS");
      arithmetic_update_metadata(coord[2],"Z","pixel","Converted from WCS");
    }
  else
    {
      gal_wcs_img_to_world(coord[0], wcs, 1);
      arithmetic_update_metadata(coord[0], wcs->ctype[0], wcs->cunit[0],
                                 "Converted from pixel coordinates");
      arithmetic_update_metadata(coord[1], coord[1]?wcs->ctype[1]:NULL,
                                 coord[1]?wcs->cunit[1]:NULL,
                                 "Converted from pixel coordinates");
      arithmetic_update_metadata(coord[2], coord[2]?wcs->ctype[2]:NULL,
                                 coord[2]?wcs->cunit[2]:NULL,
                                 "Converted from pixel coordinates");
    }

  /* Reverse the column orders and put them on the stack. */
  for(i=0;i<ndim;++i)
    {
      coord[i]->next=NULL;
      gal_list_data_add(stack, coord[i]);
    }
}





static double
arithmetic_distance_flat(double a1, double a2, double b1, double b2)
{
  double d1=a1-b1, d2=a2-b2;
  return sqrt(d1*d1 + d2*d2);
}





static void
arithmetic_distance(struct tableparams *p, gal_data_t **stack, int operator)
{
  size_t i, j;
  double *o, *a1, *a2, *b1, *b2;
  gal_data_t *a, *b, *tmp, *out;
  char *colname=NULL, *colcomment=NULL;
  double (*distance_func)(double, double, double, double)=NULL;

  /* Pop the columns for point 'b'.*/
  tmp=arithmetic_stack_pop(stack, operator, NULL);
  tmp=gal_data_copy_to_new_type_free(tmp, GAL_TYPE_FLOAT64);
  b=arithmetic_stack_pop(stack, operator, NULL);
  b=gal_data_copy_to_new_type_free(b, GAL_TYPE_FLOAT64);
  b->next=tmp;

  /* Pop the columns for point 'a'.*/
  tmp=arithmetic_stack_pop(stack, operator, NULL);
  tmp=gal_data_copy_to_new_type_free(tmp, GAL_TYPE_FLOAT64);
  a=arithmetic_stack_pop(stack, operator, NULL);
  a=gal_data_copy_to_new_type_free(a, GAL_TYPE_FLOAT64);
  a->next=tmp;

  /* Make sure the sizes are consistant: note that one point can have a
     single coordinate, but we don't know which one. */
  if(a->size!=a->next->size)
    error(EXIT_FAILURE, 0, "the sizes of the third and fourth operands "
          "of the '%s' operator (respectively containing %zu and %zu "
          "numbers) must be equal", arithmetic_operator_name(operator),
          a->next->size, a->size);
  if(b->size!=b->next->size)
    error(EXIT_FAILURE, 0, "the sizes of the first and second operands "
          "of the '%s' operator (respectively containing %zu and %zu "
          "numbers) must be equal", arithmetic_operator_name(operator),
          b->next->size, b->size);

  /* Basic settings based on the operator. */
  switch(operator)
    {
    case ARITHMETIC_TABLE_OP_DISTANCEFLAT:
      colname="dist-flat";
      distance_func=arithmetic_distance_flat;
      colcomment="Distance measured on a flat surface.";
      break;
    case ARITHMETIC_TABLE_OP_DISTANCEONSPHERE:
      colname="dist-spherical";
      distance_func=gal_wcs_angular_distance_deg;
      colcomment="Distance measured on a great circle.";
      break;
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
            "fix the problem. The operator code %d isn't recognized",
            __func__, PACKAGE_BUGREPORT, operator);
    }

  /* Make the output array based on the largest size. */
  out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1,
                     (a->size>b->size ? &a->size : &b->size), NULL, 0,
                     p->cp.minmapsize, p->cp.quietmmap, colname, NULL,
                     colcomment);

  /* Measure the distances.  */
  o=out->array;
  a1=a->array; a2=a->next->array;
  b1=b->array; b2=b->next->array;
  if(a->size==1 || b->size==1) /* One of them is a single point. */
    for(i=0;i<a->size;++i)
      for(j=0;j<b->size;++j)
        o[a->size>b->size?i:j] = distance_func(a1[i], a2[i], b1[j], b2[j]);
  else                         /* Both have the same length. */
    for(i=0;i<a->size;++i)     /* (all were originally from the same table) */
      o[i] = distance_func(a1[i], a2[i], b1[i], b2[i]);

  /* Clean up and put the output dataset onto the stack. */
  gal_list_data_free(a);
  gal_list_data_free(b);
  gal_list_data_add(stack, out);
}





/* Convert the ISO date format to seconds since Unix time. */
static void
arithmetic_datetosec(struct tableparams *p, gal_data_t **stack,
                     int operator)
{
  size_t i, v;
  int64_t *iarr;
  gal_data_t *out;
  double subsec=NAN;
  char *subsecstr=NULL;
  char *unit=NULL, *name=NULL, *comment=NULL;

  /* Input dataset. */
  gal_data_t *in=arithmetic_stack_pop(stack, operator, NULL);
  char **strarr=in->array;

  /* Make sure the input has a 'string' type. */
  if(in->type!=GAL_TYPE_STRING)
    error(EXIT_FAILURE, 0, "the operand given to 'date-to-sec' "
          "should have a string type, but it is '%s'",
          gal_type_name(in->type, 1));

  /* Output metadata. */
  switch(operator)
    {
    case ARITHMETIC_TABLE_OP_DATETOSEC:
      unit="sec";
      name="UNIXSEC";
      comment="Unix seconds (from 00:00:00 UTC, 1 January 1970)";
      break;
    case ARITHMETIC_TABLE_OP_DATETOMILLISEC:
      unit="msec";
      name="UNIXMILLISEC";
      comment="Unix milli-seconds (from 00:00:00 UTC, 1 January 1970)";
      break;
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s "
            "to fix the problem. The operator code %d isn't "
            "recognized", __func__, PACKAGE_BUGREPORT, operator);
    }

  /* Allocate the output dataset. */
  out=gal_data_alloc(NULL, GAL_TYPE_INT64, 1, &in->size, NULL, 1,
                     p->cp.minmapsize, p->cp.quietmmap, name, unit,
                     comment);

  /* Convert each input string into number of seconds. */
  iarr=out->array;
  for(i=0; i<in->size; ++i)
    {
      /* Read the number of seconds and sub-seconds and write into the
         output. */
      v=gal_fits_key_date_to_seconds(strarr[i], &subsecstr,
                                     &subsec);
      iarr[i] = ( v==GAL_BLANK_SIZE_T
                  ? GAL_BLANK_INT64
                  : ( operator == ARITHMETIC_TABLE_OP_DATETOSEC
                      ? v
                      : (isnan(subsec)
                         ? v*1000
                         : v*1000 + (int64_t)(subsec*1000) ) ) );
    }

  /* Clean up and put the resulting calculation back on the stack. */
  if(in) gal_data_free(in);
  gal_list_data_add(stack, out);
}




















/*********************************************************************/
/********************          Operations        *********************/
/*********************************************************************/
static void
arithmetic_placeholder_name(gal_data_t *col)
{
  static size_t counter=0;

  /* Increment counter next time this function is called. */
  ++counter;

  /* Free any possibly existing metadata. */
  if(col->name)    free(col->name);
  if(col->unit)    free(col->unit);
  if(col->comment) free(col->comment);

  /* Set the new meta-data. */
  errno=0;
  if( asprintf(&col->name, "ARITH_%zu", counter)==-1 )
    error(EXIT_FAILURE, errno, "%s: asprintf error for name", __func__);
  if( asprintf(&col->unit, "arith_unit_%zu", counter)==-1)
    error(EXIT_FAILURE, errno, "%s: asprintf error for unit", __func__);
  if( asprintf(&col->comment, "Column from arithmetic operation %zu",
               counter)==-1 )
    error(EXIT_FAILURE, errno, "%s: asprintf error for comment", __func__);
}





static void
arithmetic_operator_run(struct tableparams *p,
                        struct arithmetic_token *token,
                        struct gal_arithmetic_set_params *setprm,
                        gal_data_t **stack)
{
  int flags=GAL_ARITHMETIC_FLAGS_BASIC;
  gal_data_t *d1=NULL, *d2=NULL, *d3=NULL;

  /* Set the operating-mode flags if necessary. */
  if(p->cp.quiet) flags |= GAL_ARITHMETIC_FLAG_QUIET;
  if(p->envseed)  flags |= GAL_ARITHMETIC_FLAG_ENVSEED;

  /* When 'num_operands!=0', the operator is in the library. */
  if(token->num_operands)
    {
      /* Pop the necessary number of operators. Note that the
         operators are poped from a linked list (which is
         last-in-first-out). So for the operators which need a
         specific order, the first poped operand is actally the
         last (right most, in in-fix notation) input operand.*/
      switch(token->num_operands)
        {
        case 1:
          d1=arithmetic_stack_pop(stack, token->operator, NULL);
          break;

        case 2:
          d2=arithmetic_stack_pop(stack, token->operator, NULL);
          d1=arithmetic_stack_pop(stack, token->operator, NULL);
          break;

        case 3:
          d3=arithmetic_stack_pop(stack, token->operator, NULL);
          d2=arithmetic_stack_pop(stack, token->operator, NULL);
          d1=arithmetic_stack_pop(stack, token->operator, NULL);
          break;

        case -1:
          error(EXIT_FAILURE, 0, "operators with a variable number of "
                "operands are not yet implemented. Please contact us at "
                "%s to include them", PACKAGE_BUGREPORT);
          break;

        default:
          error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to fix "
                "the problem. '%zu' is not recognized as an operand "
                "counter (with '%s')", __func__, PACKAGE_BUGREPORT,
                token->num_operands,
                arithmetic_operator_name(token->operator));
        }

      /* Run the arithmetic operation. Note that 'gal_arithmetic' is a
         variable argument function (like printf). So the number of
         arguments it uses depend on the operator. In other words, when the
         operator doesn't need three operands, the extra arguments will be
         ignored. */
      gal_list_data_add(stack, gal_arithmetic(token->operator, p->cp.numthreads,
                                              flags, d1, d2, d3) );

      /* Reset the meta-data for the element that was just put on the
         stack. */
      arithmetic_placeholder_name(*stack);
    }

  /* This operator is specific to this program (Table). */
  else
    {
      switch(token->operator)
        {
        case ARITHMETIC_TABLE_OP_WCSTOIMG:
        case ARITHMETIC_TABLE_OP_IMGTOWCS:
          arithmetic_wcs(p, stack, token->operator);
          break;

        case ARITHMETIC_TABLE_OP_DATETOSEC:
        case ARITHMETIC_TABLE_OP_DATETOMILLISEC:
          arithmetic_datetosec(p, stack, token->operator);
          break;

        case ARITHMETIC_TABLE_OP_DISTANCEFLAT:
        case ARITHMETIC_TABLE_OP_DISTANCEONSPHERE:
          arithmetic_distance(p, stack, token->operator);
          break;

        case ARITHMETIC_TABLE_OP_SET:
          gal_arithmetic_set_name(setprm, token->name_def);
          break;

        default:
          error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
                "fix the problem. The operator code %d is not recognized",
                __func__, PACKAGE_BUGREPORT, token->operator);
        }
    }
}





/* Apply reverse polish mechanism for this column. */
static void
arithmetic_reverse_polish(struct tableparams *p, struct column_pack *outpack)
{
  struct arithmetic_token *token;
  gal_data_t *single, *stack=NULL;
  struct gal_arithmetic_set_params setprm={0};

  /* Initialize the arithmetic functions/pointers. */
  setprm.params=&stack;
  setprm.tokens=outpack->arith;
  setprm.pop=arithmetic_stack_pop_wrapper_set;
  setprm.used_later=arithmetic_set_name_used_later;

  /* Go through all the tokens given to this element. */
  for(token=outpack->arith;token!=NULL;token=token->next)
    {
      /* We are on an operator. */
      if(token->operator!=GAL_ARITHMETIC_OP_INVALID)
        arithmetic_operator_run(p, token, &setprm, &stack);

      /* We are on a named variable. */
      else if( token->name_use )
        gal_list_data_add(&stack,
                          gal_arithmetic_set_copy_named(&setprm,
                                                        token->name_use));

      /* Constant number: just put it ontop of the stack. */
      else if(token->constant)
        {
          gal_list_data_add(&stack, token->constant);
          token->constant=NULL;
        }

      /* A column from the table. */
      else if(token->index!=GAL_BLANK_SIZE_T)
        gal_list_data_add(&stack, p->colarray[token->index]);

      /* Un-recognized situation. */
      else
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
              "fix the problem. The token can't be identified as an "
              "operator, constant or column", __func__, PACKAGE_BUGREPORT);

      /* Increment the token counter. */
      ++setprm.tokencounter;
    }

  /* Put everything that remains in the stack (reversed) into the final
     table. Just note that 'gal_list_data_add' behaves differently for
     lists, so we'll add have to manually set the 'next' element to NULL
     before adding the column to the final table. */
  gal_list_data_reverse(&stack);
  while(stack!=NULL)
    {
      /* Keep the top element in 'single' and move 'stack' to the next
         element. */
      single=stack;
      stack=stack->next;

      /* A small sanity check. */
      if(single->size==1 && p->table && single->size!=p->table->size)
        error(EXIT_FAILURE, 0, "the arithmetic operation resulted in a "
              "single value, but other columns have also been requested "
              "which have more elements/rows");

      /* Set 'single->next' to NULL so it isn't treated as a list, and set
         all flags to zero (the arithmetic operation may have changed what
         the flags pointed to). */
      single->flag=0;
      single->next=NULL;
      gal_list_data_add(&p->table, single);
    }
}




















/*********************************************************************/
/********************         High-level         *********************/
/*********************************************************************/
void
arithmetic_operate(struct tableparams *p)
{
  size_t i;
  struct column_pack *outpack;

  /* From now on, we will be looking for columns from the index in
     'colarray', so to keep things clean, we'll set all the 'next' elements
     to NULL. */
  for(i=0; i<p->numcolarray; ++i) p->colarray[i]->next=NULL;

  /* We'll also reset the output table pointer, to fill it in as we
     progress. */
  p->table=NULL;

  /* Go over each package of columns. */
  for(outpack=p->outcols; outpack!=NULL; outpack=outpack->next)
    {
      if(outpack->arith)
        arithmetic_reverse_polish(p, outpack);
      else
        {
          for(i=0;i<outpack->numsimple;++i)
            gal_list_data_add(&p->table, p->colarray[outpack->start+i]);
        }
    }

  /* Reverse the final output to be in the proper order. Note that all the
     column contents have either been moved into the new table, or have
     already been freed. */
  gal_list_data_reverse(&p->table);
}
