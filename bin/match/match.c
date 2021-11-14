/*********************************************************************
Match - A program to match catalogs and WCS warps
Match is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
     Sachin Kumar Singh <sachinkumarsingh092@gmail.com>
Copyright (C) 2017-2021, Free Software Foundation, Inc.

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

#include <gnuastro/match.h>
#include <gnuastro/table.h>
#include <gnuastro/kdtree.h>
#include <gnuastro/pointer.h>
#include <gnuastro/threads.h>
#include <gnuastro/permutation.h>

#include <gnuastro-internal/timing.h>
#include <gnuastro-internal/checkset.h>

#include <main.h>



/* Number of columns in a file. */
static gal_list_str_t *
match_add_all_cols(char *filename, char *extname, gal_list_str_t *stdinlines,
                   gal_list_str_t *incols, size_t *num)
{
  char *tstr;
  int tableformat;
  gal_data_t *colinfo=NULL;
  gal_list_str_t *tmp, *finalcols=NULL;
  size_t i, numrows, numcols=GAL_BLANK_SIZE_T;

  /* Go over all the given input columns. */
  for(tmp=incols; tmp!=NULL; tmp=tmp->next)
    {
      if(!strcmp(tmp->v,"_all"))
        {
          /* Read all the column information (if it hasn't been read until
             now). */
          if( numcols == GAL_BLANK_SIZE_T )
            {
              colinfo=gal_table_info(filename, extname,
                                     filename ? NULL : stdinlines, &numcols,
                                     &numrows, &tableformat);
              gal_data_array_free(colinfo, numcols, 1);
            }

          /* Add each column number to the list of columns. */
          for(i=0;i<numcols;++i)
            {
              errno=0;
              if( asprintf(&tstr, "%zu", i+1)<0 )
                error(EXIT_FAILURE, errno, "asprintf allocation");
              gal_list_str_add(&finalcols, tstr, 0);
            }
        }
      else
        gal_list_str_add(&finalcols, tmp->v, 1);
    }

  /* If a new list of columns is ready, re-order tham and write
     them in. Note that there may be multiple '_all' terms, so we
     need to do this after parsing all the requested columns. */
  gal_list_str_reverse(&finalcols);

  /* For a check.
  gal_list_str_print(finalcols);
  exit(1);
  */

  /* Clean up and return. */
  *num=numcols;
  return finalcols;
}





static gal_data_t *
match_cat_from_coord(struct matchparams *p, gal_list_str_t *cols,
                     size_t *numcolmatch)
{
  void *rptr;
  gal_list_str_t *col;
  uint8_t read, readtype;
  size_t colcounter, counter;
  gal_data_t *tmp, *ttmp, *out=NULL;

  /* Go over the desired columns and only return the good ones. */
  colcounter=0;
  for(col=cols;col!=NULL;col=col->next)
    {
      /* In 'ui_preparations_out_cols', we have done the necessary sanity
         checks, so we can safely use the values. */
      rptr=gal_type_string_to_number(col->v, &readtype);
      if(readtype!=GAL_TYPE_UINT8)
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to fix "
              "the problem. The given string didn't have a 'uint8' type",
              __func__, PACKAGE_BUGREPORT);
      read=*((uint8_t *)rptr);

      /* Find the proper column in the second input's columns. Just note
         that column counting starts from 1.*/
      counter=1;
      for(tmp=p->cols2;tmp!=NULL;tmp=tmp->next)
        if(counter++ == read)
          {
            ttmp=gal_data_copy(tmp);
            ttmp->next=NULL;
            gal_list_data_add(&out, ttmp);
            ++numcolmatch[colcounter];
            break;
          }

      /* Increment the column counter. */
      ++colcounter;
    }

  /* Reverse the list. */
  gal_list_data_reverse(&out);

  /* Return the output columns. */
  return out;
}





