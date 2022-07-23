/*********************************************************************
ConvertType - Convert between various types of files.
ConvertType is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2016-2022 Free Software Foundation, Inc.

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

#include <argp.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <gnuastro/eps.h>
#include <gnuastro/txt.h>
#include <gnuastro/wcs.h>
#include <gnuastro/pdf.h>
#include <gnuastro/list.h>
#include <gnuastro/fits.h>
#include <gnuastro/jpeg.h>
#include <gnuastro/tiff.h>
#include <gnuastro/table.h>
#include <gnuastro/blank.h>
#include <gnuastro/color.h>
#include <gnuastro/arithmetic.h>

#include <gnuastro-internal/timing.h>
#include <gnuastro-internal/options.h>
#include <gnuastro-internal/checkset.h>
#include <gnuastro-internal/fixedstringmacros.h>

#include "main.h"

#include "ui.h"
#include "authors-cite.h"





/**************************************************************/
/*********      Argp necessary global entities     ************/
/**************************************************************/
/* Definition parameters for the Argp: */
const char *
argp_program_version = PROGRAM_STRING "\n"
                       GAL_STRINGS_COPYRIGHT
                       "\n\nWritten/developed by "PROGRAM_AUTHORS;

const char *
argp_program_bug_address = PACKAGE_BUGREPORT;

static char
args_doc[] = "InputFile1 [InputFile2] ... [InputFile4]";

const char
doc[] = GAL_STRINGS_TOP_HELP_INFO PROGRAM_NAME" will convert any of the "
  "known input formats to any other of the known formats. The output file "
  "will have the same number of pixels.\n"
  GAL_STRINGS_MORE_HELP_INFO
  /* After the list of options: */
  "\v"
  PACKAGE_NAME" home page: "PACKAGE_URL;




















/**************************************************************/
/*********    Initialize & Parse command-line    **************/
/**************************************************************/
static void
ui_initialize_options(struct converttparams *p,
                      struct argp_option *program_options,
                      struct argp_option *gal_commonopts_options)
{
  size_t i;
  struct gal_options_common_params *cp=&p->cp;


  /* Set the necessary common parameters structure. */
  cp->program_struct     = p;
  cp->poptions           = program_options;
  cp->program_name       = PROGRAM_NAME;
  cp->program_exec       = PROGRAM_EXEC;
  cp->program_bibtex     = PROGRAM_BIBTEX;
  cp->program_authors    = PROGRAM_AUTHORS;
  cp->coptions           = gal_commonopts_options;

  /* Program specific non-zero values. */
  p->maxbyte             = UINT8_MAX;
  p->quality             = GAL_BLANK_UINT8;

  /* Modify the common options. */
  for(i=0; !gal_options_is_last(&cp->coptions[i]); ++i)
    {
      /* Select individually */
      switch(cp->coptions[i].key)
        {
        case GAL_OPTIONS_KEY_HDU:
          cp->coptions[i].value=&p->hdus;
          cp->coptions[i].type=GAL_TYPE_STRLL;
          cp->coptions[i].doc="FITS input HDU, multiple calls possible.";
          break;

        case GAL_OPTIONS_KEY_OUTPUT:
          cp->coptions[i].mandatory=GAL_OPTIONS_MANDATORY;
          cp->coptions[i].doc="Output filename or suffix.";
          break;

        case GAL_OPTIONS_KEY_MINMAPSIZE:
          cp->coptions[i].mandatory=GAL_OPTIONS_MANDATORY;
          break;

        case GAL_OPTIONS_KEY_TYPE:
        case GAL_OPTIONS_KEY_TABLEFORMAT:
          cp->coptions[i].flags=OPTION_HIDDEN;
          break;
        }

      /* Select by group. */
      switch(cp->coptions[i].group)
        {
        case GAL_OPTIONS_GROUP_TESSELLATION:
          cp->coptions[i].doc=NULL; /* Necessary to remove title. */
          cp->coptions[i].flags=OPTION_HIDDEN;
          break;
        }
    }
}





/* Parse a single option: */
error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
  struct converttparams *p = state->input;

  /* Pass 'gal_options_common_params' into the child parser.  */
  state->child_inputs[0] = &p->cp;

  /* In case the user incorrectly uses the equal sign (for example
     with a short format or with space in the long format, then 'arg'
     start with (if the short version was called) or be (if the long
     version was called with a space) the equal sign. So, here we
     check if the first character of arg is the equal sign, then the
     user is warned and the program is stopped: */
  if(arg && arg[0]=='=')
    argp_error(state, "incorrect use of the equal sign ('='). For short "
               "options, '=' should not be used and for long options, "
               "there should be no space between the option, equal sign "
               "and value");

  /* Set the key to this option. */
  switch(key)
    {
    /* Read the non-option tokens (arguments): */
    case ARGP_KEY_ARG:
      /* The user may give a shell variable that is empty! In that case
         'arg' will be an empty string! We don't want to account for such
         cases (and give a clear error that no input has been given). */
      if(arg[0]!='\0') gal_list_str_add(&p->inputnames, arg, 0);
      break;

    /* This is an option, set its value. */
    default:
      return gal_options_set_from_key(key, arg, p->cp.poptions, &p->cp);
    }

  return 0;
}




















/**************************************************************/
/***************       Sanity Check         *******************/
/**************************************************************/
static void
ui_colormap_sanity_check(struct converttparams *p)
{
  char **strarr;
  float *farray;
  size_t nparams=0;
  int ccode=COLOR_INVALID;

  /* See how many parameters are necessary.
     Notes for TAB completion:
        1. Keep 'gray' and 'grey' in the same line.
        2. Keep a space after the ',' before the strings.   */
  strarr=p->colormap->array;
  if     ( !strcmp(strarr[0], "hsv"))    {ccode=COLOR_HSV;     nparams=2;}
  else if( !strcmp(strarr[0], "sls"))    {ccode=COLOR_SLS;     nparams=0;}
  else if( !strcmp(strarr[0], "viridis")){ccode=COLOR_VIRIDIS; nparams=0;}
  else if( !strcmp(strarr[0], "gray") || !strcmp(strarr[0], "grey"))
                                         { ccode=COLOR_GRAY; nparams=0; }
  else if( !strcmp(strarr[0], "sls-inverse"))
    { ccode=COLOR_SLS_INVERSE; nparams=0; }
  else
    error(EXIT_FAILURE, 0, "'%s' not recognized as a colormap given "
          "to '--colormap'", strarr[0]);
  p->colormap->status=ccode;

  /* Check if the proper number of parameters are given for this color
     space. Note that the actual colorspace name is the first element in
     'monotocolor'. */
  if(p->colormap->size!=1 && p->colormap->size != (nparams+1) )
    error(EXIT_FAILURE, 0, "%zu parameters given to '--monotocolor' for "
          "the '%s' color space (which needs %zu)",
          p->colormap->size-1, strarr[0], nparams);

  /* Allocate the necessary space for the parameters (when
     necessary). */
  if(nparams>0)
    {
      /* If no parameters were given, put the full range. */
      if(p->colormap->size==1)
        {
          p->colormap->next=gal_data_alloc(NULL, GAL_TYPE_FLOAT32, 1,
                                           &nparams, NULL, 0,
                                           p->cp.minmapsize,
                                           p->cp.quietmmap,
                                           NULL,NULL,NULL);
          farray=p->colormap->next->array;
          switch(p->colormap->status)
            {
            case COLOR_HSV: farray[0]=0; farray[1]=360; break;
            default:
              error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at "
                    "%s to fix the problem. The value '%d' is not "
                    "recognized for a color space that needs default "
                    "parameters", __func__, PACKAGE_BUGREPORT,
                    p->colormap->status);
            }
        }
      else
        {
          /* Increment the array pointer (temporarily) so we can read
             the rest of the parameters as float32. Note that we can't
             use '+=' because it is a 'void *' pointer. We'll have to
             use 'strarry', because its type is defined. */
          p->colormap->size-=1;
          p->colormap->array = strarr + 1;
          p->colormap->next=gal_data_copy_to_new_type(p->colormap,
                                                       GAL_TYPE_FLOAT32);
          p->colormap->array = strarr;
          p->colormap->size+=1;

          /* For a check
             {
               size_t i;
               farray=p->monotocolor->next->array;
               for(i=0;i<p->monotocolor->next->size;++i)
               printf("%f\n", farray[i]);
               exit(1);
             }
          */
        }
    }
}





