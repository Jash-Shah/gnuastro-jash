/*********************************************************************
Fits - View and manipulate FITS extensions and/or headers.
Fits is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2015-2022 Free Software Foundation, Inc.

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

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gnuastro/wcs.h>
#include <gnuastro/fits.h>
#include <gnuastro/pointer.h>
#include <gnuastro-internal/timing.h>

#include <gnuastro-internal/checkset.h>

#include "main.h"

#include "fits.h"








/***********************************************************************/
/******************           Preparations          ********************/
/***********************************************************************/
static void
keywords_open(struct fitsparams *p, fitsfile **fptr, int iomode)
{
  if(*fptr==NULL)
    *fptr=gal_fits_hdu_open(p->input->v, p->cp.hdu, iomode);
}




















/***********************************************************************/
/******************        File manipulation        ********************/
/***********************************************************************/
static void
keywords_rename_keys(struct fitsparams *p, fitsfile **fptr, int *r)
{
  int status=0;
  char *copy, *str, *from, *to;

  /* Set the FITS file pointer. */
  keywords_open(p, fptr, READWRITE);

  /* Tokenize the */
  while(p->rename!=NULL)
    {
      /* Pop out the top element. */
      str=gal_list_str_pop(&p->rename);

      /* Take a copy of the input string for error reporting, because
         'strtok' will write into the array. */
      gal_checkset_allocate_copy(str, &copy);

      /* Tokenize the input. */
      from = strtok(str,  ", ");
      to   = strtok(NULL, ", ");

      /* Make sure both elements were read. */
      if(from==NULL || to==NULL)
        error(EXIT_FAILURE, 0, "'%s' could not be tokenized in order to "
              "complete rename. There should be a space character "
              "or a comma (,) between the two keyword names. If you have "
              "used the space character, be sure to enclose the value to "
              "the '--rename' option in double quotation marks", copy);

      /* Rename the keyword */
      fits_modify_name(*fptr, from, to, &status);
      if(status) *r=fits_has_error(p, FITS_ACTION_RENAME, from, status);
      else p->updatechecksum=1;
      status=0;

      /* Clean up the user's input string. Note that 'strtok' just changes
         characters within the allocated string, no extra allocation is
         done. */
      free(str);
      free(copy);
    }
}





/* Special write options don't have any value and the value has to be found
   within the script. */
static int
keywords_write_special(struct fitsparams *p, fitsfile **fptr,
                       gal_fits_list_key_t *keyll)
{
  int status=0;

  if( !strcasecmp(keyll->keyname,"checksum")
      || !strcasecmp(keyll->keyname,"datasum") )
    {
      /* If a value is given, then just write what the user gave. */
      if( keyll->value )
        return 1;
      else
        {
          /* Calculate and write the 'CHECKSUM' and 'DATASUM' keywords. */
          if( fits_write_chksum(*fptr, &status) )
            gal_fits_io_error(status, NULL);

          /* If the user just wanted datasum, remove the checksum
             keyword. */
          if( !strcasecmp(keyll->keyname,"datasum") )
            if( fits_delete_key(*fptr, "CHECKSUM", &status) )
              gal_fits_io_error(status, NULL);

          /* Inform the caller that everything is done. */
          return 0;
        }
    }
  else if( keyll->keyname[0]=='/' )
    {
      gal_fits_key_write_title_in_ptr(keyll->value, *fptr);
      p->updatechecksum=1;
      return 0;
    }
  else
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
          "fix the problem. The 'keyname' value '%s' is not "
          "recognized as one with no value", __func__,
          PACKAGE_BUGREPORT, keyll->keyname);

  /* Function should not reach here. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to fix this "
        "problem. Control should not reach the end of this function",
        __func__, PACKAGE_BUGREPORT);
  return 0;
}





static void
keywords_write_update(struct fitsparams *p, fitsfile **fptr,
                      gal_fits_list_key_t *keyll, int u1w2)
{
  int status=0, continuewriting=0;
  gal_fits_list_key_t *tmp;

  /* Open the FITS file if it hasn't been opened yet. */
  keywords_open(p, fptr, READWRITE);

  /* Go through each key and write it in the FITS file. */
  while(keyll!=NULL)
    {
      /* Deal with special keywords. */
      continuewriting=1;
      if( keyll->value==NULL || keyll->keyname[0]=='/' )
        continuewriting=keywords_write_special(p, fptr, keyll);

      /* Write the information: */
      if(continuewriting)
        {
          if(u1w2==1)
            {
              if(keyll->value)
                {
                  if( fits_update_key(*fptr,
                                      gal_fits_type_to_datatype(keyll->type),
                                      keyll->keyname, keyll->value,
                                      keyll->comment, &status) )
                    gal_fits_io_error(status, NULL);
                }
              else
                {
                  if(fits_write_key_null(*fptr, keyll->keyname,
                                         keyll->comment, &status))
                    gal_fits_io_error(status, NULL);
                }
            }
          else if (u1w2==2)
            {
              if(keyll->value)
                {
                  if( fits_write_key(*fptr,
                                     gal_fits_type_to_datatype(keyll->type),
                                     keyll->keyname, keyll->value,
                                     keyll->comment, &status) )
                    gal_fits_io_error(status, NULL);
                }
              else
                {
                  if(fits_write_key_null(*fptr, keyll->keyname,
                                         keyll->comment, &status))
                    gal_fits_io_error(status, NULL);
                }
              if(keyll->unit
                 && fits_write_key_unit(*fptr, keyll->keyname, keyll->unit,
                                        &status) )
                gal_fits_io_error(status, NULL);
            }
          else
            error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' so "
                  "we can fix this problem. The value %d is not valid for "
                  "'u1w2'", __func__, PACKAGE_BUGREPORT, u1w2);

          /* Add the unit (if one was given). */
          if(keyll->unit
             && fits_write_key_unit(*fptr, keyll->keyname, keyll->unit,
                                    &status) )
            gal_fits_io_error(status, NULL);

          /* By this stage, a keyword has been written or updated. So its
             necessary to update the checksum in the end. This should be
             under the 'if(continuewriting)' conditional (because */
          p->updatechecksum=1;
        }

      /* Free the allocated spaces if necessary: */
      if(keyll->vfree) free(keyll->value);
      if(keyll->kfree) free(keyll->keyname);
      if(keyll->cfree) free(keyll->comment);

      /* Keep the pointer to the next keyword and free the allocated
         space for this keyword.*/
      tmp=keyll->next;
      free(keyll);
      keyll=tmp;
    }
}