/*static*/ void /* Static is commented to avoid warning. */
match_catalog_permute_inplace(struct matchparams *p, gal_data_t *in,
                              size_t *permutation, size_t nummatched)
{
  char **strarr;
  size_t i, numnotmatched;

  /* Do the permutation. */
  gal_permutation_apply(in, permutation);

  /* Correct the size of the array so only the matching/no-matching columns
     are saved as output. Note that the 'size' element is only for
     Gnuastro, it has no effect on later freeing of the array in the memory
     (we are not 'realloc'ing). */
  if(p->notmatched)
    {
      /* We want to move the non-matched rows after permutation
         to the top set of rows. But when we have strings, the
         strings that will be over-written need to be freed
         first. */
      numnotmatched = in->size - nummatched;
      if(in->type==GAL_TYPE_STRING)
        {
          strarr=in->array;
          for(i=0;i<nummatched;++i)
            if(strarr[i]) free(strarr[i]);
        }

      /* Move the non-matched elements up to the top. */
      memcpy(in->array,
             gal_pointer_increment(in->array, nummatched,
                                   in->type),
             numnotmatched*gal_type_sizeof(in->type));

      /* If we are on a string, the pointers at the bottom (that
         have been moved to the top), should not be set to NULL
         to avoid any potential double freeing. */
      if(in->type==GAL_TYPE_STRING)
        {
          strarr=in->array;
          for(i=numnotmatched; i<in->size;++i) strarr[i]=NULL;
        }

      /* Correct the size of the tile. */
      in->size = in->dsize[0] = numnotmatched;
    }

  /* This is a normal match (not not-match). */
  else
    {
      /* If we are on a string column, free the allocated space
         for each element that should be removed. */
      if(in->type==GAL_TYPE_STRING)
        {
          strarr=in->array;
          for(i=nummatched;i<in->size;++i)
            if(strarr[i]) { free(strarr[i]); strarr[i]=NULL; }
        }

      /* Correct the size. */
      in->size = in->dsize[0] = nummatched;
    }
}





static void
match_arrange_in_new_col(struct matchparams *p, gal_data_t *in,
                         size_t *permutation, size_t nummatched)
{
  size_t c=0, i;
  size_t istart=p->notmatched ? nummatched : 0;
  size_t iend=p->notmatched ? in->size : nummatched;
  size_t outsize=p->notmatched ? in->size - nummatched : nummatched;

  /* Allocate the array. */
  void *out=gal_pointer_allocate_ram_or_mmap(in->type, outsize, 0,
                                             p->cp.minmapsize,
                                             &in->mmapname, p->cp.quietmmap,
                                             __func__, "out");

  /* Copy the matched rows into the output array. */
  for(i=istart;i<iend;++i)
    memcpy(gal_pointer_increment(out, c++, in->type),
           gal_pointer_increment(in->array, permutation[i],
                                 in->type),
           gal_type_sizeof(in->type));

  /**********************************/
  /* Add a check so if the column is a string, we free the strings that
     aren't included in the output. */
  /**********************************/

  /* Free the existing array, and correct the sizes. */
  in->size = in->dsize[0] = outsize;
  free(in->array);
  in->array=out;
}





/* Parameters for parallelization of output creation. */
struct ma_params
{
  struct matchparams *p;        /* General program settings. */
  gal_data_t       *cat;        /* Dataset (all rows) to arrange. */
  size_t     nummatched;        /* Number of matched. */
  size_t   *permutation;        /* The permutation. */
};

static void *
match_arrange(void *in_prm)
{
  /* Low-level definitions to be done first. */
  struct gal_threads_params *tprm=(struct gal_threads_params *)in_prm;
  struct ma_params *map=(struct ma_params *)tprm->params;

  /* High-level variables. */
  gal_data_t *tmp;
  size_t c, i, index;

  /* go over all columns associated to this thread. */
  for(i=0; tprm->indexs[i] != GAL_BLANK_SIZE_T; ++i)
    {
      /* For easy reading. */
      index = tprm->indexs[i];

      /* Find this column in the whole table (linked list). */
      c=0;
      for(tmp=map->cat; tmp!=NULL; tmp=tmp->next) if(c++==index) break;

      /* Rearrange this columns' elements. */
      match_arrange_in_new_col(map->p, tmp, map->permutation,
                               map->nummatched);
    }

  /* Wait for all the other threads to finish, then return. */
  if(tprm->b) pthread_barrier_wait(tprm->b);
  return NULL;
}