/* List the acceptable colors with a demo of what they look like (by
   setting the background color of an "EXAMPLE" string to the desired color
   following the ANSI escape sequence standard:
   https://en.wikipedia.org/wiki/ANSI_escape_code */
static void
ui_list_colors(struct converttparams *p)
{
  size_t i;
  int rgbi[3];
  float rgbf[3];

  /* Print the metadata lightgoldenrodyellow */
  printf("# Column 1: Color-ID   [counter, u8] Color's numerical identifier.\n");
  printf("# Column 2: Color-Name [name, str20] Extended Web color name.\n");
  printf("# Column 3: FRAC-R     [frac,   f32] Fraction of Red.\n");
  printf("# Column 4: FRAC-G     [frac,   f32] Fraction of Green.\n");
  printf("# Column 5: FRAC-B     [frac,   f32] Fraction of Blue.\n");
  printf("# Column 6: HEX        [hex,   str6] Color code in hexadecimal.\n");
  printf("# Column 7: EXAMPLE    [n/a,  str35] Example of color in 24-bit "
         "terminals\n");

  /* Print each color's information. */
  for(i=1;i<GAL_COLOR_NUMBER;++i)
    {
      gal_color_in_rgb(i, rgbf);
      rgbi[0]=rgbf[0]*255; rgbi[1]=rgbf[1]*255; rgbi[2]=rgbf[2]*255;
      printf("%-3zu %-20s %-5.2f %-5.2f %-5.2f %02X%02X%02X  "
             "\x1b[48;2;%d;%d;%dm EXAMPLE \x1b[0m\n",
             i, gal_color_id_to_name(i),
             rgbf[0], rgbf[1], rgbf[2],
             rgbi[0], rgbi[1], rgbi[2],
             rgbi[0], rgbi[1], rgbi[2]);
    }

  /* Print information about colors. */
  if( !p->cp.quiet )
    {
      printf("#\n");
      printf("# When viewed within a 24-bit or \"true color\" terminal, "
             "the demonstration ('EXAMPLE') column will show the desired "
             "color as the background of the text 'EXAMPLE'. If your "
             "terminal doesn't support 24-bit true color or the ANSI "
             "escape sequence standard "
             "(https://en.wikipedia.org/wiki/ANSI_escape_code), the "
             "last column's color will either be rounded to the "
             "nearest supported color, or that column may be displayed "
             "as a long string of numbers and brackets (which are the "
             "raw source behind the color-coding). On macOS, the default "
             "terminal emulator (iTerm) doesn't support 24-bit colors, "
             "so it is recommended to install and use iTerm2 "
             "(https://iterm2.com: it is free software and available "
             "in Homebrew). This message can be removed with the "
             "'--quiet' (or '-q') option.\n");
    }

  /* There is nothing else for the program to do, simply return
     successfully. */
  exit(EXIT_SUCCESS);
}





/* List the available fonts for the user to select from. Ghostscript
   command was inspired from
   https://superuser.com/questions/379384/ghostscript-how-do-i-find-out-what-fonts-are-available*/
static void
ui_list_fonts(struct converttparams *p)
{
  char command[]="gs -q -dNODISPLAY -dBATCH "
    "-c '(*) {cvn ==} 256 string /Font resourceforall' "
    "| sed -e's|^/||'";

  if(system(command))
    error(EXIT_FAILURE, 0, "the Ghostscript command (printed after "
          "this message) to list the available fonts was not "
          "successful! The Ghostscript command was: %s", command);

  /* Let the users know about '--showfonts'. */
  if(!p->cp.quiet)
    printf("#\n# NOTICE: with '--showfonts' you can see all the "
           "fonts in a PDF file. This can help if you aren't "
           "already familiar with the shapes of each font. You "
           "can remove this notice with the '--quiet' option\n");

  /* Abort the program. */
  exit(EXIT_SUCCESS);
}





/* List the available fonts for the user to select from */
static void
ui_show_fonts(struct converttparams *p)
{
  FILE *fp;
  char *outps, *outpdf, *command;

  /* Set the PDF and PS file names. */
  outpdf = gal_checkset_automatic_output(&p->cp, p->cp.output, "-fonts.pdf");
  gal_checkset_writable_remove(outpdf, 0, p->cp.dontdelete);
  outps=gal_checkset_automatic_output(&p->cp, outpdf, ".ps");

  /* This command was taken from the 'ps2pdfwr' installed script that comes
     with Ghostscript by default ('ps2pdfwr' is ultimately what 'ps2pdf'
     calls, after some checks on the PDF version). */
  if( asprintf(&command, "gs -P- -dSAFER -q -P- -dNOPAUSE -dBATCH "
               "-sDEVICE=pdfwrite -sOutputFile=%s %s", outpdf, outps)<0 )
    error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);

  /* Write the contents of the PostScript file. Inspired from:
     https://superuser.com/questions/379384/ghostscript-how-do-i-find-out-what-fonts-are-available*/
  errno=0;
  fp=fopen(outps, "w");
  if(fp==NULL)
    error(EXIT_FAILURE, errno, "%s", outps);
  fprintf(fp, "%%!\n");
  fprintf(fp, "<< /PageSize [500 80] >> setpagedevice\n");
  fprintf(fp, "(*) {dup cvn findfont 20 scalefont setfont\n");
  fprintf(fp, "10 50 moveto show\n");
  fprintf(fp, "10 10 moveto (ABCDEFGHIJKLMNOPQRSTUVWXYZ) show showpage}\n");
  fprintf(fp, "256 string /Font resourceforall\n");
  fprintf(fp, "%%%%EOF");
  fclose(fp);

  /* Convert this to PDF. */
  if(system(command))
    error(EXIT_FAILURE, 0, "the Ghostscript command (printed after "
          "this message) to list the available fonts was not "
          "successful! The Ghostscript command was: %s", command);

  /* Delete the PostScript file. */
  errno=0;
  if(unlink(outps))
    error(EXIT_FAILURE, errno, "%s", outps);

  /* Let the user know that the printed fonts are now available. */
  if(!p->cp.quiet)
    printf("Fonts shown in (one page per font): %s\n", outpdf);

  /* Cleanup and finish. */
  free(outps);
  free(outpdf);
  exit(EXIT_SUCCESS);
}





/* Read and check ONLY the options. When arguments are involved, do the
   check in 'ui_check_options_and_arguments'. */