static void
keywords_print_all_keys(struct fitsparams *p, fitsfile **fptr)
{
  size_t i=0;
  int nkeys, status=0;
  char *fullheader, *c, *cf;

  /* Convert the header into a contiguous string. */
  if( fits_hdr2str(*fptr, 0, NULL, 0, &fullheader, &nkeys, &status) )
    gal_fits_io_error(status, NULL);

  /* FLEN_CARD supposes that the NULL string character is in the
     end of each keyword header card. In fits_hdr2str, the NULL
     characters are removed and so the maximum length is one
     less. */
  cf=(c=fullheader)+nkeys*(FLEN_CARD-1);
  do
    {
      if(i && i%(FLEN_CARD-1)==0)
        putc('\n', stdout);
      putc(*c++, stdout);
      ++i;
    }
  while(c<cf);
  printf("\n");

  if (fits_free_memory(fullheader, &status) )
    gal_fits_io_error(status, "problem in header.c for freeing "
                      "the memory used to keep all the headers");
}





static void
keywords_list_key_names(struct fitsparams *p, fitsfile *fptr)
{
  size_t i=0;
  int status=0;
  char keyname[FLEN_CARD], value[FLEN_CARD], *comment=NULL;

  /* Initialize 'keyname' so the first one (that is blank) isn't
     printed. */
  keyname[0]='\0';

  /* Go through all the keywords until you reach 'END'. */
  while( strcmp(keyname, "END") )
      {
        /* Print the most recent keyword: this is placed before reading the
           keyword because we want to stop upon reading 'END'. */
        if( strlen(keyname) ) printf("%s\n", keyname);

        /* Read the next keyword. */
        fits_read_keyn(fptr, i++, keyname, value, comment, &status);
      }
}