/* Read the catalog in the given file and use the given permutation to keep
   the proper columns. */
static gal_data_t *
match_catalog_read_write_all(struct matchparams *p, size_t *permutation,
                             size_t nummatched, int f1s2,
                             size_t **numcolmatch)
{
  int hasall=0;
  struct ma_params map;
  gal_data_t *tmp, *cat;
  gal_list_str_t *cols, *tcol;

  char *hdu              = (f1s2==1) ? p->cp.hdu     : p->hdu2;
  gal_list_str_t *incols = (f1s2==1) ? p->acols      : p->bcols;
  size_t *numcols        = (f1s2==1) ? &p->anum      : &p->bnum;
  char *extname          = (f1s2==1) ? "INPUT_1"     : "INPUT_2";
  char *outname          = (f1s2==1) ? p->out1name   : p->out2name;
  char *filename         = (f1s2==1) ? p->input1name : p->input2name;

  /* If special columns are requested. */
  if(p->outcols)
    {
      /* As a special situation, the user can ask to incude all of the
         columns from one of the inputs with the special '_all' name. So,
         we'll check if that is the case and write in all the columns where
         they are requested.*/
      for(tcol=incols; tcol!=NULL; tcol=tcol->next)
        if(!strcmp(tcol->v,"_all")) { hasall=1; break; }

      /* If atleast one instance of '_all' is present, then reset the list
         of columns to include in output. */
      if(hasall)
        {
          cols=match_add_all_cols(filename, hdu, p->stdinlines, incols,
                                  numcols);
          if(f1s2==1) { gal_list_str_free(p->acols, 0); p->acols=cols; }
          else        { gal_list_str_free(p->bcols, 0); p->bcols=cols; }
        }
      else
        cols=incols;

      /* When the output contains columns from both inputs, we need to keep
         the number of columns matched against each column identifier. */
      *numcolmatch=gal_pointer_allocate(GAL_TYPE_SIZE_T,
                                        gal_list_str_number(cols), 1,
                                        __func__, "numcolmatch");
    }
  else cols=incols;


  /* Read the full table. NOTE that with '--coord', for the second input,
     both 'filename' and 'p->stdinlines' will be NULL. */
  if(filename || p->stdinlines)
    cat=gal_table_read(filename, hdu, filename ? NULL : p->stdinlines,
                       cols, p->cp.searchin, p->cp.ignorecase,
                       p->cp.numthreads, p->cp.minmapsize,
                       p->cp.quietmmap, *numcolmatch);
  else
    cat=match_cat_from_coord(p, cols, *numcolmatch);

  /* Arrange the output rows. */
  if(permutation)
    {
      /* When we are in no-match AND outcols mode, we don't need to touch
         the rows of the first input catalog (we want all of them) */
      if( (p->notmatched && p->outcols && f1s2==1) == 0 )
        {
          map.p=p;
          map.cat=cat;
          map.nummatched=nummatched;
          map.permutation=permutation;
          gal_threads_spin_off(match_arrange, &map,
                               gal_list_data_number(cat),
                               p->cp.numthreads, p->cp.minmapsize,
                               p->cp.quietmmap);

        }
    }

  /* If no match was found ('permutation==NULL'), and the matched columns
     are requested, empty all the columns that are to be written (only
     keeping the meta-data). */
  else
    if(p->notmatched==0)
      {
        for(tmp=cat; tmp!=NULL; tmp=tmp->next)
          {
            tmp->size=0;
            free(tmp->dsize); tmp->dsize=NULL;
            free(tmp->array); tmp->array=NULL;
          }
      }


  /* Write the catalog to the output. */
  if(p->outcols)
    return cat;
  else if(cat)
    {
      /* Write the catalog to a file. */
      gal_table_write(cat, NULL, NULL, p->cp.tableformat, outname,
                      extname, 0);

      /* Clean up. */
      gal_list_data_free(cat);
    }

  return NULL;
}





