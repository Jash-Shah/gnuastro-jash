/*********************************************************************
eps -- functions to write EPS files.
This is part of GNU Astronomy Utilities (Gnuastro) package.

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

#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

#include <gnuastro/box.h>
#include <gnuastro/eps.h>
#include <gnuastro/list.h>
#include <gnuastro/color.h>
#include <gnuastro/blank.h>

#include <gnuastro-internal/timing.h>
#include <gnuastro-internal/checkset.h>






/*************************************************************
 **************      Acceptable EPS names      ***************
 *************************************************************/
int
gal_eps_name_is_eps(char *name)
{
  size_t len;
  if(name)
    {
      len=strlen(name);
      if ( (    len>=3 && strcmp(&name[len-3], "eps") == 0 )
           || ( len>=3 && strcmp(&name[len-3], "EPS") == 0 )
           || ( len>=4 && strcmp(&name[len-4], "epsf") == 0 )
           || ( len>=4 && strcmp(&name[len-4], "epsi") == 0 ) )
        return 1;
      else
        return 0;
    }
  else return 0;
}





int
gal_eps_suffix_is_eps(char *name)
{
  if(name)
    {
      if (strcmp(name, "eps") == 0 || strcmp(name, ".eps") == 0
          || strcmp(name, "EPS") == 0 || strcmp(name, ".EPS") == 0
          || strcmp(name, "epsf") == 0 || strcmp(name, ".epsf") == 0
          || strcmp(name, "epsi") == 0 || strcmp(name, ".epsi") == 0)
        return 1;
      else
        return 0;
    }
  else return 0;
}




















/*************************************************************
 **************       Write an EPS image        **************
 *************************************************************/
static int
eps_is_binary(gal_data_t *in, uint8_t *bitone)
{
  gal_data_t *channel;
  uint8_t second_set=0;
  unsigned char *i, *fi, first=0, second=0;

  /* Go through all the channels. */
  for(channel=in; channel!=NULL; channel=channel->next)
    {
      /* Go through all the values and see if there is more than two values
         in the array. */
      fi = (i=channel->array) + channel->size;
      first=*i;
      do
        if(*i!=first)
          {
            if(second_set)
              { if(*i!=second) break; }
            else
              { second=*i; second_set=1; }
          }
      while(++i<fi);

      /* If we didn't get to the end of the array, then we have a
         multi-valued (not binary) image. */
      if(i!=fi)
        return 0;
    }

  /* If we get to this point, then all the channels were binary, so return
     success. */
  *bitone = first>second ? first : second;
  return 1;
}





/* Convert the channels into into a 0 and 1 bit stream. This function is
   only called when the image is binary (has only two values). NOTE: each
   row has to have an integer number of bytes, so when the number of pixels
   in a row is not a multiple of 8, we'll add one. */
static gal_data_t *
eps_convert_to_bitstream(gal_data_t *in, size_t *numbytes, uint8_t bitone)
{
  size_t i, j, k, bytesinrow;
  gal_data_t *channel, *out=NULL;
  unsigned char *bits, byte, curbit, *arr;
  size_t s0=in->dsize[0], s1=in->dsize[1];

  /* Find the size values and allocate the array. */
  if( s1 % 8 ) bytesinrow = s1/8 + 1;
  else         bytesinrow = s1/8;
  *numbytes = bytesinrow*s0;

  /* Go over all the channels. */
  for(channel=in; channel!=NULL; channel=channel->next)
    {
      /* Allocate the array. Note that we currently don't have an
         allocation system for bits, so we'll allocate space in bytes, then
         convert  */
      gal_list_data_add_alloc(&out, NULL, GAL_TYPE_UINT8, 1, numbytes,
                              NULL, 0, -1, 1, NULL, NULL, NULL);
      out->type=GAL_TYPE_BIT;
      bits=out->array;

      /* Put the values in. */
      arr=channel->array;
      for(i=0;i<s0;++i)       /* i*s0+j is the byte, not bit position. */
        for(j=0;j<bytesinrow;++j)
          {
            /* Set the 8 bits to zero. */
            byte=0;

            /* Current bit position. */
            curbit=0x80;

            /* Write the next 8 values as bits. */
            for(k=0;k<8;++k)
              {
                if( j*8+k < s1 )
                  {
                    if(arr[i*s1+j*8+k]==bitone)
                      byte |= curbit;
                    curbit >>= 1;
                  }
                else break;
              }

            /* Write the byte into the array. */
            bits[i*bytesinrow+j]=byte;
          }
    }

  /* Reverse the list and return it. */
  gal_list_data_reverse(&out);
  return out;
}