static void
ui_read_check_only_options(struct converttparams *p)
{
  gal_data_t *cond;

  /* If the user has asked to list colors or fonts,, that is the only thing
     the program should do. */
  if(p->listcolors || p->listfonts || p->showfonts)
    {
      if(p->listcolors + p->listfonts + p->showfonts > 1)
        error(EXIT_FAILURE, 0, "only one of the '--listcolors', "
              "'--listfonts' or '--showfonts' should be called in "
              "one command");
      if(p->listfonts)  ui_list_fonts(p);
      if(p->showfonts)  ui_show_fonts(p);
      if(p->listcolors) ui_list_colors(p);
    }

  /* Read the truncation values into a data structure and see if flux low
     is indeed smaller than fluxhigh. */
  if(p->fluxlowstr)
    {
      p->fluxlow=gal_data_copy_string_to_number(p->fluxlowstr);
      if(p->fluxlow==NULL)
        error(EXIT_FAILURE, 0, "value to the '--fluxlow' ('-L', %s) "
              "couldn't be read as a number", p->fluxlowstr);
    }

  if(p->fluxhighstr)
    {
      p->fluxhigh=gal_data_copy_string_to_number(p->fluxhighstr);
      if(p->fluxhigh==NULL)
        error(EXIT_FAILURE, 0, "value to the '--fluxhigh' ('-H', %s) "
              "couldn't be read as a number", p->fluxhighstr);
    }

  if(p->fluxhighstr && p->fluxlowstr)
    {
      cond=gal_arithmetic(GAL_ARITHMETIC_OP_GT, 1,
                          GAL_ARITHMETIC_FLAG_NUMOK,
                          p->fluxhigh, p->fluxlow);

      if( *((unsigned char *)cond->array) == 0 )
        error(EXIT_FAILURE, 0, "The value of '--fluxlow' must be less "
              "than '--fluxhigh'");

      gal_data_free(cond);
    }

  /* Check the colormap. */
  if(p->colormap)
    ui_colormap_sanity_check(p);

  /* Check the marks information (the minimum required parameters are the X
     and Y positions). */
  if(p->marksname)
    {
      /* If the mark coordinates are given, blend them into one list  */
      if(p->markcoords)
        {
          gal_options_merge_list_of_csv(&p->markcoords);
          if(gal_list_str_number(p->markcoords)!=2)
            error(EXIT_FAILURE, 0, "two values should be give to the "
                  "'--markcoords' (or '-r') option, while you have given "
                  "%zu", gal_list_str_number(p->markcoords));
        }
      else
        error(EXIT_FAILURE, 0, "the '--markcoords' (or '-r') is necessary "
              "to define the positions of the marks over the output (recall "
              "that marks are only supported in EPS or PDF formats)");

      /* It is mandatory to define a mode (of 'wcs' or 'img). */
      if( p->mode )
        {
          if( strcmp(p->mode, "wcs") && strcmp(p->mode, "img") )
            error(EXIT_FAILURE, 0, "'%s' is not recognized for the "
                  "'--mode' (or '-O') option. The recognized values "
                  "are 'img' or 'wcs'", p->mode);
        }
      else
        error(EXIT_FAILURE, 0, "the '--mode' (or '-O') is necessary "
              "to define how the mark coordinates should be interpreted "
              "(recall that marks are only supported in EPS or PDF "
              "formats)");

      /* Make sure the size column(s) are in one list. */
      if(p->marksize)
        {
          gal_options_merge_list_of_csv(&p->marksize);
          if(gal_list_str_number(p->marksize)>2)
            error(EXIT_FAILURE, 0, "the '--marksize' option takes two "
                  "values (column names or numbers) at most, but you "
                  "have given %zu values",
                  gal_list_str_number(p->marksize));
        }
    }
}





static void
ui_check_options_and_arguments(struct converttparams *p)
{
  /* Reverse the 'inputnames' linked list if it was given (recall that we
     also accept input from the standard input). Note that the 'hdu' linked
     list was reversed during option parsing, so we don't need to do it
     here any more. */
  gal_list_str_reverse(&p->inputnames);
}




















/**************************************************************/
/***************       Preparations         *******************/
/**************************************************************/
static struct change *
ui_make_change_struct(char *arg)
{
  char *p=arg;
  gal_data_t *data;
  size_t len=0, counter=0;
  struct change *out=NULL, *last=NULL, *ch;

  /* First set all the delimiters to '\0' and count the number of
     characters in the full string. */
  while(*p!='\0')
    {
      if( isspace(*p) || *p==':' || *p==',' ) *p='\0';
      ++p;
    }
  len=p-arg;

  /* Now, go through the string and read everything that remains. */
  p=arg;
  while(p<arg+len)
    {
      if(*p=='\0')
        ++p;
      else
        {
          /* Read the number and increment the counter. */
          ++counter;
          data=gal_data_copy_string_to_number(p);
          if(data==NULL)
            error(EXIT_FAILURE, 0, "'%s' (input number %zu to the "
                  "'--change' option) couldn't be read as a number", p,
                  counter);

          /* Go to the end of this number (until you reach a '\0'). */
          while(*p!='\0') {++p; continue;}

          /* Put the data structure in the correct place. When the counter
             is an odd number, we have just started a new set of changes.*/
          if(counter%2)             /* Odd. */
            {
              /* Allocate space for the new structure. */
              errno=0;
              ch=malloc(sizeof *ch);
              if(ch==NULL)
                error(EXIT_FAILURE, errno, "%s: allocating %zu bytes "
                      "for 'ch'", __func__, sizeof *ch);

              /* If the last structure has already been defined (!=NULL)
                 then we should set its next element to 'ch' and change it
                 to point to 'ch'. On the other hand, when this is the
                 first structure to be created, then 'last==NULL', so to
                 start off the process, we should put 'ch' into both the
                 'out' and 'last' lists.. */
              if(last)
                {
                  last->next=ch;
                  last=ch;
                }
              else
                out=last=ch;

              /* Put 'data' in the 'from' element, and since this is the
                 last structure, set its next element to NULL. */
              last->from=data;
              last->next=NULL;
            }
          else                      /* Even. */
            last->to=data;
        }
    }

  /*
    {
    struct change *tmp;
    for(tmp=out;tmp!=NULL;tmp=tmp->next)
    printf("%f --> %f\n", tmp->from, tmp->to);
    }
  */
  return out;
}





/* Go through the input files and make a linked list of all the channels
   that exist in them. When this function finishes the list of channels
   will be filled in the same order as they were read from the inputs. */