static int
keywords_verify(struct fitsparams *p, fitsfile **fptr)
{
  int dataok, hduok, status=0;

  /* Ask CFITSIO to verify the two keywords. */
  if( fits_verify_chksum(*fptr, &dataok, &hduok, &status) )
    gal_fits_io_error(status, NULL);

  /* Print some introduction: */
  if(!p->cp.quiet)
    printf("%s\n"
           "Checking integrity of %s (hdu %s)\n"
           "%s"
           "--------\n"
           "Basic info (remove all extra info with '--quiet'):\n"
           "    - DATASUM: verifies only the data (not keywords).\n"
           "    - CHECKSUM: verifies data and keywords.\n"
           "They can be added-to/updated-in an extension/HDU with:\n"
           "    $ astfits %s -h%s --write=checksum\n"
           "--------\n", PROGRAM_STRING, p->input->v, p->cp.hdu,
           ctime(&p->rawtime), p->input->v, p->cp.hdu);

  /* Print the verification result. */
  printf("DATASUM:  %s\n",
         dataok==1 ? "Verified" : (dataok==0 ? "NOT-PRESENT" : "INCORRECT"));
  printf("CHECKSUM: %s\n",
         hduok==1  ? "Verified" : (hduok==0  ? "NOT-PRESENT" : "INCORRECT"));

  /* Return failure if any of the keywords are not verified. */
  return (dataok==-1 || hduok==-1) ? EXIT_FAILURE : EXIT_SUCCESS;
}





/* To copy keys to output file when '--copykeys' is given in 'STR,STR,STR'
   format. */
static void
keywords_copykeys_name(struct fitsparams *p, fitsfile *fptr,
                       char *inkeys, size_t numinkeys,
                       int *updatechecksum)
{
  size_t i, j;
  int status=0, found;
  char **strarr=p->copykeysname->array;

  /* Copy the requested headers into the output files header. */
  for(i=0; i<p->copykeysname->size; ++i)
    {
      /* Find the requested key from the set of input keywords that were
         read before. The FITS standard does specify that the keyword
         should only have upper-case letters, but CFITSIO does accept small
         case (and write upper-case), so we'll also ignore case when
         comparing.*/
      found=0;
      for(j=0;j<numinkeys-1;++j) /* numkeys-1: it includes 'END'. */
        {
          /* First check the first few characters, if they don't match, see
             if it stars with 'HIERARCH' (which is placed before long
             keyword names). */
          if( !strncasecmp(strarr[i], &inkeys[j*80], strlen(strarr[i])) )
            found=1;
          else
            {
              if( !strncasecmp("HIERARCH", &inkeys[j*80], 8)
                  && !strncasecmp(strarr[i], &inkeys[j*80+9],
                                  strlen(strarr[i])) )
                found=1;
            }

          /* Write the keyword if it was found. */
          if(found==1)
            {
              if(fits_write_record(fptr, &inkeys[j*80], &status))
                gal_fits_io_error(status, NULL);
              else { *updatechecksum=1; break; }
            }
          }

      /* If the key wasn't found after a full parsing of the keywords,
         print an error message. */
      if(found==0)
        error(EXIT_FAILURE, 0, "%s: no keyword with name '%s' found",
              gal_fits_name_save_as_string(p->input->v, p->cp.hdu),
              strarr[i]);
    }
}





/* To copy keys to output file when --copykeys are given in INT:INT
    format. */
static void
keywords_copykeys_range(struct fitsparams *p, fitsfile *fptr,
                        char *inkeys, size_t numinkeys,
                        int *updatechecksum)
{
  size_t i;
  long initial;
  int status=0;

  /* Initial sanity check. Since 'numinkeys' includes 'END' (counting from
     1, as we do here), the first keyword must not be larger OR EQUAL to
     'numinkeys'. */
  if(p->copykeysrange[0]>=numinkeys)
    error(EXIT_FAILURE, 0, "%s (hdu %s): first keyword number give "
          "to '--copykeys' (%ld) is larger than the number of "
          "keywords in this header (%zu, including the 'END' "
          "keyword)", p->input->v, p->cp.hdu, p->copykeysrange[0],
          numinkeys);

  /* If the user wanted to count from the end (by giving a negative value),
     then do that. */
  if(p->copykeysrange[1]<0)
    {
      /* Set the last keyword requested. */
      initial=p->copykeysrange[1];
      p->copykeysrange[1] += numinkeys;

      /* Sanity check. */
      if(p->copykeysrange[0]>=p->copykeysrange[1])
        error(EXIT_FAILURE, 0, "%s (hdu %s): the last keyword given to "
              "'--copykeys' (%ld, or %ld after counting from the bottom) "
              "is earlier than the first (%ld)", p->input->v, p->cp.hdu,
              initial, p->copykeysrange[1], p->copykeysrange[0]);
    }

  /* Final sanity check (on range limit). */
  if(p->copykeysrange[1]>=numinkeys)
    error(EXIT_FAILURE, 0, "%s (hdu %s): second keyword number give to "
          "'--copykeys' (%ld) is larger than the number of keywords in "
          "this header (%zu, including the 'END' keyword)", p->input->v,
          p->cp.hdu, p->copykeysrange[1], numinkeys);


  /* Copy the requested headers into the output. */
  for(i=p->copykeysrange[0]-1; i<=p->copykeysrange[1]-1; ++i)
    {
      if( fits_write_record(fptr, &inkeys[i*80], &status ) )
        gal_fits_io_error(status, NULL);
      else *updatechecksum=1;
    }
}