static void
eps_write_hex(gal_data_t *write, FILE *fp, size_t numbytes)
{
  unsigned char *arr;
  gal_data_t *channel;
  size_t i=0, j, elem_for_newline=35;

  for(channel=write; channel!=NULL; channel=channel->next)
    {
      if(channel->status)       /* A blank channel has status==1. */
        fprintf(fp, "{<00>} %% Channel %zu is blank\n", i);
      else
        {
          arr=channel->array;
          fprintf(fp, "{<");
          for(j=0;j<numbytes;++j)
            {
              fprintf(fp, "%02X", arr[j]);
              if(j%elem_for_newline==0) fprintf(fp, "\n");
            }
          fprintf(fp, ">}\n");
        }
      ++i;
    }
}





static void
eps_write_ascii85(gal_data_t *write, FILE *fp, size_t numbytes)
{
  unsigned char *arr;
  gal_data_t *channel;
  uint32_t anint, base;
  size_t i=0, j, k, elem_for_newline=15;   /* 15*5=75 */

  for(channel=write; channel!=NULL; channel=channel->next)
    {
      if(channel->status)
        fprintf(fp, "{<00>} %% Channel %zu is blank\n", i);
      else
        {
          arr=channel->array;
          fprintf(fp, "{<~");
          for(j=0;j<numbytes;j+=4)
            {
              /* This is the last four bytes */
              if(numbytes-j<4)
                {
                  anint=arr[j]*256*256*256;
                  if(numbytes-j>1)  anint+=arr[j+1]*256*256;
                  if(numbytes-j==3) anint+=arr[j+2]*256;
                }
              else
                anint=( arr[j]*256*256*256 + arr[j+1]*256*256
                        + arr[j+2]*256     + arr[j+3]         );

              /* If all four bytes are zero, then just print 'z'. */
              if(anint==0) fprintf(fp, "z");
              else
                {
                  /* To check, just change the fprintf below to printf:
                     printf("\n\n");
                     printf("%u %u %u %u\n", in[i], in[i+1],
                            in[i+2], in[i+3]);
                  */
                  base=85*85*85*85;
                  /* Do the ASCII85 encoding: */
                  for(k=0;k<5;++k)
                    {
                      fprintf(fp, "%c", anint/base+33);
                      anint%=base;
                      base/=85;
                    }
                }
              /* Go to the next line if on the right place: */
              if(j%elem_for_newline==0) fprintf(fp, "\n");
            }
          fprintf(fp, "~>}\n");
        }
      ++i;
    }
}





static void
eps_write_image(gal_data_t *in, FILE *fp, int hex, int dontoptimize,
                int forps)
{
  int bpc=8;
  uint8_t bitone;
  gal_data_t *write;
  size_t i, numbytes, *dsize=in->dsize;
  size_t numch=gal_list_data_number(in);

  /* Set the number of bits per component. */
  if( numch==1 && dontoptimize==0 && eps_is_binary(in, &bitone) )
    {
      bpc=1;
      write=eps_convert_to_bitstream(in, &numbytes, bitone);
    }
  else
    {
      write=in;
      numbytes=in->size;
    }

  /* Write the basic meta data. */
  switch(numch)
    {
    case 1: fprintf(fp, "/DeviceGray setcolorspace\n"); break;
    case 3: fprintf(fp, "/DeviceRGB setcolorspace\n");  break;
    case 4: fprintf(fp, "/DeviceCMYK setcolorspace\n"); break;
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! The number of channels (%zu) is "
            "not 1, 3 or 4. Please contact us so we can find the issue and "
            "fix it", __func__, numch);
    }
  fprintf(fp, "<<\n");
  fprintf(fp, "  /ImageType 1\n");
  fprintf(fp, "  /Width %zu\n", dsize[1]);
  fprintf(fp, "  /Height %zu\n", dsize[0]);
  fprintf(fp, "  /ImageMatrix [ %zu 0 0 %zu 0 0 ]\n", dsize[1], dsize[0]);
  fprintf(fp, "  /MultipleDataSources true\n");
  fprintf(fp, "  /BitsPerComponent %d\n", bpc);
  fprintf(fp, "  /Decode[");
  for(i=0;i<numch;++i) {fprintf(fp, " 0 1");} fprintf(fp, " ]\n");
  fprintf(fp, "  /Interpolate false\n");
  fprintf(fp, "  /DataSource [\n");

  /* Based on the encoding, write the contents of the image. */
  if(hex)
    eps_write_hex(write, fp, numbytes);
  else
    eps_write_ascii85(write, fp, numbytes);

  /* Finish the file. */
  fprintf(fp, "  ]\n");
  fprintf(fp, ">>\n");
  fprintf(fp, "image\n\n");

  /* Finish the image part based on the final output. Note that 'grestore'
     will also fix the translation we done on top so the point moves to
     (0,0) before making the next path. */
  if(forps) fprintf(fp, "showpage\n\n");
  else      fprintf(fp, "grestore\n\n");

  /* Clean up. */
  if(write!=in)
    gal_list_data_free(write);
}