static void
ui_make_channels_ll(struct converttparams *p)
{
  char *hdu=NULL;
  gal_data_t *data;
  size_t dsize=0, dirnum;
  gal_list_str_t *name, *lines;

  /* Initialize the counting of channels. */
  p->numch=0;

  /* If any standard input is provided, we want to process that first. Note
     that since other input arguments are also allowed (as other channels),
     we'll need to process the standard input independently first, then go
     onto the possible list of other files.*/
  lines=gal_txt_stdin_read(p->cp.stdintimeout);
  if(lines)
    {
      data=gal_txt_image_read(NULL, lines, p->cp.minmapsize,
                              p->cp.quietmmap);
      gal_list_data_add(&p->chll, data);
      gal_list_str_free(lines, 1);
      ++p->numch;
    }


  /* Go through the input files and add the channel(s). */
  for(name=p->inputnames; name!=NULL; name=name->next)
    {
      /* Check if p->numch has not exceeded 4. */
      if(p->numch>=4)
        error(EXIT_FAILURE, 0, "the number of input color channels (not "
              "necessarily files) has exceeded 4! Note that one file can "
              "contain more than one color channel (for example a JPEG "
              "file in RGB has 3 channels)");

      /* Make sure this input file exists (if it isn't blank). */
      if(strcmp(name->v, "blank")) gal_checkset_check_file(name->v);

      /* FITS: */
      if( gal_fits_file_recognized(name->v) )
        {
          /* Get the HDU value for this channel. */
          if(p->globalhdu)
            hdu=p->globalhdu;
          else
            {
              if(p->hdus)
                hdu=gal_list_str_pop(&p->hdus);
              else
                error(EXIT_FAILURE, 0, "not enough HDUs. Every input FITS "
                      "image needs a HDU (identified by name or number, "
                      "counting from zero). You can use multiple calls to "
                      "the '--hdu' ('-h') option for each input FITS "
                      "image (in the same order as the input FITS files), "
                      "or use '--globalhdu' ('-g') once when the same "
                      "HDU should be used for all of them");
            }

          /* Read in the array and its WCS information. */
          data=gal_fits_img_read(name->v, hdu, p->cp.minmapsize,
                                 p->cp.quietmmap);
          data->wcs=gal_wcs_read(name->v, hdu, p->cp.wcslinearmatrix,
                                 0, 0, &data->nwcs);
          data->ndim=gal_dimension_remove_extra(data->ndim, data->dsize,
                                                data->wcs);
          gal_list_data_add(&p->chll, data);

          /* A FITS file only has one channel. */
          ++p->numch;
        }


      /* TIFF: */
      else if( gal_tiff_name_is_tiff(name->v) )
        {
          /* Get the directory value for this channel. */
          if(p->hdus)
            {
              hdu=gal_list_str_pop(&p->hdus);
              dirnum=gal_tiff_dir_string_read(hdu);
            }
          else
            dirnum=0;

          /* Read the TIFF image into memory. */
          data=gal_tiff_read(name->v, dirnum, p->cp.minmapsize,
                             p->cp.quietmmap);
          p->numch += gal_list_data_number(data);
          gal_list_data_add(&p->chll, data);
        }


      /* JPEG: */
      else if ( gal_jpeg_name_is_jpeg(name->v) )
        {
          data=gal_jpeg_read(name->v, p->cp.minmapsize, p->cp.quietmmap);
          p->numch += gal_list_data_number(data);
          gal_list_data_add(&p->chll, data);
        }



      /* Blank: */
      else if(strcmp(name->v, BLANK_CHANNEL_NAME)==0)
        {
          gal_list_data_add_alloc(&p->chll, NULL, GAL_TYPE_INVALID, 0,
                                  &dsize, NULL, 0, p->cp.minmapsize,
                                  p->cp.quietmmap, "blank", NULL, NULL);
          ++p->numch;
        }



      /* EPS:  */
      else if ( gal_eps_name_is_eps(name->v) )
        error(EXIT_FAILURE, 0, "EPS files cannot be used as input. Since "
              "EPS files are not raster graphics. EPS is only an output "
              "format");



      /* PDF:  */
      else if ( gal_pdf_name_is_pdf(name->v) )
        error(EXIT_FAILURE, 0, "PDF files cannot be used as input. Since "
              "PDF files are not raster graphics. PDF is only an output "
              "format");


      /* Text: */
      else
        {
          data=gal_txt_image_read(name->v, NULL, p->cp.minmapsize,
                                  p->cp.quietmmap);
          gal_list_data_add(&p->chll, data);
          ++p->numch;
        }
    }

  /* If there weren't any channels, abort with an error. */
  if(p->numch==0)
    error(EXIT_FAILURE, 0, "%s",
          gal_options_stdin_error(p->cp.stdintimeout, 0, "input"));

  /* Reverse the list of channels into the input order. */
  gal_list_data_reverse(&p->chll);
}





static void
ui_prepare_input_channels_check_wcs(struct converttparams *p)
{
  int printwarning=0;
  float wcsmatch=1.0;
  gal_data_t *tmp, *coords=NULL;
  size_t one=1, numwcs=0, numnonblank=0;
  double *c1, *c2, r1=NAN, r2=NAN, *pixscale=NULL;

  /* If all the inputs have WCS, check to see if the inputs are aligned and
     print a warning if they aren't. */
  for(tmp=p->chll; tmp!=NULL; tmp=tmp->next)
    {
      if(tmp->wcs && tmp->type!=GAL_TYPE_INVALID) ++numwcs;
      if(tmp->type!=GAL_TYPE_INVALID)             ++numnonblank;
    }
  if(numwcs==numnonblank)
    {
      /* Allocate the coordinate columns. */
      gal_list_data_add_alloc(&coords, NULL, GAL_TYPE_FLOAT64, 1,
                              &one, NULL, 0, -1, 1, NULL, NULL, NULL);
      gal_list_data_add_alloc(&coords, NULL, GAL_TYPE_FLOAT64, 1,
                              &one, NULL, 0, -1, 1, NULL, NULL, NULL);

      /* Go over each image and put its central pixel in the coordinates
         and do the world-coordinate transformation. Recall that the C
         coordinates are the inverse order of FITS coordinates and that
         FITS coordinates count from 1 (not 0).*/
      for(tmp=p->chll; tmp!=NULL; tmp=tmp->next)
        if(tmp->wcs)
          {
            /* Fill the coordinate values. */
            c1=coords->array;
            c2=coords->next->array;
            c1[0] = tmp->dsize[1] / 2 + 1;
            c2[0] = tmp->dsize[0] / 2 + 1;

            /* Get the RA/Dec. */
            gal_wcs_img_to_world(coords, tmp->wcs, 1);

            /* If the pixel scale hasn't been calculated yet, do it (we
               only need it once, should be similar in all). */
            if(pixscale==NULL)
              pixscale=gal_wcs_pixel_scale(tmp->wcs);

            /* If the reference/first center is not yet defined then write
               the conversions in it. If it is defined, compare with it
               with the new dataset and print a warning if necessary. */
            if( isnan(r1) )
              { r1=c1[0]; r2=c2[0]; }
            else
              {
                /* For a check.
                printf("check: %g, %g\n", fabs(c1[0]-r1)/pixscale[0],
                       fabs(c2[0]-r2)/pixscale[1]);
                */

                /* See if a warning should be printed. */
                if( fabs(c1[0]-r1)/pixscale[0] > wcsmatch
                    || fabs(c2[0]-r2)/pixscale[1] > wcsmatch )
                  printwarning=1;
              }
          }
    }

  /* Print the warning message if necessary. */
  if(printwarning && p->cp.quiet==0)
    {
      error(EXIT_SUCCESS, 0, "WARNING: The WCS information of the input "
            "FITS images don't match (by more than %g pixels in the "
            "center), even though the input images have the same number "
            "of pixels in each dimension. Therefore the color channels "
            "of the output colored image may not be aligned. If this is "
            "not a problem, you can suppress this warning with the "
            "'--quiet' option.\n\n"
            "A solution to align your images is provided in the "
            "\"Aligning images with small WCS offsets\" section of "
            "Gnuastro's manual. Please run the command below to see "
            "it (you can return to the command-line by pressing 'q'):\n\n"
            "   info gnuastro \"Aligning images\"\n",
            wcsmatch);
    }

  /* Clean up. */
  free(pixscale);
}