static void
keywords_copykeys(struct fitsparams *p, char *inkeys, size_t numinkeys)
{
  int status=0;
  fitsfile *fptr;
  int updatechecksum=0, checksumexists=0;

  /* Open the output HDU. */
  fptr=gal_fits_hdu_open(p->cp.output, p->outhdu, READWRITE);

  /* See if a 'CHECKSUM' key exists in the HDU or not (to update in case we
     wrote anything). */
  checksumexists=gal_fits_key_exists_fptr(fptr, "CHECKSUM");

  /* Call different functions depending on whether a list or range of
      keywords is given. */
  if(p->copykeysname)
    keywords_copykeys_name(p, fptr, inkeys, numinkeys, &updatechecksum);
  else
    keywords_copykeys_range(p, fptr, inkeys, numinkeys, &updatechecksum);

  /* If a checksum existed, and we made changes in the file, we should
     update the checksum. */
  if(checksumexists && updatechecksum)
    if( fits_write_chksum(fptr, &status) )
      gal_fits_io_error(status, NULL);

  /* Close the output FITS file. */
  if(fits_close_file(fptr, &status))
    gal_fits_io_error(status, NULL);
}





static void
keywords_date_to_seconds(struct fitsparams *p, fitsfile *fptr)
{
  int status=0;
  double subsec;
  size_t seconds;
  char *subsecstr=NULL;
  char fitsdate[FLEN_KEYWORD];

  /* Read the requested FITS keyword. */
  if( fits_read_key(fptr, TSTRING, p->datetosec, &fitsdate, NULL, &status) )
    gal_fits_io_error(status, NULL);

  /* Return the number of seconds (and subseconds).*/
  seconds=gal_fits_key_date_to_seconds(fitsdate, &subsecstr, &subsec);
  if(seconds==GAL_BLANK_SIZE_T)
    error(EXIT_FAILURE, 0, "the time string couldn't be interpretted");

  /* Print the result (for the sub-seconds, print everything after the */
  if( !p->cp.quiet )
    {
      printf("%s (hdu %s), key '%s': %s\n", p->input->v, p->cp.hdu,
             p->datetosec, fitsdate);
      printf("Seconds since 1970/01/01 (00:00:00): %zu%s\n\n", seconds,
             subsecstr?subsecstr:"");
      printf("(To suppress verbose output, run with '-q')\n");
    }
  else
    printf("%zu%s\n", seconds, subsecstr?subsecstr:"");

  /* Clean up. */
  if(subsecstr) free(subsecstr);
}