/* The "point" is the smallest unit of measure in Postscript (and
   typesetting in general). It is defined like this: 72 points is 1
   international inch. So 72 points is 2.54 cm. */
void
gal_eps_to_pt(float widthincm, size_t *dsize, size_t *w_h_in_pt)
{
  w_h_in_pt[0] = widthincm*72.0f/2.54f;
  w_h_in_pt[1] = (float)( dsize[0] * w_h_in_pt[0] )/(float)(dsize[1]);
}





/* Return the macro corresponding the to the given shape. */
uint8_t
gal_eps_shape_name_to_id(char *n)
{
  if(      !strcasecmp(n, "line")      ) return GAL_EPS_MARK_SHAPE_LINE;
  else if( !strcasecmp(n, "plus")      ) return GAL_EPS_MARK_SHAPE_PLUS;
  else if( !strcasecmp(n, "cross")     ) return GAL_EPS_MARK_SHAPE_CROSS;
  else if( !strcasecmp(n, "point")     ) return GAL_EPS_MARK_SHAPE_POINT;
  else if( !strcasecmp(n, "circle")    ) return GAL_EPS_MARK_SHAPE_CIRCLE;
  else if( !strcasecmp(n, "square")    ) return GAL_EPS_MARK_SHAPE_SQUARE;
  else if( !strcasecmp(n, "ellipse")   ) return GAL_EPS_MARK_SHAPE_ELLIPSE;
  else if( !strcasecmp(n, "rectangle") ) return GAL_EPS_MARK_SHAPE_RECTANGLE;
  else
    error(EXIT_FAILURE, 0, "%s: the shape name '%s' is not recognized. "
          "The currently recognized shapes are: 'point', 'circle', "
          "'square', 'ellipse' or 'rectangle'", __func__, n);

  /* Control should not reach here. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to find "
        "and solve the problem. Control should not reach this part "
        "of the function", __func__, PACKAGE_BUGREPORT);
  return GAL_EPS_MARK_SHAPE_INVALID;
}





/* Convert a shape ID to a name */
char *
gal_eps_shape_id_to_name(uint8_t id)
{
  /* Return the proper name. */
  switch(id)
    {
    case GAL_EPS_MARK_SHAPE_LINE:      return "line";
    case GAL_EPS_MARK_SHAPE_PLUS:      return "plus";
    case GAL_EPS_MARK_SHAPE_CROSS:     return "cross";
    case GAL_EPS_MARK_SHAPE_POINT:     return "point";
    case GAL_EPS_MARK_SHAPE_CIRCLE:    return "circle";
    case GAL_EPS_MARK_SHAPE_SQUARE:    return "square";
    case GAL_EPS_MARK_SHAPE_ELLIPSE:   return "ellipse";
    case GAL_EPS_MARK_SHAPE_RECTANGLE: return "rectangle";
    default:
      error(EXIT_FAILURE, 0, "%s: the shape id '%u' is not recognized. "
            "Please see the 'GAL_EPS_MARK_SHAPE_*' macros in "
            "'gnuastro/eps.h' for the acceptable ids", __func__, id);
    }

  /* Control should not reach here. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to find "
        "and solve the problem. Control should not reach this part "
        "of the function", __func__, PACKAGE_BUGREPORT);
  return GAL_BLANK_STRING;
}





/* Find the center coordinate */
static void
eps_mark_to_pt(float x_pix, float y_pix, float size1_pix,
               float size2_pix, double pix_in_pt,
               uint32_t borderwidth, long ymin, uint8_t shapev,
               float *x_pt, float *y_pt,
               float *size1_pt, float *size2_pt, float *ymin_pt)
{
  /* Necessary shift (in points) because of the coordiante system that has
     been edited due to the image and border width */
  float shiftpt=borderwidth;

  /* Do the conversion: 1) Subtract by half so the bottom-left corners of
     the point and FITS X axis match. 2) Multiply by the pixel-to-point
     conversion scale. 3) account for the shift. */
  *x_pt = (x_pix-0.5) * pix_in_pt + shiftpt;
  *y_pt = (y_pix-0.5) * pix_in_pt + shiftpt;

  /* Other parameters. Note that for an ellipse, the second size paramter
     is the axis ratio. */
  *ymin_pt  = ymin      * pix_in_pt;
  *size1_pt = size1_pix * pix_in_pt;
  *size2_pt = ( shapev==GAL_EPS_MARK_SHAPE_ELLIPSE
                ? size2_pix
                : size2_pix * pix_in_pt );
}





/* Low-level function to draw the shape in the corrected coordinate system
   (after translation and rotation). */
static void
eps_mark_draw_shape(FILE *fp, uint8_t shape, float s1, float s2)
{
  /* Draw the shape. */
  switch(shape)
    {
    case GAL_EPS_MARK_SHAPE_LINE:
      fprintf(fp, "%f 0  moveto %f 0 lineto \n", -s1/2, s1/2);
      break;

    case GAL_EPS_MARK_SHAPE_PLUS:
      fprintf(fp, "%f 0  moveto %f 0 lineto \n", -s1/2, s1/2);
      fprintf(fp, "0  %f moveto 0 %f lineto closepath\n", -s1/2, s1/2);
      break;

    case GAL_EPS_MARK_SHAPE_CROSS:
      fprintf(fp, "%f %f moveto %f %f lineto \n", -s1/M_SQRT2/2,
              -s1/M_SQRT2/2, s1/M_SQRT2/2, s1/M_SQRT2/2);
      fprintf(fp, "%f %f moveto %f %f lineto \n", -s1/M_SQRT2/2,
              s1/M_SQRT2/2, s1/M_SQRT2/2, -s1/M_SQRT2/2);
      break;

    case GAL_EPS_MARK_SHAPE_POINT:
      fprintf(fp, "newpath 0 0 %f 0 360 arc fill closepath\n", s1);
      break;

    case GAL_EPS_MARK_SHAPE_CIRCLE:
      fprintf(fp, "newpath 0 0 %f 0 360 arc closepath\n", s1);
      break;

    case GAL_EPS_MARK_SHAPE_ELLIPSE:
      fprintf(fp, "newpath 0 0 %f %f 0 360 ellipse\n", s1, s1*s2);
      break;

    case GAL_EPS_MARK_SHAPE_SQUARE:
      fprintf(fp, "newpath -%.2f -%.2f %.2f %.2f rectstroke\n",
              s1/2, s1/2, s1, s1);
      break;

    case GAL_EPS_MARK_SHAPE_RECTANGLE:
      fprintf(fp, "newpath -%.2f -%.2f %.2f %.2f rectstroke\n",
              s1/2, s2/2, s1, s2);
      break;

    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "fix the problem. The code '%d' is not recognized for the "
            "'shape' variable", __func__, PACKAGE_BUGREPORT, shape);
    }
}





/* Insert the part about the mark shape. */
static void
eps_mark_add_shape(FILE *fp, size_t index, float x, float y, float s1,
                   float s2, float linewidth, uint8_t color, uint8_t shape,
                   float rotate, char *text, float textsize, char *textfont,
                   float yminpt)
{
  float rgb[3];

  /* If the rotation angle is negative, add it with 360 so it becomes
     positive. */
  if(rotate<0.0) rotate+=360.0;

  /* Print a comment, identifying that this is a mark. */
  fprintf(fp, "%% Mark %zu (a %s):\n", index+1,
          gal_eps_shape_id_to_name(shape));

  /* If the shape is a circle, or the user gave a rotation angle of zero,
     then rotation is irrelevant, so set it to NaN, so we don't even write
     it into the file. */
  if(shape==GAL_EPS_MARK_SHAPE_CIRCLE || rotate==0.0) rotate=NAN;

  /* If 'linewidth' isn't blank, then we need to set it for each mark. */
  if( !isnan(linewidth) )
    fprintf(fp, "%f setlinewidth\n", linewidth);

  /* Set the color (if it hasn't already been set. */
  if( color != GAL_BLANK_UINT8 )
    {
      gal_color_in_rgb(color, rgb);
      fprintf(fp, "%.2f %.2f %.2f setrgbcolor\n", rgb[0], rgb[1], rgb[2]);
    }

  /* Move the coordinate to the desired central coordinate and do the
     rotation if requested. */
  fprintf(fp, "%f %f translate\n", x, y);
  if( !isnan(rotate) ) fprintf(fp, "%f rotate\n", rotate);

  /* Draw the shape. */
  eps_mark_draw_shape(fp, shape, s1, s2);

  /* Correct the rotation. */
  if( !isnan(rotate) )
    fprintf(fp, "-%f rotate\n", rotate);
  fprintf(fp, "-%f -%f translate\n", x, y);

  /* Finish the stroke. */
  fprintf(fp, "stroke\n\n");

  /* Write the text (if 'text' isn't NULL, and it isn't a blank string. */
  if( text && strcmp(text, GAL_BLANK_STRING) )
    {
      fprintf(fp, "%% Text of the mark above\n");
      fprintf(fp, "(%s) %g /%s %g %g centertoptext\n\n", text,
              textsize, textfont, x, yminpt);
    }
}




/* Simplify the checking of various columns. */
static void *
eps_mark_prepare_col(gal_data_t *marks, char *name, uint8_t type,
                     int abort, char *extrainfo)
{
  gal_data_t *tmp=gal_list_data_select_by_name(marks, name);

  /* If a list element was found, check if it has the correct type. */
  if(tmp)
    {
      if(tmp->type!=type)
        error(EXIT_FAILURE, 0, "%s: the '%s' column should have "
              "a %s numeric data type%s", __func__, name,
              gal_type_name(type, 1), extrainfo);
      return tmp->array;
    }
  else
    {
      if(abort)
        error(EXIT_FAILURE, 0, "%s: no column with name '%s' was found "
              "in the list of mark metadata", __func__, name);
    }

  /* No list element with the desired name was found. */
  return NULL;
}





/* Make sure the inputs are in the desired format and return the usable
   arrays for the main function to add marks. */
static void
eps_mark_prepare(gal_data_t *marks, float **x, float **y, uint8_t **shape,
                 uint8_t **color, float **size1, float **size2,
                 float **lwidth, float **rotate, char ***text,
                 char ***font, float **fontsize)
{
  size_t i;
  gal_data_t *tmp;

  /* Make sure all the columns given to 'marks' have the same number
     elements. */
  for(tmp=marks;tmp!=NULL;tmp=tmp->next)
    {
      if(tmp->size!=marks->size)
        error(EXIT_FAILURE, 0, "%s: the mark column '%s' has "
              "a different number of rows, or elements (%zu), than "
              "the first (named '%s' with %zu rows)", __func__,
              tmp->name, tmp->size, marks->name, marks->size);
    }

  /* Check each column. */
  *x=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_XPIX,
                          GAL_TYPE_FLOAT32, 1, "");
  *y=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_YPIX,
                          GAL_TYPE_FLOAT32, 1, "");
  *text=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_TEXT,
                             GAL_TYPE_STRING, 0, "");
  *font=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_FONT,
                             GAL_TYPE_STRING, 0, "");
  *size1=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_SIZE1,
                             GAL_TYPE_FLOAT32, 0, "");
  *size2=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_SIZE2,
                             GAL_TYPE_FLOAT32, 0, "");
  *rotate=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_ROTATE,
                               GAL_TYPE_FLOAT32, 0, "");
  *lwidth=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_LINEWIDTH,
                               GAL_TYPE_FLOAT32, 0, "");
  *fontsize=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_FONTSIZE,
                                 GAL_TYPE_FLOAT32, 0, "");
  *shape=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_SHAPE,
                              GAL_TYPE_UINT8, 0, ". Note that the macros "
                              "containing shape identifiers have the "
                              "'GAL_EPS_MARK_SHAPE-' prefix and are "
                              "defined in 'gnuastro/eps.h'");
  *color=eps_mark_prepare_col(marks, GAL_EPS_MARK_COLNAME_COLOR,
                              GAL_TYPE_UINT8, 0, ". Note that the macros "
                              "containing color identifiers have the "
                              "'GAL_COLOR_' prefix and are "
                              "defined in 'gnuastro/color.h'");

  /* Some sanity checks for the columns that need one. */
  if( *rotate || *shape )
    for(i=0;i<marks->size;++i)
      {
        /* Rotation angle should be between 0 to 360. */
        if( *rotate && ((*rotate)[i]<-360 || (*rotate)[i]>360 ) )
          error(EXIT_FAILURE, 0, "%s: %g is not a valid rotation angle "
                "(in degrees). It belongs to mark number %zu (counting "
                "from 1)", __func__, (*rotate)[i], i+1);

        /* For an ellipse, the 'size2' column should be positive and
           smaller than one. */
        if( (*shape)[i]==GAL_EPS_MARK_SHAPE_ELLIPSE
            && *size2
            && ( (*size2)[i]<0 || (*size2)[i]>1 ) )
          error(EXIT_FAILURE, 0, "%s: %g is not a valid 'size2' column "
                "for an ellipse shape (from mark number %zu, counting "
                "from 1). For an ellipse, the 'size2' column is the"
                "axis ratio, so it should always be between 0 and 1",
                __func__, (*size2)[i], i+1);
      }
}