/* Read the input(s)/channels. */
static void
ui_prepare_input_channels(struct converttparams *p)
{
  struct wcsprm *wcs=NULL;
  size_t i, ndim=0, *dsize=NULL;
  gal_data_t *tmp, *blank, *prev;

  /* Fill in the channels linked list. */
  ui_make_channels_ll(p);


  /* Make sure there are 1 (for grayscale), 3 (for RGB) or 4 (for CMYK)
     color channels. */
  if(p->numch!=1 && p->numch!=3 && p->numch!=4)
    error(EXIT_FAILURE, 0, "the number of input color channels has to "
          "be 1 (for non image data, grayscale or only K channel in "
          "CMYK), 3 (for RGB) and 4 (for CMYK). You have given %zu "
          "color channels. Note 1: some file formats (for example "
          "JPEG in RGB mode) can contain more than one color channel, "
          "if such a file is given all its channels are read, so "
          "separate them first. Note 2: if your first input channel "
          "was given through the standard input (piped from another "
          "program) you can fix this error by giving a larger value "
          "to the '--stdintimeout' option (currently %ld "
          "micro-seconds)", p->numch, p->cp.stdintimeout);


  /* If there are multiple color channels, then ignore the monotocolor
     option if it is given. But if there is only one, make sure that the
     'colormap' option is actually given.*/
  if( p->numch==1 )
    {
      if( p->colormap==NULL )
        error(EXIT_FAILURE, 0, "no colormap! When there is only one input "
              "channel, it is necessary to specify a color map. For "
              "example 'gray', 'hsv', 'viridis' or 'sls'.\n\n"
              "For more on ConvertType's color mapping, see the "
              "description under '--colormap' in the Gnuastro book:\n\n"
              "   $ info astconvertt");
    }
  else if( p->numch>1 && p->colormap )
    {
      if(p->colormap->next) gal_data_free(p->colormap->next);
      gal_data_free(p->colormap);
      p->colormap=NULL;
    }


  /* Go over the channels and make the proper checks/corrections. We won't
     be checking blank channels here, recall that blank channels had a
     dimension of zero. */
  for(tmp=p->chll; tmp!=NULL; tmp=tmp->next)
    if(tmp->ndim>0)
      {
        /* Set the reference size (to check and also use for the blank
           channels). */
        if(dsize==NULL)
          {
            ndim=tmp->ndim;
            dsize=tmp->dsize;
          }
        else
          {
            if(tmp->ndim!=ndim)
              error(EXIT_FAILURE, 0, "All channels must have the same "
                    "number of dimensions, the first input channel had "
                    "%zu dimensions while atleast one other has %zu",
                    ndim, tmp->ndim);
            for(i=0;i<ndim;++i)
              if(dsize[i]!=tmp->dsize[i])
                error(EXIT_FAILURE, 0, "The length along each dimension "
                      "of the channels must be the same");
          }

        /* Incase there is WCS information, also keep a pointer to the
           first WCS information encountered. */
        if(wcs==NULL && tmp->wcs)
          wcs=tmp->wcs;
      }

  /* Make sure the images are all aligned to the same grid. */
  ui_prepare_input_channels_check_wcs(p);

  /* If ndim is still NULL, then there were no non-blank inputs, so print
     an error. */
  if(dsize==NULL)
    error(EXIT_FAILURE, 0, "all the input(s) are of type blank");


  /* Now, fill in the blank channels with zero valued arrays. */
  prev=NULL;
  for(tmp=p->chll; tmp!=NULL; tmp=tmp->next)
    {
      /* If this is a blank structure, then set it to a zero valued
         array. */
      if(tmp->ndim==0)
        {
          /* Make the blank data structure. */
          blank=gal_data_alloc(NULL, GAL_TYPE_UINT8, ndim, dsize,
                               wcs, 1, p->cp.minmapsize, p->cp.quietmmap,
                               "blank channel", NULL, NULL);

          /* We will use the status value of the data structuer to mark it
             as one that was originally blank. */
          blank->status=1;

          /* If a previous node pointed to this old blank structure, then
             correct it. */
          if(prev) prev->next=blank; else p->chll=blank;

          /* Set the next pointer of this one to same pointer that the old
             blank pointer pointed to. */
          blank->next=tmp->next;

          /* Free the old data structure and put this one in its place. */
          gal_data_free(tmp);
          tmp=blank;
        }

      /* This is the final (to be used) data structure, so keep its pointer
         in case the next one is blank and this structure's 'next' element
         must be corrected. */
      prev=tmp;
    }
}





/* We know cp->output is a known suffix, we just don't know if it has a '.'
   before it or not. If it doesn't, one will be added to it and the output
   name will be set using the automatic output function. */