static void
keywords_wcs_convert(struct fitsparams *p)
{
  int nwcs;
  size_t ndim, *insize;
  char *suffix, *output;
  gal_data_t *data=NULL;
  struct wcsprm *inwcs, *outwcs=NULL;
  size_t *dsize, defaultsize[2]={2000,2000};

  /* If the extension has any data, read it, otherwise just make an empty
     array. */
  if(gal_fits_hdu_format(p->input->v, p->cp.hdu)==IMAGE_HDU)
    {
      /* Read the size of the dataset (we don't need the actual size!). */
      insize=gal_fits_img_info_dim(p->input->v, p->cp.hdu, &ndim);
      free(insize);

      /* If the number of dimensions is two, then read the dataset,
         otherwise, ignore it. */
      if(ndim==2)
        data=gal_fits_img_read(p->input->v, p->cp.hdu, p->cp.minmapsize,
                               p->cp.quietmmap);
    }

  /* Read the input's WCS and make sure one exists. */
  inwcs=gal_wcs_read(p->input->v, p->cp.hdu, p->cp.wcslinearmatrix,
                     0, 0, &nwcs);
  if(inwcs==NULL)
    error(EXIT_FAILURE, 0, "%s (hdu %s): doesn't have any WCS structure "
          "for converting its coordinate system or distortion",
          p->input->v, p->cp.hdu);

  /* In case there is no dataset and the conversion is between TPV to SIP,
     we need to set a default size and use that for the conversion, but we
     also need to warn the user. */
  if(p->wcsdistortion && data==NULL)
    {
      if( !p->cp.quiet
          && gal_wcs_distortion_identify(inwcs)==GAL_WCS_DISTORTION_TPV
          && p->distortionid==GAL_WCS_DISTORTION_SIP )
        error(0, 0, "no data associated with WCS for distortion "
              "conversion.\n\n"
              "The requested conversion can't be done analytically, so a "
              "solution has to be found by fitting the parameters over a "
              "grid of pixels. We will use a default grid of %zux%zu pixels "
              "and will proceed with the conversion. But it would be more "
              "accurate if it is the size of the image that this WCS is "
              "associated with",
              defaultsize[1], defaultsize[0]);
      dsize=defaultsize;
    }
  else dsize=data->dsize;

  /* Do the conversion. */
  if(p->wcscoordsys)
    outwcs=gal_wcs_coordsys_convert(inwcs, p->coordsysid);
  else if(p->wcsdistortion)
    outwcs=gal_wcs_distortion_convert(inwcs, p->distortionid, dsize);
  else
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to fix "
          "the problem. The requested mode for this function is not "
          "recognized", __func__, PACKAGE_BUGREPORT);

  /* Set the output filename. */
  if(p->cp.output)
    output=p->cp.output;
  else
    {
      if( asprintf(&suffix, "-%s.fits",
                   p->wcsdistortion ? p->wcsdistortion : p->wcscoordsys)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
      output=gal_checkset_automatic_output(&p->cp, p->input->v, suffix);
    }
  gal_checkset_writable_remove(output, 0, p->cp.dontdelete);

  /* Write the output file. */
  if(data)
    {
      /* Add the output WCS to the dataset and write it. */
      data->wcs=outwcs;
      gal_fits_img_write(data, output, NULL, PROGRAM_NAME);

      /* Clean up, but remove the pointer first (so it doesn't free it
         here). */
      data->wcs=NULL;
      gal_data_free(data);
    }
  else
    gal_wcs_write(outwcs, output, p->wcsdistortion, NULL, PROGRAM_NAME);

  /* Clean up. */
  wcsfree(inwcs);
  wcsfree(outwcs);
  if(output!=p->cp.output) free(output);
}





static void
keywords_value_in_output_copy(gal_data_t *write, gal_data_t *key,
                              size_t in_counter)
{
  char **strarrk, **strarrw;

  /* Small sanity check. */
  if(write->type != key->type)
    error(EXIT_FAILURE, 0, "%s: the input datasets must have "
          "the same data type. The 'write' and 'key' arguments "
          "are respectively '%s' and '%s'", __func__,
          gal_type_name(write->type, 1),
          gal_type_name(key->type, 1));

  /* Copy the value. */
  if(key->type==GAL_TYPE_STRING)
    {
      strarrk=key->array;
      strarrw=write->array;
      strarrw[ in_counter ] = strarrk[0];
      strarrk[0]=NULL;
    }
  else
    memcpy(gal_pointer_increment(write->array, in_counter,
                                 write->type),
           key->array, gal_type_sizeof(write->type));
}





/* Write the value in the first row. The first row is unique here: if there
   is only one input dataset, the dataset name will not be in the
   output. But when there is more than one dataset, we include a column for
   the name of the dataset. */
static gal_data_t *
keywords_value_in_output_first(struct fitsparams *p, gal_data_t *topout,
                               char *filename, gal_data_t *keysll,
                               size_t ninput)
{
  char **strarr;
  gal_data_t *out=NULL;
  gal_data_t *write, *key;
  size_t in_counter=0; /* This function is only for the first row. */

  /* If a name column is necessary. */
  if(topout)
    {
      /* Small sanity check. */
      if(topout->next)
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
              "fix the problem. The 'next' pointer of 'topout' should "
              "be NULL", __func__, PACKAGE_BUGREPORT);

      /* The size of the output should be the same as 'ninput'. */
      if(topout->size!=ninput)
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
              "fix the problem. The number of elements in 'topout' "
              "(%zu) is different from 'ninput' (%zu)", __func__,
              PACKAGE_BUGREPORT, out->size, ninput);

      /* Write the filename. */
      strarr=topout->array;
      gal_checkset_allocate_copy(filename, &strarr[in_counter]);
    }

  /* Add the new columns into the raw output (only keyword values). */
  for(key=keysll; key!=NULL; key=key->next)
    {
      /* If the keyword couldn't be read for any reason then 'key->status'
         will be non-zero. In this case, return a string type and put a
         blank string value. */
      if( key->status )
        {
          key->type=GAL_TYPE_STRING;
          if(p->cp.quiet==0)
            error(EXIT_SUCCESS, 0, "%s (hdu %s): does not contain a "
                  "keyword '%s'", filename, p->cp.hdu, key->name);
        }

      /* Allocate the full column for this key and add it to the end of
         the existing output list of columns: IMPORTANT NOTE: it is
         necessary to initialize the values because we may need to
         change the types before fully writing values within it. */
      write=gal_data_alloc(NULL, key->type, 1, &ninput, NULL,
                           1, p->cp.minmapsize, p->cp.quietmmap,
                           key->name, key->unit, key->comment);

      /* Copy the value of this key into the output. Note that for strings,
         the arrays are initialized to NULL. */
      if( key->status )
        {
          strarr=write->array;
          gal_checkset_allocate_copy(GAL_BLANK_STRING,
                                     &strarr[in_counter]);
        }
      else
        keywords_value_in_output_copy(write, key, in_counter);

      /* Put the allocated column into the output list. */
      gal_list_data_add(&out, write);
    }

  /* Reverse the list (to be the same order as the user's request). */
  gal_list_data_reverse(&out);

  /* If a first row (containing the filename) is given, then add the
     allocated datasets to its end */
  if(topout) { topout->next=out; out=topout; }

  /* Return the output. */
  return out;
}