/* Write the default mark properties (like linewidth, color and etc). This
   can happen if they aren't given by the user, or that all the values
   given by the user are the same. */
static void
eps_mark_add_defaults(FILE *fp, gal_data_t *marks, uint8_t **color_o,
                      float **lwidth_o, uint8_t *shape, char **text)
{
  size_t i;
  float rgb[3];
  uint8_t *color=*color_o;
  float *lwidth=*lwidth_o;
  int samelwidth=1, samecolor=1;

  /* Parse all the information */
  for(i=1;i<marks->size;++i)
    {
      if(samecolor==1  && color  && color[i]!=color[0]  ) samecolor=0;
      if(samelwidth==1 && lwidth && lwidth[i]!=lwidth[0]) samelwidth=0;
    }

  /* Default color */
  if(samecolor)
    {
      fprintf(fp, "%% Same color for all marks:\n");
      gal_color_in_rgb(color ? color[0] : GAL_EPS_MARK_DEFAULT_COLOR, rgb);
      fprintf(fp, "%.2f %.2f %.2f setrgbcolor\n\n", rgb[0], rgb[1], rgb[2]);
    }

  /* Default line-width. */
  if(samelwidth)
    {
      fprintf(fp, "%% Same line width for all marks:\n");
      fprintf(fp, "%f setlinewidth\n\n",
              lwidth ? lwidth[0] : GAL_EPS_MARK_DEFAULT_LINEWIDTH);
      *lwidth_o=NULL;
    }

  /* If an ellipse has been requested, define the function (this function
     has been inspired from http://www.redgrittybrick.org/ellipse.html */
  if(shape)
    {
      for(i=0;i<marks->size;++i)
        if(shape[i]==GAL_EPS_MARK_SHAPE_ELLIPSE)
          {
            fprintf(fp, "%% Function for ellipse shape:\n");
            fprintf(fp, "/ellipse {\n");
            fprintf(fp, "    /endangle exch def\n");
            fprintf(fp, "    /startangle exch def\n");
            fprintf(fp, "    /yrad exch def\n");
            fprintf(fp, "    /xrad exch def\n");
            fprintf(fp, "    /y exch def\n");
            fprintf(fp, "    /x exch def\n");
            fprintf(fp, "    /savematrix matrix currentmatrix def\n");
            fprintf(fp, "    x y translate\n");
            fprintf(fp, "    xrad yrad scale\n");
            fprintf(fp, "    0 0 1 startangle endangle arc\n");
            fprintf(fp, "    savematrix setmatrix\n");
            fprintf(fp, "} def\n\n");
            break;
          }
    }

  /* In case we have text, use this function to put them such that they the
     given coordinate is at the top of the string. This was inspired from
     [1] (the main change is that '-2 div' (for 'dy') has been set to '-1
     mul' so the given coordinate is at the top. It should be called with
     the following format (just set the upper-case words).

         (STRING) FONTSIZE FONTNAME X Y centertoptext

     [1] https://stackoverflow.com/questions/3618194/how-to-determine-string-height-in-postscript*/
  if(text)
    {
      fprintf(fp, "%% Print text with coordinate at center-top:\n");
      fprintf(fp, "/centertoptext {\n");

      /* Save the current state. */
      fprintf(fp, "  gsave\n");

      /* Move to the top two popped operands (X and Y), and set the fonts. */
      fprintf(fp, "   moveto findfont exch scalefont setfont\n");

      /* Save the state. */
      fprintf(fp, "   gsave\n");

      /* 1. dup: Duplicate the string.
         2. charpath: move to where the string finishes.
         3. flattenpath: account for curves.
         4. pathbox: put four numbers on the stack:
             x0: bottom-left point's X axis position.
             y0: bottom-left point's Y axis position.
             x1: top-right point's X axis position.
             y1: top-right point's Y axis position. */
      fprintf(fp, "    dup false charpath flattenpath pathbbox\n");

      /* Go back to the previous settings (before charpath changed it). */
      fprintf(fp, "   grestore\n");

      /* 3 -1 roll: permutes the top three: y0 x1 y1 ---> x1 y1 y0 */
      fprintf(fp, "   3 -1 roll\n");

      /* Subtract the second popped operand from the first (at the top, we
         now have the height of the text's box or 'dy'. Then multiply this
         by -1, giving us '-dy' (which is the distance we want to shift
         vertically/downwards before "showing" the string. */
      fprintf(fp, "   sub -1 mul\n");

      /* 3 1 roll: permute the top three points: x0 x1 dy --> dy x0 x1 */
      fprintf(fp, "   3 1 roll\n");

      /* Subtract x0 from x1 get the negative 'dx' on top of the stack.
         Then divide it by two, so we have '-dx/2' (we need to shift the
         word by the amount along the horizintal/left-ward before "show"ing
         it).*/
      fprintf(fp, "   sub 2 div\n");

      /* Exchange the top operands so they are in this order: '-dx -dy'. */
      fprintf(fp, "   exch \n");

      /* Move (relatively) by the given displacement and print the string
         (which has stayed at the bottom of the stack until now) and print
         the string.. */
      fprintf(fp, "   rmoveto show\n");

      /* Restore the graphical environment for next paths. */
      fprintf(fp, "  grestore\n");
      fprintf(fp, "} bind def\n\n");
    }
}