/* When merging is to be done by rows (the non-matched rows of the second
   catalog get merged into the first for the same columns). */
static void
match_catalog_write_one_row(struct matchparams *p, gal_data_t *a,
                            gal_data_t *b)
{
  char **strarr;
  gal_data_t *ta, *tb, *cat=NULL;
  size_t i, dsize=a->size+b->size;

  /* A small sanity check. */
  if( gal_list_data_number(a) != gal_list_data_number(b) )
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
          "fix it. The number of columns in the two catalogs are not "
          "equal (%zu and %zu respectively)", __func__,
          PACKAGE_BUGREPORT, gal_list_data_number(a),
          gal_list_data_number(b));

  /* Check if there is actually any row to add? */
  if(b->size>0)
    {
      /* Go over the columns of the first and make the final output columns
         with new sizes, but same types and metadata as the first input.*/
      tb=b;
      for(ta=a; ta!=NULL; ta=ta->next)
        {
          /* Make sure both have the same type. */
          if(ta->type!=tb->type)
            error(EXIT_FAILURE, 0, "when '--notmatched' and '--outcols' "
                  "are used together, the each column given to '--outcols' "
                  "must have the same datatype in both tables. However, "
                  "the first input has a type of '%s' for one of the "
                  "columns, while the second has a type of '%s'",
                  gal_type_name(ta->type, 1), gal_type_name(tb->type, 1));

          /* Allocate the necessary space. */
          gal_list_data_add_alloc(&cat, NULL, ta->type, ta->ndim,
                                  &dsize, NULL, 0, p->cp.minmapsize,
                                  p->cp.quietmmap, ta->name, ta->unit,
                                  ta->comment);

          /* Copy the data of the first and second inputs in output. */
          memcpy(cat->array, ta->array,
                 ta->size*gal_type_sizeof(ta->type));
          memcpy(gal_pointer_increment(cat->array, ta->size, cat->type),
                 tb->array, tb->size*gal_type_sizeof(tb->type));

          /* If we have a string column, the allocated spaces of each row
             should now only be freed within the 'cat' column, so set the
             values within 'a' and 'b' to NULL. */
          if(ta->type==GAL_TYPE_STRING)
            {
              strarr=ta->array; for(i=0;i<ta->size;++i) strarr[i]=NULL;
              strarr=tb->array; for(i=0;i<tb->size;++i) strarr[i]=NULL;
            }

          /* Increment 'tb'. */
          tb=tb->next;
        }

      /* Reverse the table and write it out. */
      gal_list_data_reverse(&cat);
      gal_table_write(cat, NULL, NULL, p->cp.tableformat,
                      p->out1name, "MATCHED", 0);
      gal_list_data_free(cat);
    }

  /* There wasn't any row to add, just write the 'a' columns and don't free
     it ('a' will be freed in the higher-level function). */
  else
    gal_table_write(a, NULL, NULL, p->cp.tableformat, p->out1name,
                    "MATCHED", 0);
}





/* When specific columns from both inputs are requested, this function
   will write them out into a single table. */