static void
keywords_value_in_output_rest_replace(gal_data_t *list, gal_data_t *old,
                                      gal_data_t *new)
{
  gal_data_t *parse;
  new->next=old->next;
  for(parse=list; parse!=NULL; parse=parse->next)
    if(parse->next==old)
      {
        gal_data_free(old);
        parse->next=new;
        break;
      }
}





/* This function is for the case that we have more than one row. In this
   case, we always want the input file's name to be printed. */
static void
keywords_value_in_output_rest(struct fitsparams *p, gal_data_t *out,
                              char *filename, gal_data_t *keysll,
                              size_t in_counter)
{
  int goodtype;
  char **strarr;
  gal_data_t *write, *key;
  gal_data_t *goodkey, *goodwrite;

  /* Write the file name in the first column. */
  strarr=out->array;
  gal_checkset_allocate_copy(filename, &strarr[in_counter]);

  /* Go over all the keys are write them in. */
  write=out;
  for(key=keysll; key!=NULL; key=key->next)
    {
      /* Increment the write column also. */
      write=write->next;

      /* If the status is non-zero then the keyword couldn't be read. In
         this case, put a blank value in this row. */
      if(key->status)
        {
          gal_blank_write(gal_pointer_increment(write->array, in_counter,
                                                write->type),
                          write->type);
          if(p->cp.quiet==0)
            error(EXIT_SUCCESS, 0, "%s (hdu %s): does not contain a "
                  "keyword '%s'", filename, p->cp.hdu, key->name);
          continue;
        }

      /* This key is good and the type is string (which is the type for a
         key that doesn't exist in the previous file(s)). In this case,
         check if all the previous rows are blank. If they are all blank
         then this keyword didn't exist in any of the previous files and
         this is the first one that has the keyword. So change the type of
         the column to the final type. */
      else
        {
          if( write->type==GAL_TYPE_STRING
              && write->type!=key->type
              && gal_blank_number(write, 1)==write->size )
            {
              goodwrite=gal_data_alloc(NULL, key->type, 1, out->dsize,
                                       NULL, 0, p->cp.minmapsize,
                                       p->cp.quietmmap, key->name,
                                       key->unit, key->comment);
              gal_blank_initialize(goodwrite);
              keywords_value_in_output_rest_replace(out, write,
                                                    goodwrite);
              write=goodwrite;
            }
        }

      /* If the previous files didn't have metadata for this keyword but
         this file does, use the metadata here. */
      if(write->unit==NULL && key->unit)
        { write->unit=key->unit; key->unit=NULL; }
      if(write->comment==NULL && key->comment)
        { write->comment=key->comment; key->comment=NULL; }

      /* If the column types are the same, then put them in. */
      if(key->type==write->type)
        keywords_value_in_output_copy(write, key, in_counter);
      else
        {
          /* Find the most inclusive type. */
          goodtype=gal_type_out(key->type, write->type);

          /* Convert each of the two into the same type. */
          goodkey = ( key->type==goodtype
                      ? key
                      : gal_data_copy_to_new_type(key, goodtype) );
          goodwrite = ( write->type==goodtype
                        ? write
                        : gal_data_copy_to_new_type(write, goodtype) );

          /* Copy the row into the output. */
          keywords_value_in_output_copy(goodwrite, goodkey, in_counter);

          /* If the "good" writing dataset has been changed, then
             replace it in the output (correct its 'next' pointer, and
             set the previous column to point to it. */
          if(goodwrite!=write)
            {
              keywords_value_in_output_rest_replace(out, write,
                                                    goodwrite);
              write=goodwrite;
            }

          /* If a different key has been used, clean it. */
          if(goodkey!=key) gal_data_free(goodkey);
        }
    }
}