/* Set the second size parameter (depending on a shape being defined or
   not). */
static float
eps_mark_size2(uint8_t *shape, float *size2arr, size_t i)
{
  if(size2arr) return size2arr[i];
  else
    {
      if(shape)
        switch(shape[i])
          {
          case GAL_EPS_MARK_SHAPE_ELLIPSE:
            return GAL_EPS_MARK_DEFAULT_SIZE2_ELLIPSE;
          default: return GAL_EPS_MARK_DEFAULT_SIZE2;
          }
      else
        return GAL_EPS_MARK_DEFAULT_SIZE2;
    }
  return NAN;
}





/* Fill the 'fpixel' and 'lpixel' arrays for the shapes that may be
   affected by rotation. */
static void
eps_mark_in_img_fl_pixel(float x, float y, float s1, float s2,
                         uint8_t shape, long *fpixel, long *lpixel)
{
  long width[2];
  double center[2]={x, y};

  /* Depending the shape, set the four corners in relation to the shape's
     center (these will later be added to the X,Y of the center). */
  switch(shape)
    {
    case GAL_EPS_MARK_SHAPE_LINE:
      width[0]=s1; width[1]=0.1; break; /* We will add line-width later, */
                                        /* And width[1] shouldn't be 0.  */
    case GAL_EPS_MARK_SHAPE_PLUS:
    case GAL_EPS_MARK_SHAPE_SQUARE:
      width[0]=width[1]=s1; break;

    case GAL_EPS_MARK_SHAPE_POINT:
    case GAL_EPS_MARK_SHAPE_CIRCLE:
      width[0]=width[1]=s1*2; break; /* s1 is radius: half of width */

    case GAL_EPS_MARK_SHAPE_CROSS:
      width[0]=width[1]=M_SQRT2*s1; break;

    case GAL_EPS_MARK_SHAPE_RECTANGLE:
      width[0]=s1; width[1]=s2;
      break;

    case GAL_EPS_MARK_SHAPE_ELLIPSE:
      width[0]=s1*2; width[1]=s1*s2*2; /* s1: half major axis. */
      break;                           /* s2: axis ratio. */

    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "find and fix the problem. The code '%u' isn't recognized "
            "for the 'shape' variable", __func__, PACKAGE_BUGREPORT,
            shape);
    }

  /* Find the before-rotation fpixel and lpixel */
  gal_box_border_from_center(center, 2, width, fpixel, lpixel);

  /* For a check:
  printf("%s:center: %g, %g\n",   __func__, center[0], center[1]);
  printf("%s:width:  %ld, %ld\n", __func__, width[0],  width[1]);
  printf("%s:fpixel: %ld, %ld\n", __func__, fpixel[0], fpixel[1]);
  printf("%s:lpixel: %ld, %ld\n", __func__, lpixel[0], lpixel[1]);
  */
}