static void
match_catalog_write_one_col(struct matchparams *p, gal_data_t *a,
                            gal_data_t *b, size_t *acolmatch,
                            size_t *bcolmatch)
{
  gal_data_t *cat=NULL;
  char **strarr=p->outcols->array;
  size_t i, j, k, ac=0, bc=0, npop;

  /* Go over the initial list of strings. */
  for(i=0; i<p->outcols->size; ++i)
    switch(strarr[i][0])
      {
      case 'a':
        for(j=0;j<acolmatch[ac];++j)
          {
            npop = strcmp(strarr[i]+1,"_all") ? 1 : p->anum;
            for(k=0;k<npop;++k)
              gal_list_data_add(&cat, gal_list_data_pop(&a));
          }
        ac++;
        break;

      case 'b':
        for(j=0;j<bcolmatch[bc];++j)
          {
            npop = strcmp(strarr[i]+1,"_all") ? 1 : p->bnum;
            for(k=0;k<npop;++k)
              gal_list_data_add(&cat, gal_list_data_pop(&b));
          }
        bc++;
        break;

      default:
        error(EXIT_FAILURE, 0, "a bug! Please contact us at %s to "
              "fix the problem. the value to strarr[%zu][0] (%c) "
              "is not recognized", PACKAGE_BUGREPORT, i, strarr[i][0]);
      }

  /* A small sanity check. */
  if(a || b)
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us to fix "
          "the problem. The two 'a' and 'b' arrays must be NULL "
          "by this point: 'a' %s NULL, 'b' %s NULL", __func__,
          a?"is not":"is", b?"is not":"is");

  /* Reverse the table and write it out. */
  gal_list_data_reverse(&cat);
  gal_table_write(cat, NULL, NULL, p->cp.tableformat, p->out1name,
                  "MATCHED", 0);
  gal_list_data_free(cat);
}





static void
match_catalog_kdtree_build(struct matchparams *p)
{
  char *msg;
  size_t root;
  struct timeval t1;
  gal_data_t *kdtree;
  gal_fits_list_key_t *keylist=NULL;

  /* Meta-data in the output fits file. */
  char *unit = "index";
  char *comment = "k-d tree root index (counting from 0).";

  /* Construct a k-d tree from 'p->cols1': the index of root is stored in
     'root'. */
  if(!p->cp.quiet) gettimeofday(&t1, NULL);
  kdtree = gal_kdtree_create(p->cols1, &root);
  if(!p->cp.quiet)
    {
      if( asprintf(&msg, "k-d tree constructed (%zu rows).",
                   p->cols1->size)<0 )
        error(EXIT_FAILURE, errno, "asprintf allocation");
      gal_timing_report(&t1, msg, 1);
      free(msg);
    }

  /* Write the k-d tree to a file and write root index and input name
     as FITS keywords ('gal_table_write' frees 'keylist'). */
  gal_fits_key_list_title_add(&keylist, "k-d tree parameters", 0);
  gal_fits_key_write_filename("KDTIN", p->input1name, &keylist, 0,
                              p->cp.quiet);
  gal_fits_key_list_add_end(&keylist, GAL_TYPE_SIZE_T,
                            MATCH_KDTREE_ROOT_KEY, 0,
                            &root, 0, comment, 0, unit, 0);
  gal_table_write(kdtree, &keylist, NULL, GAL_TABLE_FORMAT_BFITS,
                  p->out1name, "kdtree", 0);

  /* Let the user know that the k-d tree has been built. */
  if(!p->cp.quiet)
    fprintf(stdout, "  - Output (k-d tree): %s\n", p->out1name);
}





/* Wrapper over the k-d tree library to return an output in the same format
   as 'gal_match_sort_based'. */