static void
keywords_value(struct fitsparams *p)
{
  int status;
  fitsfile *fptr=NULL;
  gal_list_str_t *input, *tmp;
  size_t i, ii=0, ninput, nkeys;
  gal_data_t *out=NULL, *keysll=NULL;

  /* Count how many inputs there are, and allocate the first column with
     the name. */
  ninput=gal_list_str_number(p->input);
  if(ninput>1 || p->cp.quiet==0)
    out=gal_data_alloc(NULL, GAL_TYPE_STRING, 1, &ninput, NULL, 0,
                       p->cp.minmapsize, p->cp.quietmmap, "FILENAME",
                       "name", "Name of input file.");

  /* Allocate the structure to host the desired keywords read from each
     FITS file and their values. But first convert the list of strings (for
     keyword names), (where each string can be a comma-separated list) into
     a list with a single value per string. */
  gal_options_merge_list_of_csv(&p->keyvalue);
  nkeys=gal_list_str_number(p->keyvalue);

  /* Parse each input file, read the keywords and put them in the output
     list. */
  for(input=p->input; input!=NULL; input=input->next)
    {
      /* Open the input FITS file. */
      fptr=gal_fits_hdu_open(input->v, p->cp.hdu, READONLY);

      /* Allocate the array to keep the keys. */
      i=0;
      keysll=gal_data_array_calloc(nkeys);
      for(tmp=p->keyvalue; tmp!=NULL; tmp=tmp->next)
        {
          if(tmp->next) keysll[i].next=&keysll[i+1];
          keysll[i].name=tmp->v;
          ++i;
        }

      /* Read the keys. Note that we only need the comments and units if
         '--colinfoinstdout' is called. */
      gal_fits_key_read_from_ptr(fptr, keysll, p->colinfoinstdout,
                                 p->colinfoinstdout);

      /* Close the input FITS file. */
      status=0;
      if(fits_close_file(fptr, &status))
        gal_fits_io_error(status, NULL);

      /* Write the values of this column into the final output. */
      if(ii==0)
        {
          ++ii;
          out=keywords_value_in_output_first(p, out, input->v,
                                             keysll, ninput);
        }
      else
        keywords_value_in_output_rest(p, out, input->v, keysll,
                                      ii++);

      /* Clean up. */
      for(i=0;i<nkeys;++i) keysll[i].name=NULL;
      gal_data_array_free(keysll, nkeys, 1);
    }

  /* Write the values. */
  gal_checkset_writable_remove(p->cp.output, 0, p->cp.dontdelete);
  gal_table_write(out, NULL, NULL, p->cp.tableformat,
                  p->cp.output, "KEY-VALUES", p->colinfoinstdout);

  /* Clean up. */
  gal_list_str_free(p->keyvalue, 0);
}




















/***********************************************************************/
/******************           Main function         ********************/
/***********************************************************************/
/* NOTE ON CALLING keywords_open FOR EACH OPERATION:

   'keywords_open' is being called individually for each separate operation
   because the necessary permissions differ: when the user only wants to
   read keywords, they don't necessarily need write permissions. So if they
   haven't asked for any writing/editing operation, we shouldn't open in
   write-mode. Because the user might not have the permissions to write and
   they might not want to write. 'keywords_open' will only open the file
   once (if the pointer is already allocated, it won't do anything). */