void
ui_add_dot_use_automatic_output(struct converttparams *p)
{
  gal_list_str_t *stll;
  char *tmp, *firstname="converttype.txt", *suffix=p->cp.output;

  /* Find the first non-blank file name in the input(s). */
  for(stll=p->inputnames; stll!=NULL; stll=stll->next)
    if(strcmp(stll->v, BLANK_CHANNEL_NAME))
      {
        firstname=stll->v;
        break;
      }

  /* If the suffix does not start with a '.', put one there. */
  if(suffix[0]!='.')
    {
      if( asprintf(&tmp, ".%s", suffix)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
      free(suffix);
      suffix=tmp;
    }

  /* Set the automatic output and make sure we have write access. */
  p->cp.output=gal_checkset_automatic_output(&p->cp, firstname, suffix);
}





/* Set output name, not that for ConvertType, the output option value is
   mandatory (in 'args.h'). So by the time the program reaches here, we
   know it exists. */
static void
ui_set_output(struct converttparams *p)
{
  struct gal_options_common_params *cp=&p->cp;

  /* FITS */
  if(gal_fits_name_is_fits(cp->output))
    {
      p->outformat=OUT_FORMAT_FITS;
      if( gal_fits_suffix_is_fits(cp->output) )
        ui_add_dot_use_automatic_output(p);
    }

  /* JPEG */
  else if(gal_jpeg_name_is_jpeg(cp->output))
    {
      /* If marks are necessary, then we need to use Ghostscript to put the
         marks over the image. */
      if(p->marksname)
        {
          p->outformat=OUT_FORMAT_PDF;
          if(!p->cp.quiet)
            error(EXIT_SUCCESS, 0, "WARNING: output format is JPEG (a "
                  "raster graphics format), but you have requested "
                  "vector graphics marks (which are native to formats "
                  "like PDF or EPS). The marks will therefore become "
                  "pixelated. If the pixelation over the marks is too "
                  "strong (the quality is too low!), you need to "
                  "increase the resolution. You can do this by "
                  "increasing the centimeter-width of the output by "
                  "giving a larger number to '--widthincm' (or '-w'; "
                  "currently it is %g cm). Just don't increase it too "
                  "much, otherwise your output file size will become "
                  "very large (in bytes). Vector formats are optimal "
                  "for marks (PDF or EPS) and will become much smaller "
                  "(in bytes) while having infinite resolution. Also, "
                  "Ghostscript, and its 'jpeg' output device will be "
                  "used, in case you don't have Ghostscript or this "
                  "device isn't activated, the program will crash. "
                  "This warning can be suppressed with '--quiet' "
                  "(or '-q')", p->widthincm);
        }
      else
        {
          /* Small sanity checks. */
          if(p->quality == GAL_BLANK_UINT8)
            error(EXIT_FAILURE, 0, "the '--quality' ('-u') option is "
                  "necessary for jpeg outputs, but it has not been given");
          if(p->quality > 100)
            error(EXIT_FAILURE, 0, "'%u' is larger than 100. The value to "
                  "the '--quality' ('-u') option must be between 1 and 100 "
                  "(inclusive)", p->quality);

          /* Preparations. */
          p->outformat=OUT_FORMAT_JPEG;
        }
      if( gal_jpeg_suffix_is_jpeg(cp->output) )
        ui_add_dot_use_automatic_output(p);
    }

  /* TIFF */
  else if( gal_tiff_name_is_tiff(cp->output) )
      error(EXIT_FAILURE, 0, "writing TIFF files is not yet supported, "
            "please get in touch with us at %s so we implement it",
            PACKAGE_BUGREPORT);

  /* EPS */
  else if(gal_eps_name_is_eps(cp->output))
    {
      if(p->borderwidth==0 && p->widthincm==0)
        error(EXIT_FAILURE, 0, "at least one of '--widthincm' ('-u'), or "
              "'--borderwidth ('-b') options are necessary for an EPS "
              "output");
      p->outformat=OUT_FORMAT_EPS;
      if( gal_eps_suffix_is_eps(cp->output) )
        ui_add_dot_use_automatic_output(p);
    }

  /* PDF */
  else if(gal_pdf_name_is_pdf(cp->output))
    {
      if(p->borderwidth==0 && p->widthincm==0)
        error(EXIT_FAILURE, 0, "at least one of '--widthincm' ('-u'), or "
              "'--borderwidth ('-b') options are necessary for a PDF "
                 "output");
      p->outformat=OUT_FORMAT_PDF;
      if( gal_pdf_suffix_is_pdf(cp->output) )
        ui_add_dot_use_automatic_output(p);
    }

  /* Default: plain text. */
  else
    {
      p->outformat=OUT_FORMAT_TXT;

      /* If the given value is 'stdout', then set p->cp.output to NULL, so
         the result will be printed to the standard output. */
      if( !strcmp(p->cp.output, "stdout") )
        {
          free(p->cp.output);
          p->cp.output=NULL;
        }
      else
        {
          /* Plain text files don't have any unique set of suffixes. So,
             here, we will just adopt two of the most common ones: 'txt' or
             'dat'. If the output is just one of these two suffixes, then
             we will use automatic output to generate the full name,
             otherwise, we'll just take the user's given value as the
             filename. */
          if( !strcmp(cp->output, "txt") || !strcmp(cp->output, ".txt")
              || !strcmp(cp->output, "dat") || !strcmp(cp->output, ".dat") )
            ui_add_dot_use_automatic_output(p);

          /* If output type is not an image, there should only be one color
             channel: */
          if(p->numch>1)
            error(EXIT_FAILURE, 0, "text output ('--output=%s') can only "
                  "be completed with one input color channel. You have "
                  "given %zu. Note that some formats (for example JPEG) "
                  "can have more than one color channel in each file. "
                  "You can first convert the file to FITS, then convert "
                  "the desired channel to text by specifying the HDU",
                  cp->output, p->numch);
        }
    }

  /* Check if the output already exists and remove it if allowed. */
  gal_checkset_writable_remove(cp->output, 0, cp->dontdelete);
}




















/***********************************************************************/
/****************    Marks for EPS or PDF outputs   ********************/
/***********************************************************************/
void
ui_marks_read_raw(struct converttparams *p, gal_data_t **coord1,
                  gal_data_t **coord2, gal_data_t **size1,
                  gal_data_t **size2, gal_data_t **linewidth,
                  gal_data_t **color, gal_data_t **shape,
                  gal_data_t **rotate, gal_data_t **text,
                  gal_data_t **font, gal_data_t **fontsize)
{
  size_t colnum=0;
  gal_list_str_t *cols=NULL;
  gal_data_t *table, *latest;

  /* Set the requested columns. */
  gal_list_str_add(&cols, p->markcoords->v, 1);         ++colnum;
  gal_list_str_add(&cols, p->markcoords->next->v,1);    ++colnum;
  if(p->marksize)
    { gal_list_str_add(&cols, p->marksize->v, 1);       ++colnum; }
  if(p->marksize && p->marksize->next)
    { gal_list_str_add(&cols, p->marksize->next->v, 1); ++colnum; }
  if(p->marklinewidth)
    { gal_list_str_add(&cols, p->marklinewidth, 1);     ++colnum; }
  if(p->markcolor)
    { gal_list_str_add(&cols, p->markcolor, 1);         ++colnum; }
  if(p->markshape)
    { gal_list_str_add(&cols, p->markshape, 1);         ++colnum; }
  if(p->markrotate)
    { gal_list_str_add(&cols, p->markrotate, 1);        ++colnum; }
  if(p->marktext)
    { gal_list_str_add(&cols, p->marktext, 1);          ++colnum; }
  if(p->markfont)
    { gal_list_str_add(&cols, p->markfont, 1);          ++colnum; }
  if(p->markfontsize)
    { gal_list_str_add(&cols, p->markfontsize, 1);      ++colnum; }

  /* Put the columns in the same order defined above (recall that its a
     last-in-first-out list). */
  gal_list_str_reverse(&cols);

  /* Read the table */
  table=gal_table_read(p->marksname, p->markshdu, NULL, cols,
                       p->cp.searchin, p->cp.ignorecase,
                       p->cp.numthreads, p->cp.minmapsize,
                       p->cp.quietmmap, NULL);

  /* Make sure that we have only one column for each entry (it may happen
     that a table has two columns with the same name!). */
  if(gal_list_data_number(table)!=colnum)
    error(EXIT_FAILURE, 0, "%s: more than one column was found for "
          "one of your '--mark*' columns. This usually happens when "
          "more than one column has the same name",
          gal_fits_name_save_as_string(p->marksname, p->markshdu));

  /* Put each of the columns in their proper pointer.
     IMPORTANT: KEEP THE ORDER THE SAME AS THE 'cols' DEFINITION.*/
  latest=*coord1=table;
  latest=*coord2=latest->next;
  if(p->marksize)
    {                       latest=*size1=latest->next;
      if(p->marksize->next) latest=*size2=latest->next;
    }
  if(p->marklinewidth)      latest=*linewidth=latest->next;
  if(p->markcolor)          latest=*color=latest->next;
  if(p->markshape)          latest=*shape=latest->next;
  if(p->markrotate)         latest=*rotate=latest->next;
  if(p->marktext)           latest=*text=latest->next;
  if(p->markfont)           latest=*font=latest->next;
  if(p->markfontsize)       latest=*fontsize=latest->next;

  /* Now that they are read, un-list each column sot they can be treated
     independently (we don't want their 'next' pointer to interfere with
     future steps). */
  (*coord1)->next=(*coord2)->next=NULL;
  if(p->marktext)       (*text)->next=NULL;
  if(p->markfont)       (*font)->next=NULL;
  if(p->marksize)       (*size1)->next=NULL;
  if(p->markcolor)      (*color)->next=NULL;
  if(p->markshape)      (*shape)->next=NULL;
  if(p->markrotate)     (*rotate)->next=NULL;
  if(p->markfontsize)   (*fontsize)->next=NULL;
  if(p->marklinewidth)  (*linewidth)->next=NULL;
  if(p->marksize && p->marksize->next) (*size2)->next=NULL;
}





/* Error message for cases that WCS mode has been requested by the
   inputs don't have WCS. */
static void
ui_marks_error_no_wcs(void)
{
  error(EXIT_FAILURE, 0, "none of the input channel(s) have "
        "WCS while you had defined your coordinates and sizes "
        "to be in WCS mode (with '--mode=wcs'). If your "
        "coordinates and sizes are in image coordinates "
        "(in units of pixels), please use '--mode=img'");
}





static void
ui_marks_read_coords(struct converttparams *p, gal_data_t **coord1,
                     gal_data_t **coord2)
{
  int wcsfound=0;
  gal_data_t *c1=*coord1, *c2=*coord2, *tmp;

  /* If the coordinates are in WCS mode, convert them. */
  if( !strcmp(p->mode, "wcs") )
    {
      /* Find the first channel with WCS and use it to convert the
         coordinates to pixel coordinates. */
      for(tmp=p->chll; tmp!=NULL; tmp=tmp->next)
        if(tmp->wcs)
          {
            /* The coordinates need to have 64-bit floating point type for
               the WCS conversion. */
            c1=gal_data_copy_to_new_type_free(c1, GAL_TYPE_FLOAT64);
            c2=gal_data_copy_to_new_type_free(c2, GAL_TYPE_FLOAT64);

            /* Set the second one as the 'next' of the first and do the
               conversion. */
            c1->next=c2;
            gal_wcs_world_to_img(c1, tmp->wcs, 1);
            wcsfound=1;
            c1->next=NULL;
            break;
          }

      /* If no WCS could be found, abort. */
      if(wcsfound==0) ui_marks_error_no_wcs();
    }

  /* The columns should have specific names. */
  if(c1->name) free(c1->name);
  if(c2->name) free(c2->name);
  gal_checkset_allocate_copy(GAL_EPS_MARK_COLNAME_XPIX, &c1->name);
  gal_checkset_allocate_copy(GAL_EPS_MARK_COLNAME_YPIX, &c2->name);

  /* The columns should have specific types. */
  *coord1=gal_data_copy_to_new_type_free(c1, GAL_TYPE_FLOAT32);
  *coord2=gal_data_copy_to_new_type_free(c2, GAL_TYPE_FLOAT32);
}





/* In WCS-mode, the user has given sizes in WCS units (usually degrees). We
   need to convert them to image coordinates for the EPS library. */
static void
ui_marks_size_to_image(struct converttparams *p, gal_data_t *size1,
                       gal_data_t *size2, gal_data_t *shape)
{
  uint8_t *u;
  float *f, *ff;
  gal_data_t *tmp;
  double *ps=NULL;

  /* Multiplication factor based on size. */
  double m = ( p->sizeinarcsec
               ? 3600
               : (p->sizeinarcmin ? 60 : 1.0 ) );

  /* If both arrays to convert are NULL, then don't bother with the rest of
     this function. */
  if(size1==NULL && size2==NULL) return;

  /* Find the first channel with WCS and use it to convert the
     coordinates to pixel coordinates. */
  for(tmp=p->chll; tmp!=NULL; tmp=tmp->next)
    if(tmp->wcs)
      ps=gal_wcs_pixel_scale(tmp->wcs);

  /* If no WCS could be found, print an error. */
  if(ps==NULL) ui_marks_error_no_wcs();

  /* Use the first dimension's pixel scale for 'size1', and second
     dimension's pixel scale for 'size2'. */
  if(size1)
    { ff=(f=size1->array)+size1->size; do *f /= m*ps[0]; while(++f<ff); }
  if(size2)
    {
      /* The second size column is only relevant if a shape is defined.
         Also, for the ellipse rows, we shouldn't multiply by the pixel
         scale. */
      if(shape)
        {
          u=shape->array;
          ff=(f=size2->array)+size1->size;
          do if(*u++!=GAL_EPS_MARK_SHAPE_ELLIPSE) *f /= m*ps[1];
          while(++f<ff);
        }
    }
}





/* Make sure named-columns (like shape or color) are in the code-format. */
static gal_data_t *
ui_marks_read_named_cols(gal_data_t *in, int shape1_color2)
{
  size_t i;
  uint8_t n, *u;
  gal_data_t *out=NULL;
  char *name=NULL, **strarr, *modestr;

  /* Prepare the output based on type. */
  if(in->type==GAL_TYPE_STRING)
    {
      /* Allocate the output dataset. */
      out=gal_data_alloc(NULL, GAL_TYPE_UINT8, 1, in->dsize, NULL,
                         0, in->minmapsize, in->quietmmap, NULL,
                         NULL, NULL);

      /* Go over the array and convert the names to codes. If the name is
         not correct the functions will abort with an informative error.*/
      u=out->array;
      strarr=in->array;
      switch(shape1_color2)
        {
        case 1: /* Shape */
          for(i=0;i<in->size;++i) u[i]=gal_eps_shape_name_to_id(strarr[i]);
          name="SHAPE";
          break;
        case 2: /* Color */
          for(i=0;i<in->size;++i) u[i]=gal_color_name_to_id(strarr[i]);
          name="COLOR";
          break;
        default:
          error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' "
                "to find and fix the problem. The operation code '%d' "
                "isn't recognized", __func__, PACKAGE_BUGREPORT,
                shape1_color2);
        }

      /* Clean the input column. */
      gal_data_free(in);
    }
  else
    {
      /* Set the constants. */
      name = shape1_color2==1 ? "SHAPE" : "COLOR";
      modestr = shape1_color2==1 ? "shape" : "color";
      n=shape1_color2==1 ? GAL_EPS_MARK_SHAPE_NUMBER : GAL_COLOR_NUMBER;

      /* Convert the type and do a sanity check. */
      out=gal_data_copy_to_new_type_free(in, GAL_TYPE_UINT8);
      u=out->array;
      for(i=0;i<in->size;++i)
        {
        if( u[i]==0 || u[i]>n )
          error(EXIT_FAILURE, 0, "the %s numerical identifier '%u' "
                "(in row %zu) is not recognized! The largest "
                "numerical identifier for %ss is %u",
                modestr, u[i], i, modestr, n);
        }

      /* Set the dataset name. */
    }

  /* Set the specific name of the output. */
  if(out->name) free(out->name);
  gal_checkset_allocate_copy(name, &out->name);

  /* Return the output dataset. */
  return out;
}