static gal_data_t *
match_catalog_kdtree(struct matchparams *p, size_t *nummatched)
{
  char *msg;
  struct timeval t1;
  gal_data_t *out=NULL;

  /* Operate according to the required mode. */
  switch(p->kdtreemode)
    {
    /* Build a k-d tree and don't continue. */
    case MATCH_KDTREE_BUILD:
      match_catalog_kdtree_build(p);
      break;

    /* Do the k-d tree matching. */
    case MATCH_KDTREE_FILE:
    case MATCH_KDTREE_INTERNAL:

      /* If the k-d tree should be constructed internally, build it,
         otherwise, we have already read an checked the k-d tree in 'ui.c',
         so go directly to the matching. */
      if(p->kdtreemode==MATCH_KDTREE_INTERNAL)
        {
          if(!p->cp.quiet) gettimeofday(&t1, NULL);
          p->kdtreedata = gal_kdtree_create(p->cols1, &p->kdtreeroot);
          if(!p->cp.quiet)
            gal_timing_report(&t1, "Internal k-d tree constructed.", 1);
        }

      /* Do k-d tree based match. */
      if(!p->cp.quiet)
        {
          gettimeofday(&t1, NULL);
          printf("  - Match using the k-d tree ...\n");
        }
      out = gal_match_kdtree(p->cols1, p->cols2, p->kdtreedata,
                             p->kdtreeroot, p->aperture->array,
                             p->cp.numthreads, p->cp.minmapsize,
                             p->cp.quietmmap, nummatched);
      if(!p->cp.quiet)
        {
          if( asprintf(&msg, "... %zu matches found, done!",
                       *nummatched)<0 )
            error(EXIT_FAILURE, errno, "asprintf allocation");
          gal_timing_report(&t1, msg, 1);
          free(msg);
        }
      gal_list_data_free(p->kdtreedata);
      break;

    /* Abort if the mode isn't recognized (its a bug!). */
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to fix "
            "the problem! The code %d isn't recognized for 'kdtreemode'",
            __func__, PACKAGE_BUGREPORT, p->kdtreemode);
    }

  /* Return the final match. */
  return out;
}





static gal_data_t *
match_catalog_sort_based(struct matchparams *p, size_t *nummatched)
{
  char *msg;
  gal_data_t *mcols;
  struct timeval t1;

  /* Let the user know that the matching has started. */
  if(!p->cp.quiet)
    {
      gettimeofday(&t1, NULL);
      printf("  - Matching by sorting ...\n");
    }

  /* Do the matching. */
  mcols=gal_match_sort_based(p->cols1, p->cols2, p->aperture->array,
                             0, 1, p->cp.minmapsize, p->cp.quietmmap,
                             nummatched);

  /* Let the user know that it finished. */
  if(!p->cp.quiet)
    {
      if( asprintf(&msg, "... %zu matches found, done!", *nummatched)<0 )
        error(EXIT_FAILURE, errno, "asprintf allocation");
      gal_timing_report(&t1, msg, 1);
      free(msg);
    }

  /* Return the permutations. */
  return mcols;
}