/* See if the given shape falls within the image or not. Currently, this
   doesn't account for rotation due to time limits at development time. */
static int
eps_mark_in_img(size_t *dsize, size_t i, float x, float y,
                float *size1arr, float *size2arr,
                uint8_t *shapearr, float *rotarr, long *ymin)
{
  long naxes[2]={dsize[1], dsize[0]};
  float s2 = eps_mark_size2(shapearr, size2arr, i);
  long fpixel_i[2], lpixel_i[2], fpixel_o[2], lpixel_o[2];
  uint8_t shape = shapearr ? shapearr[i] : GAL_EPS_MARK_DEFAULT_SHAPE;
  float   s1    = size1arr ? size1arr[i] : GAL_EPS_MARK_DEFAULT_SIZE1;
  float   rot   = rotarr   ? rotarr[i]   : GAL_EPS_MARK_DEFAULT_ROTATE;

  /* Find the first and last pixels of the marks based on its shape. For a
     point, things are easy, but for others, we have a separate
     function. */
  eps_mark_in_img_fl_pixel(x, y, s1, s2, shape, fpixel_i, lpixel_i);

  /* Update the box's first and last pixels based on the rotation. This is
     not necessary if we don't have any rotation or we are dealing with a
     circle shape. */
  if( rot!=0.0
      && shape!=GAL_EPS_MARK_SHAPE_POINT
      && shape!=GAL_EPS_MARK_SHAPE_CIRCLE )
    gal_box_border_rotate_around_center(fpixel_i, lpixel_i, 2, rot);

  /* Set the smallest vertical position of the box. */
  *ymin=fpixel_i[1];

  /* See if there is any overlap */
  return gal_box_overlap(naxes, fpixel_i, lpixel_i,
                         fpixel_o, lpixel_o, 2);
}