/* All numbered columns should have a float32 type with a specific name. */
static gal_data_t *
ui_marks_read_fixedtype_col(struct converttparams *p, gal_data_t *in,
                            uint8_t type, char *name, int onlypositive,
                            char *colname)
{
  float *f;
  size_t i;
  gal_data_t *out=gal_data_copy_to_new_type_free(in, type);

  /* Small sanity check. */
  if(type!=GAL_TYPE_FLOAT32 && type!=GAL_TYPE_STRING)
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' "
          "to fix the problem. The input's type should either be "
          "float32 or string, but it is '%s'", __func__,
          PACKAGE_BUGREPORT, gal_type_name(type, 1));

  /* If it is important that this column is positive, do the check. */
  if(onlypositive)
    {
      /* Small sanity check. */
      if(type!=GAL_TYPE_FLOAT32)
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' "
              "to fix the problem. When 'onlypositive' is set, the "
              "type should be float32, but it is '%s'",  __func__,
              PACKAGE_BUGREPORT, gal_type_name(type, 1));

      /* Make sure all the elements are positive. */
      f=out->array;
      for(i=0;i<out->size;++i)
        if(f[i]<0.0)
          error(EXIT_FAILURE, 0, "%s: column '%s', row %zu has a "
                "negative value (%g)! This column's values should "
                "be positive",
                gal_fits_name_save_as_string(p->marksname, p->markshdu),
                colname, i+1, f[i]);
    }

  /* Free the possibly existing current name. */
  if(out->name) free(out->name);
  gal_checkset_allocate_copy(name, &out->name);

  /* Return the proper dataset. */
  return out;
}