static void
match_catalog(struct matchparams *p)
{
  uint32_t *u, *uf;
  struct timeval t1;
  gal_data_t *tmp, *a=NULL, *b=NULL, *mcols=NULL;
  size_t nummatched, *acolmatch=NULL, *bcolmatch=NULL;

  /* If we want to use kd-tree for matching. */
  if(p->kdtreemode!=MATCH_KDTREE_DISABLE)
    {
      /* The main processing function. */
      mcols=match_catalog_kdtree(p, &nummatched);

      /* If the user just asked to build a k-d tree, no futher processing
         is necessary, so don't continue. */
      if(p->kdtreemode==MATCH_KDTREE_BUILD) return;
    }
  else
    mcols=match_catalog_sort_based(p, &nummatched);

  /* If the output is to be taken from the input columns (it isn't just the
     log), then do the job. */
  if(p->logasoutput==0)
    {
      /* Let the user know what is happening. */
      if(!p->cp.quiet)
        {
          gettimeofday(&t1, NULL);
          printf("  - Arranging matched rows (skip this with "
                 "'--logasoutput')...\n");
        }

      /* Read (and possibly write) the outputs. Note that we only need to
         read the table when it is necessary for the output (the user might
         have asked for '--outcols', only with columns of one of the two
         inputs). */
      if(p->outcols==NULL || p->acols)
        a=match_catalog_read_write_all(p, mcols?mcols->array:NULL,
                                       nummatched, 1, &acolmatch);
      if(p->outcols==NULL || p->bcols)
        b=match_catalog_read_write_all(p, mcols?mcols->next->array:NULL,
                                       nummatched, 2, &bcolmatch);

      /* If one catalog (with specific columns from either of the two
         inputs) was requested, then write it out. */
      if(p->outcols)
        {
          /* Arrange the columns and write the output. */
          if(p->notmatched)
            match_catalog_write_one_row(p, a, b);
          else
            {
              match_catalog_write_one_col(p, a, b, acolmatch, bcolmatch);
              a=b=NULL; /*They are freed in function above. */
            }

          /* Clean up. */
          if(acolmatch) free(acolmatch);
          if(bcolmatch) free(bcolmatch);
        }

      /* Clean up. */
      if(a) gal_list_data_free(a);
      if(b) gal_list_data_free(b);

      /* Let the user know. */
      if( !p->cp.quiet )
        gal_timing_report(&t1, "... done!", 1);
    }

  /* Write the raw information in a log file if necessary.  */
  if(p->logname && mcols)
    {
      /* Note that unsigned 64-bit integers are not recognized in FITS
         tables. So if the log file is a FITS table, covert the two
         index columns to uint32. */
      tmp=gal_data_copy_to_new_type(mcols, GAL_TYPE_UINT32);
      tmp->next=mcols->next;
      tmp->size=nummatched;
      gal_data_free(mcols);
      mcols=tmp;

      /* We also want everything to be incremented by one. In a C
         program, counting starts with zero, so 'gal_match_sort_based'
         will return indexs starting from zero. But outside a C
         program, on the command-line people expect counting to start
         from 1 (for example with AWK). */
      uf = (u=mcols->array) + tmp->size; do (*u)++; while(++u<uf);

      /* Same for the second set of indexs. */
      tmp=gal_data_copy_to_new_type(mcols->next, GAL_TYPE_UINT32);
      uf = (u=tmp->array) + tmp->size; do (*u)++; while(++u<uf);
      tmp->next=mcols->next->next;
      gal_data_free(mcols->next);
      tmp->size=nummatched;
      mcols->next=tmp;

      /* Correct the comments. */
      free(mcols->comment);
      mcols->comment="Row index in first catalog (counting from 1).";
      free(mcols->next->comment);
      mcols->next->comment="Row index in second catalog (counting "
        "from 1).";

      /* Write them into the table. */
      gal_table_write(mcols, NULL, NULL, p->cp.tableformat, p->logname,
                      "LOG_INFO", 0);

      /* Set the comment pointer to NULL: they weren't allocated. */
      mcols->comment=NULL;
      mcols->next->comment=NULL;
    }

  /* Clean up. */
  gal_list_data_free(mcols);

  /* Print the number of matches if not in quiet mode. */
  if(!p->cp.quiet)
    {
      if(p->out2name && strcmp(p->out1name, p->out2name))
        fprintf(stdout, "  - Output-1: %s\n  - Output-2: %s\n",
                p->out1name, p->out2name);
      else
        fprintf(stdout, "  - Output: %s\n", p->out1name);
    }
}




















/*******************************************************************/
/*************            Top level function           *************/
/*******************************************************************/
void
match(struct matchparams *p)
{
  /* Do the correct type of matching. */
  switch(p->mode)
    {
    case MATCH_MODE_CATALOG: match_catalog(p); break;
    case MATCH_MODE_WCS:
      error(EXIT_FAILURE, 0, "matching by WCS is not yet supported");
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! please contact us at %s to fix "
            "the problem: %d is not a recognized mode",
            __func__, PACKAGE_BUGREPORT, p->mode);
    }

  /* Write Match's configuration as keywords into the first extension of
     the output. */
  if(gal_fits_name_is_fits(p->out1name))
    {
      gal_fits_key_write_filename("input1", ( p->input1name
                                              ? p->input1name
                                              : "Standard input" ),
                                  &p->cp.okeys, 1, p->cp.quiet);
      gal_fits_key_write_filename("input2",
                                  p->input2name?p->input2name:"--coord",
                                  &p->cp.okeys, 1, p->cp.quiet);
      gal_fits_key_write_config(&p->cp.okeys, "Match configuration",
                                "MATCH-CONFIG", p->out1name, "0");
    }
}