/* Add markers on each desired coordinate. */
static void
eps_mark_add(gal_data_t *in, gal_data_t *marks, FILE *fp,
             size_t *w_h_in_pt, uint32_t borderwidth)
{
  size_t i;
  long ymin;
  float lwidthv;
  char **text=NULL, **font=NULL;
  uint8_t shapev, *shape=NULL, *color=NULL;
  float s1, s2, *size1=NULL, *size2=NULL;
  float yminpt, *rotate=NULL, *fontsize=NULL;
  float x, y, *xarr=NULL, *yarr=NULL, *lwidth=NULL;

  /* The size of one pixel in "points". */
  double pix_in_pt=(double)(w_h_in_pt[0])/(double)(in->dsize[1]);

  /* Load the pointers to the various metdata. */
  eps_mark_prepare(marks, &xarr, &yarr, &shape, &color,
                   &size1, &size2, &lwidth, &rotate, &text,
                   &font, &fontsize);

  /* If the mark parameters are similar, just write them once (instead of
     repeating for each mark, and set their pointers to 'NULL' so the rest
     of the function doesn't even bother with them). */
  eps_mark_add_defaults(fp, marks, &color, &lwidth, shape, text);

  /* Write the mark into the EPS file. But only when the requested region
     falls within the image (we don't want to waste storage) */
  for(i=0;i<marks->size;++i)
    if( eps_mark_in_img(in->dsize, i, xarr[i], yarr[i], size1, size2,
                        shape, rotate, &ymin) )
      {
        lwidthv=lwidth ? lwidth[i] : GAL_EPS_MARK_DEFAULT_LINEWIDTH;
        shapev=shape ? shape[i] : GAL_EPS_MARK_DEFAULT_SHAPE,
        eps_mark_to_pt(xarr[i], yarr[i],
                       size1 ? size1[i] : GAL_EPS_MARK_DEFAULT_SIZE1,
                       eps_mark_size2(shape, size2, i),
                       pix_in_pt, borderwidth, ymin, shapev,
                       &x, &y, &s1, &s2, &yminpt);
        eps_mark_add_shape(fp, i, x, y, s1, s2, lwidthv,
                           color    ? color[i]    : GAL_BLANK_UINT8,
                           shapev,
                           rotate   ? rotate[i]   : GAL_BLANK_FLOAT32,
                           text     ? text[i]     : NULL,
                           fontsize ? fontsize[i] : GAL_EPS_MARK_DEFAULT_FONTSIZE,
                           font     ? font[i]     : GAL_EPS_MARK_DEFAULT_FONT,
                           yminpt - lwidthv/2);
      }
}