void
ui_marks_read(struct converttparams *p)
{
  size_t i;
  float *s2arr;
  uint8_t *sharr;
  gal_data_t *font=NULL, *fontsize=NULL;
  gal_data_t *size2=NULL, *rotate=NULL, *text=NULL;
  gal_data_t *color=NULL, *shape=NULL, *lwidth=NULL;
  gal_data_t *coord1=NULL, *coord2=NULL, *size1=NULL;

  /* Read the columns. */
  ui_marks_read_raw(p, &coord1, &coord2, &size1, &size2,
                    &lwidth, &color, &shape, &rotate, &text,
                    &font, &fontsize);

  /* Prepare the coordinates. */
  ui_marks_read_coords(p, &coord1, &coord2);

  /* Prepare the shape and color. */
  if(shape) shape=ui_marks_read_named_cols(shape, 1);
  if(color) color=ui_marks_read_named_cols(color, 2);

  /* Set the precision to print floating point numbers as strings (if the
     user has provided one. */
  if(p->marktextprecision>0)
    text->disp_precision=p->marktextprecision;

  /* Prepare the size (the EPS library needs each dataset in the input list
     for marks to have specific names and specific formats). */
  if(size1)
    size1 = ui_marks_read_fixedtype_col(p, size1, GAL_TYPE_FLOAT32,
                                        GAL_EPS_MARK_COLNAME_SIZE1, 1,
                                        p->marksize->v);
  if(size2)
    size2 = ui_marks_read_fixedtype_col(p, size2, GAL_TYPE_FLOAT32,
                                        GAL_EPS_MARK_COLNAME_SIZE2, 1,
                                        p->marksize->next->v);
  if(rotate)
    rotate = ui_marks_read_fixedtype_col(p, rotate, GAL_TYPE_FLOAT32,
                                         GAL_EPS_MARK_COLNAME_ROTATE, 0,
                                         p->markrotate);
  if(lwidth)
    lwidth = ui_marks_read_fixedtype_col(p, lwidth, GAL_TYPE_FLOAT32,
                                         GAL_EPS_MARK_COLNAME_LINEWIDTH, 1,
                                         p->marklinewidth);
  if(text)
    text = ui_marks_read_fixedtype_col(p, text, GAL_TYPE_STRING,
                                       GAL_EPS_MARK_COLNAME_TEXT, 0,
                                       p->marktext);
  if(font)
    font = ui_marks_read_fixedtype_col(p, font, GAL_TYPE_STRING,
                                       GAL_EPS_MARK_COLNAME_FONT, 0,
                                       p->markfont);
  if(fontsize)
    fontsize = ui_marks_read_fixedtype_col(p, fontsize, GAL_TYPE_FLOAT32,
                                           GAL_EPS_MARK_COLNAME_FONTSIZE, 1,
                                           p->markfontsize);

  /* Convert the sizes to pixel if necessary. */
  if( !strcmp(p->mode,"wcs") && p->sizeinpix==0 )
    ui_marks_size_to_image(p, size1, size2, shape);

  /* Put the columns in the list to pass to the EPS library. */
  gal_list_data_add(&p->marks, coord1);
  gal_list_data_add(&p->marks, coord2);
  if(text)     gal_list_data_add(&p->marks, text);
  if(font)     gal_list_data_add(&p->marks, font);
  if(shape)    gal_list_data_add(&p->marks, shape);
  if(color)    gal_list_data_add(&p->marks, color);
  if(size1)    gal_list_data_add(&p->marks, size1);
  if(size2)    gal_list_data_add(&p->marks, size2);
  if(lwidth)   gal_list_data_add(&p->marks, lwidth);
  if(rotate)   gal_list_data_add(&p->marks, rotate);
  if(fontsize) gal_list_data_add(&p->marks, fontsize);

  /* Some sanity checks. */
  if(shape && size2)
    {
      sharr=shape->array;
      s2arr=size2->array;
      for(i=0;i<p->marks->size;++i)
        if( sharr[i]==GAL_EPS_MARK_SHAPE_ELLIPSE
            && ( s2arr[i]<=0 || s2arr[i]>1 ) )
          error(EXIT_FAILURE, 0, "%g is not a valid 'size2' column "
                "for an ellipse shape (from row number %zu of the "
                "marks table). For an ellipse, the 'size2' column is "
                "the axis ratio, so it should always be larger than "
                "0 and smaller or equal to 1", s2arr[i], i+1);
    }

  /* For a check:
  {
    gal_data_t *tmp;
    for(tmp=p->marks;tmp!=NULL;tmp=tmp->next)
      printf("%s: %s\n", __func__, tmp->name);
    exit(0);
  } */
}




















/***********************************************************************/
/****************       High-level preparations     ********************/
/***********************************************************************/
void
ui_preparations(struct converttparams *p)
{
  /* Convert the change string into the proper list. */
  if(p->changestr)
    p->change=ui_make_change_struct(p->changestr);

  /* Read the input channels. */
  ui_prepare_input_channels(p);

  /* Read the marks info. */
  if(p->marksname) ui_marks_read(p);

  /* Set the output name. */
  ui_set_output(p);
}



















/**************************************************************/
/************         Set the parameters          *************/
/**************************************************************/

void
ui_read_check_inputs_setup(int argc, char *argv[], struct converttparams *p)
{
  struct gal_options_common_params *cp=&p->cp;


  /* Include the parameters necessary for argp from this program ('args.h')
     and for the common options to all Gnuastro ('commonopts.h'). We want
     to directly put the pointers to the fields in 'p' and 'cp', so we are
     simply including the header here to not have to use long macros in
     those headers which make them hard to read and modify. This also helps
     in having a clean environment: everything in those headers is only
     available within the scope of this function. */
#include <gnuastro-internal/commonopts.h>
#include "args.h"


  /* Initialize the options and necessary information.  */
  ui_initialize_options(p, program_options, gal_commonopts_options);


  /* Read the command-line options and arguments. */
  errno=0;
  if(argp_parse(&thisargp, argc, argv, 0, 0, p))
    error(EXIT_FAILURE, errno, "parsing arguments");


  /* Read the configuration files and set the common values. */
  gal_options_read_config_set(&p->cp);


  /* Read the options into the program's structure, and check them and
     their relations prior to printing. */
  ui_read_check_only_options(p);


  /* Print the option values if asked. Note that this needs to be done
     after the option checks so un-sane values are not printed in the
     output state. */
  gal_options_print_state(&p->cp);


  /* Check that the options and arguments fit well with each other. Note
     that arguments don't go in a configuration file. So this test should
     be done after (possibly) printing the option values. */
  ui_check_options_and_arguments(p);


  /* Read/allocate all the necessary starting arrays. */
  ui_preparations(p);
}




















/**************************************************************/
/************      Free allocated, report         *************/
/**************************************************************/
void
ui_free_report(struct converttparams *p)
{
  if(p->colormap)
    {
      if(p->colormap->next) gal_data_free(p->colormap->next);
      gal_data_free(p->colormap);
    }
  gal_data_free(p->fluxlow);
  gal_data_free(p->fluxhigh);
  gal_list_str_free(p->hdus, 1);
  if(p->cp.output) free(p->cp.output);
  gal_list_str_free(p->inputnames, 0);
}