int
keywords(struct fitsparams *p)
{
  char *inkeys=NULL;
  fitsfile *fptr=NULL;
  gal_list_str_t *tstll;
  int status=0, numinkeys;
  int r=EXIT_SUCCESS, checksumexists=0;

  /* Print the requested keywords. Note that this option isn't called with
     the rest. It is independent of them. */
  if(p->keyvalue)
    keywords_value(p);


  /* Delete the requested keywords. */
  if(p->delete)
    {
      /* Open the FITS file. */
      keywords_open(p, &fptr, READWRITE);

      /* Go over all the keywords to delete. */
      for(tstll=p->delete; tstll!=NULL; tstll=tstll->next)
        {
          fits_delete_key(fptr, tstll->v, &status);
          if(status)
            r=fits_has_error(p, FITS_ACTION_DELETE, tstll->v, status);
          else
            p->updatechecksum=1;
          status=0;
        }
    }


  /* If the checksum keyword still exists in the HDU (wasn't deleted in the
     previous step), then activate the flag to recalculate it at the
     end. Note that the distortion or coordinate system functions will do
     this job separately themselves. */
  if(p->rename || p->update || p->write || p->asis || p->history
     || p->comment || p->date)
    {
      keywords_open(p, &fptr, READWRITE);
      checksumexists=gal_fits_key_exists_fptr(fptr, "CHECKSUM");
    }


  /* Rename the requested keywords. */
  if(p->rename)
    {
      keywords_open(p, &fptr, READWRITE);
      keywords_rename_keys(p, &fptr, &r);
    }


  /* Update the requested keywords. */
  if(p->update)
    {
      keywords_open(p, &fptr, READWRITE);
      keywords_write_update(p, &fptr, p->update_keys, 1);
    }


  /* Write the requested keywords. */
  if(p->write)
    {
      keywords_open(p, &fptr, READWRITE);
      keywords_write_update(p, &fptr, p->write_keys, 2);
    }


  /* Put in any full line of keywords as-is. */
  if(p->asis)
    {
      keywords_open(p, &fptr, READWRITE);
      for(tstll=p->asis; tstll!=NULL; tstll=tstll->next)
        {
          fits_write_record(fptr, tstll->v, &status);
          if(status) r=fits_has_error(p, FITS_ACTION_WRITE, tstll->v, status);
          else p->updatechecksum=1;
          status=0;
        }
    }


  /* Add the history keyword(s). */
  if(p->history)
    {
      keywords_open(p, &fptr, READWRITE);
      for(tstll=p->history; tstll!=NULL; tstll=tstll->next)
        {
          fits_write_history(fptr, tstll->v, &status);
          if(status)
            r=fits_has_error(p, FITS_ACTION_WRITE, "HISTORY", status);
          else p->updatechecksum=1;
          status=0;
        }
    }


  /* Add comment(s). */
  if(p->comment)
    {
      keywords_open(p, &fptr, READWRITE);
      for(tstll=p->comment; tstll!=NULL; tstll=tstll->next)
        {
          fits_write_comment(fptr, tstll->v, &status);
          if(status)
            r=fits_has_error(p, FITS_ACTION_WRITE, "COMMENT", status);
          else p->updatechecksum=1;
          status=0;
        }
    }


  /* Update/add the date. */
  if(p->date)
    {
      keywords_open(p, &fptr, READWRITE);
      fits_write_date(fptr, &status);
      if(status) r=fits_has_error(p, FITS_ACTION_WRITE, "DATE", status);
      else p->updatechecksum=1;
      status=0;
    }


  /* Update the checksum (if necessary). */
  if(checksumexists && p->updatechecksum)
    if( fits_write_chksum(fptr, &status) )
      gal_fits_io_error(status, NULL);


  /* Print all the keywords in the extension. */
  if(p->printallkeys)
    {
      keywords_open(p, &fptr, READONLY);
      keywords_print_all_keys(p, &fptr);
    }


  /* Verify the CHECKSUM and DATASUM keys. */
  if(p->verify)
    {
      keywords_open(p, &fptr, READONLY);
      r=keywords_verify(p, &fptr);
    }


  /* If a list/range of keywords must be copied, get all the keywords as a
     single string. */
  if(p->copykeys)
    {
      keywords_open(p, &fptr, READONLY);
      if( fits_convert_hdr2str(fptr, 0, NULL, 0, &inkeys, &numinkeys,
                               &status) )
        gal_fits_io_error(status, NULL);
      status=0;
    }


  /* Convert the FITS date string into seconds. */
  if(p->datetosec)
    {
      keywords_open(p, &fptr, READONLY);
      keywords_date_to_seconds(p, fptr);
    }


  /* List all keyword names. */
  if(p->printkeynames)
    {
      keywords_open(p, &fptr, READONLY);
      keywords_list_key_names(p, fptr);
    }


  /* Close the FITS file */
  if(fptr && fits_close_file(fptr, &status))
    gal_fits_io_error(status, NULL);


  /* Write desired keywords into output. */
  if(p->copykeys)
    {
      keywords_copykeys(p, inkeys, numinkeys);
      free(inkeys);
    }


  /* Convert the input's distortion to the desired output distortion. */
  if(p->wcsdistortion || p->wcscoordsys)
    keywords_wcs_convert(p);

  /* Return. */
  return r;
}