void
gal_eps_write(gal_data_t *in, char *filename, float widthincm,
              uint32_t borderwidth, int hex, int dontoptimize,
              int forps, gal_data_t *marks)
{
  FILE *fp;
  float hbw;
  time_t rawtime;
  size_t numch=gal_list_data_number(in);
  size_t w_h_in_pt[2], *dsize=in->dsize;

  /* Sanity checks. */
  if(numch==2 || numch>4)
    error(EXIT_FAILURE, 0, "%s: only 1, 3, and 4 color channels are "
          "acceptable, input is a list of %zu data sets", __func__, numch);
  if(in->type!=GAL_TYPE_UINT8)
    error(EXIT_FAILURE, 0, "%s: input has a '%s' type, but JPEG images "
          "can only have a 'uint8' type", __func__,
          gal_type_name(in->type, 1));


  /* Read the time to write in the output. */
  time(&rawtime);


  /* Find the bounding box  */
  hbw=(float)borderwidth/2.0;
  gal_eps_to_pt(widthincm, dsize, w_h_in_pt);


  /* Open the output file and write the top comments. */
  errno=0;
  fp=fopen(filename, "w");
  if(fp==NULL)
    error(EXIT_FAILURE, errno, "%s", filename);
  fprintf(fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(fp, "%%%%BoundingBox: 0 0 %zu %zu\n", w_h_in_pt[0]+2*borderwidth,
          w_h_in_pt[1]+2*borderwidth);
  fprintf(fp, "%%%%Creator: %s\n", PACKAGE_STRING);
  fprintf(fp, "%%%%CreationDate: %s", ctime(&rawtime));
  fprintf(fp, "%%%%LanuageLevel: 3\n");
  fprintf(fp, "%%%%EndComments\n\n");
  if(forps==0)
    fprintf(fp, "gsave\n\n");


  /* Write the image: */
  fprintf(fp, "%% Draw the image:\n");
  fprintf(fp, "%d %d translate\n", borderwidth, borderwidth);
  fprintf(fp, "%zu %zu scale\n", w_h_in_pt[0], w_h_in_pt[1]);
  eps_write_image(in, fp, hex, dontoptimize, forps);


  /* Mark parts of the image if necessary. */
  if(marks)
    eps_mark_add(in, marks, fp, w_h_in_pt, borderwidth);


    /* Commands to draw the border: */
  if(borderwidth)
    {
      fprintf(fp, "%% Draw the border:\n");
      fprintf(fp, "0 setgray\n");
      fprintf(fp, "%d setlinewidth\n", borderwidth);
      fprintf(fp, "%.1f %.1f moveto\n", hbw, hbw);
      fprintf(fp, "0 %zu rlineto\n", w_h_in_pt[1]+borderwidth);
      fprintf(fp, "%zu 0 rlineto\n", w_h_in_pt[0]+borderwidth);
      fprintf(fp, "0 -%zu rlineto\n", w_h_in_pt[1]+borderwidth);
      fprintf(fp, "closepath\n");
      fprintf(fp, "stroke\n\n");
    }


  /* Ending of the EPS file: */
  fprintf(fp, "%%%%EOF");
  fclose(fp);
}
