/* tab:4
 *
 * photo.c - photo display functions
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:      Steve Lumetta
 * Version:     3
 * Creation Date:   Fri Sep  9 21:44:10 2011
 * Filename:        photo.c
 * History:
 *  SL  1   Fri Sep  9 21:44:10 2011
 *      First written (based on mazegame code).
 *  SL  2   Sun Sep 11 14:57:59 2011
 *      Completed initial implementation of functions.
 *  SL  3   Wed Sep 14 21:49:44 2011
 *      Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"


/* types local to this file (declared in types.h) */

/* 
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as 
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;         /* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/* 
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have 
 * been stored as 2:2:2-bit RGB values (one byte each), including 
 * transparent pixels (value OBJ_CLR_TRANSP).  As with the room photos, 
 * pixel data are stored as one-byte values starting from the upper 
 * left and traversing the top row before returning to the left of the 
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;         /* defines height and width */
    uint8_t*       img;                 /* pixel data               */
};



/* file-scope variables */

/* 
 * The room currently shown on the screen.  This value is not known to 
 * the mode X code, but is needed when filling buffers in callbacks from 
 * that code (fill_horiz_buffer/fill_vert_buffer).  The value is set 
 * by calling prep_room.
 */
static const room_t* cur_room = NULL; 

//struct for level 4
struct octree_node_level4 {
        uint16_t    idx_original;
        uint16_t    idx_level_2;
        unsigned long int   red_sum;
        unsigned long int   green_sum;
        unsigned long int   blue_sum;
        unsigned int pixel_number;
        uint16_t    palette_idx;
        unsigned long int   red_average;
        unsigned long int   green_average;
        unsigned long int   blue_average;
};

//struct for level 2
struct octree_node_level2 {
        unsigned long int   red_sum;
        unsigned long int   green_sum;
        unsigned long int   blue_sum;
        unsigned int pixel_number;
        uint16_t    palette_idx;
        unsigned long int   red_average;
        unsigned long int   green_average;
        unsigned long int   blue_average;
};
    
    
/* 
 * fill_horiz_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the leftmost 
 *                pixel of a line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- leftmost pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */ 
    int            yoff;  /* y offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ?
            view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
         obj = obj_next (obj)) {
    obj_x = obj_get_x (obj);
    obj_y = obj_get_y (obj);
    img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
    if (y < obj_y || y >= obj_y + img->hdr.height ||
        x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
        continue;
    }

    /* The y offset of drawing is fixed. */
    yoff = (y - obj_y) * img->hdr.width;

    /* 
     * The x offsets depend on whether the object starts to the left
     * or to the right of the starting point for the line being drawn.
     */
    if (x <= obj_x) {
        idx = obj_x - x;
        imgx = 0;
    } else {
        idx = 0;
        imgx = x - obj_x;
    }

    /* Copy the object's pixel data. */
    for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
        pixel = img->img[yoff + imgx];

        /* Don't copy transparent pixels. */
        if (OBJ_CLR_TRANSP != pixel) {
        buf[idx] = pixel;
        }
    }
    }
}


/* 
 * fill_vert_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the top pixel of 
 *                a vertical line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- top pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */ 
    int            xoff;  /* x offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ?
            view->img[view->hdr.width * (y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
         obj = obj_next (obj)) {
    obj_x = obj_get_x (obj);
    obj_y = obj_get_y (obj);
    img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
    if (x < obj_x || x >= obj_x + img->hdr.width ||
        y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
        continue;
    }

    /* The x offset of drawing is fixed. */
    xoff = x - obj_x;

    /* 
     * The y offsets depend on whether the object starts below or 
     * above the starting point for the line being drawn.
     */
    if (y <= obj_y) {
        idx = obj_y - y;
        imgy = 0;
    } else {
        idx = 0;
        imgy = y - obj_y;
    }

    /* Copy the object's pixel data. */
    for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
        pixel = img->img[xoff + img->hdr.width * imgy];

        /* Don't copy transparent pixels. */
        if (OBJ_CLR_TRANSP != pixel) {
        buf[idx] = pixel;
        }
    }
    }
}


/* 
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_height (const image_t* im)
{
    return im->hdr.height;
}


/* 
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_width (const image_t* im)
{
    return im->hdr.width;
}

/* 
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_height (const photo_t* p)
{
    return p->hdr.height;
}


/* 
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_width (const photo_t* p)
{
    return p->hdr.width;
}


/* 
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void
prep_room (const room_t* r)
{
    /* Record the current room. */
    photo_t *p = room_photo(r);
    cur_room = r;
    fill_palette(p->palette);
}


/* 
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t*
read_obj_image (const char* fname)
{
    FILE*    in;        /* input file               */
    image_t* img = NULL;    /* image structure          */
    uint16_t x;         /* index over image columns */
    uint16_t y;         /* index over image rows    */
    uint8_t  pixel;     /* one pixel from the file  */

    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
    NULL == (img = malloc (sizeof (*img))) ||
    NULL != (img->img = NULL) || /* false clause for initialization */
    1 != fread (&img->hdr, sizeof (img->hdr), 1, in) ||
    MAX_OBJECT_WIDTH < img->hdr.width ||
    MAX_OBJECT_HEIGHT < img->hdr.height ||
    NULL == (img->img = malloc 
         (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
    if (NULL != img) {
        if (NULL != img->img) {
            free (img->img);
        }
        free (img);
    }
    if (NULL != in) {
        (void)fclose (in);
    }
    return NULL;
    }

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

    /* Loop over columns from left to right. */
    for (x = 0; img->hdr.width > x; x++) {

        /* 
         * Try to read one 8-bit pixel.  On failure, clean up and 
         * return NULL.
         */
        if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
        free (img->img);
        free (img);
            (void)fclose (in);
        return NULL;
        }

        /* Store the pixel in the image data. */
        img->img[img->hdr.width * y + x] = pixel;
    }
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return img;
}


/* 
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t*
read_photo (const char* fname)
{
    FILE*    in;    /* input file               */
    photo_t* p = NULL;  /* photo structure          */
    uint16_t x;     /* index over image columns */
    uint16_t y;     /* index over image rows    */
    uint16_t pixel; /* one pixel from the file  */
     struct octree_node_level2 level_2[level_2_size];    //8^2 nodes
    struct octree_node_level4 level_4[level_4_size];    //8^4 nodes
    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
    NULL == (p = malloc (sizeof (*p))) ||
    NULL != (p->img = NULL) || /* false clause for initialization */
    1 != fread (&p->hdr, sizeof (p->hdr), 1, in) ||
    MAX_PHOTO_WIDTH < p->hdr.width ||
    MAX_PHOTO_HEIGHT < p->hdr.height ||
    NULL == (p->img = malloc 
         (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
    if (NULL != p) {
        if (NULL != p->img) {
            free (p->img);
        }
        free (p);
    }
    if (NULL != in) {
        (void)fclose (in);
    }
    return NULL;
    }
    int position[level_4_size];
    uint32_t    i; 
    uint16_t    pixels_array[p->hdr.width * p->hdr.height]; //store all pixels
    uint32_t temp;
    
    //intialize level_4 and the array stores the index before the sort
    for(i = 0; i < level_4_size; ++i)
    {
        level_4[i].idx_original = i;
        level_4[i].idx_level_2 = init_100;
        level_4[i].red_sum = 0;
        level_4[i].green_sum = 0;
        level_4[i].blue_sum = 0;
        level_4[i].pixel_number = 0;
        level_4[i].palette_idx = init_neg1;
        position[i] = init_neg1;
    }
    
    //intialize level_2
    for(i = 0; i < level_2_size; ++i)
    {
        level_2[i].red_sum = 0;
        level_2[i].green_sum = 0;
        level_2[i].blue_sum = 0;
        level_2[i].pixel_number = 0;
        level_2[i].palette_idx = init_neg1;
        
    }
    
    /*first loop over file: map all the pixels into leverl4 array 
     *and record the number of pixels in each node, also records their sum of RGB
     *this loop also put the file into the pixels array*/
    
    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) 
    {
        /* Loop over columns from left to right. */
        for (x = 0; p->hdr.width > x; x++) 
        {
        /* 
         * Try to read one 16-bit pixel.  On failure, clean up and 
         * return NULL.
         */
            if (1 != fread (&pixel, sizeof (pixel), 1, in)) 
            {
                free (p->img);
                free (p);
                (void)fclose (in);
                return NULL;
            }
        
        i = map_to_octree (pixel, rep_level_4); //find the index in level 4
        level_4[i].idx_level_2 = map_to_octree(pixel, rep_level_2); //find the index in level 2
        pixels_array[p->hdr.width * y + x] = pixel; //stores every pixel
        temp = pixel >> shift_11;//for calculating the sum of red
        level_4[i].red_sum += temp & mask_1f;
        temp=pixel >> shift_5; //for calculating the sum of green
        level_4[i].green_sum += temp& mask_3f;
        level_4[i].blue_sum += pixel & mask_1f; //calculate the sum of blue
        level_4[i].idx_original = i; //the current index in level 4
        level_4[i].pixel_number++; //count the # of pixels for current node
        }
    }
    
    /* no need for the file anymore */
    (void)fclose (in);
        qsort(level_4, level_4_size, sizeof(struct octree_node_level4), qsort_helper); //sort level 4 to get the first 128 colors
    
    
    
    for(i = 0; i < first_128; i++)//calculate the average rgb 
    {
        //if there is no pixel, draw black; otherwise, draw average rgb
        level_4[i].red_average = (level_4[i].pixel_number==0)?0:level_4[i].red_sum / level_4[i].pixel_number;
        level_4[i].green_average = (level_4[i].pixel_number==0)?0:level_4[i].green_sum / level_4[i].pixel_number;
        level_4[i].blue_average =  (level_4[i].pixel_number==0)?0:level_4[i].blue_sum / level_4[i].pixel_number;
        level_4[i].palette_idx = old_64 + i;
        position[level_4[i].idx_original] = i;
    }
    
    //calculate the sum of rgb for each node after the first 128 in level 4
    for(i = first_128; i < level_4_size; i++)
    {
        if(level_4[i].idx_level_2 < level_2_size) //find the rgb of the next 64 colors
        {
            level_2[level_4[i].idx_level_2].red_sum += level_4[i].red_sum;
            level_2[level_4[i].idx_level_2].green_sum += level_4[i].green_sum;
            level_2[level_4[i].idx_level_2].blue_sum += level_4[i].blue_sum;
            level_2[level_4[i].idx_level_2].pixel_number += level_4[i].pixel_number;
        }
        position[level_4[i].idx_original] = i; //otherwise, update the position
    }
        
     for(i = 0; i < level_2_size; i++) //calculate the avg rgb for the next 64 colors
    {
        //if there is no pixel, draw black; otherwise, draw average rgb
        level_2[i].red_average = (level_2[i].pixel_number==0)?0:level_2[i].red_sum / level_2[i].pixel_number;
        level_2[i].green_average = (level_2[i].pixel_number==0)?0:level_2[i].green_sum / level_2[i].pixel_number;
        level_2[i].blue_average =  (level_2[i].pixel_number==0)?0:level_2[i].blue_sum / level_2[i].pixel_number;
        level_2[i].palette_idx = old_64 + first_128 + i; 
    }

    for(i = 0; i < level_2_size; i++){ //fill the palette for the second 64 color
        p->palette[i+first_128][0] = (level_2[i].red_average & 0x1F) << 1;
        p->palette[i+first_128][1] = level_2[i].green_average & 0x3F;
        p->palette[i+first_128][2] = (level_2[i].blue_average & 0x1F) << 1;
    }

    for(i = 0; i < first_128; i++){ //fill the pelette for the 128 colors
        p->palette[i][0] = (level_4[i].red_average & 0x1F) << 1;
        p->palette[i][1] = level_4[i].green_average & 0x3F;
        p->palette[i][2] = (level_4[i].blue_average & 0x1F) << 1;
    }

    //find the 128 colors' palette_idx in level 2
    for(i = first_128; i < level_4_size; i++)
    {
        if(level_4[i].idx_level_2 < level_2_size)
        {
            level_4[i].palette_idx = level_2[level_4[i].idx_level_2].palette_idx; 
        }       
    }
    
    //fill palette for the image
    for(i = 0; i < p->hdr.width * p->hdr.height; i++)
     {
            p->img[i] = level_4[position[map_to_octree(pixels_array[i], rep_level_4)]].palette_idx;
     }
    
    return p;
}


/*
 *map_to_octree
 *Description: helper function that convert the 16 bit RGB value to map to level 2 or level 4 nodes
 *              only takes the first x bits from each R,G,B and get them togeter
 *Input: pixel: the 16 bit pixel (5:6:5)  of RGB
 *       level_number: 2 or 4, indicate which level of octree does the pixel map to
 *Output: None
 *Return Value: the position in array_2 or array_4, for array_2, it only occupies the last 6 bits
          for array_4, it occupies the last 12 bits
 *Side Effects: None
 */
uint16_t map_to_octree (const uint16_t pixel, const uint8_t level_number)
{
    if(level_number != rep_level_2 && level_number != rep_level_4)
        return -1;
    uint16_t pixel_copy = pixel;
    if(level_number == rep_level_2)
    {
        return (((pixel_copy >> shift_14) << shift_4) | (((pixel_copy >> shift_9) & mask_3) << shift_2) | ((pixel_copy >> shift_3) & mask_3));
    }
    else
    {
        return (((pixel_copy >> shift_12) << shift_8) | (((pixel_copy >> shift_7) & mask_000f) << shift_4) | ((pixel_copy >> 1) & mask_000f));
    }   
}


/*
 *qsort_helper
 *Description: helper function that compare the number of pixels in two octree nodes
 *Input: general pointer a and b
 *Output: None
 *Return Value: > 0 if the element pointed by a goes after the element pointed by b
 *Side Effects: None
 */
int qsort_helper(const void *a, const void *b)
{
    const struct octree_node_level4* octree_node_a = a;
    const struct octree_node_level4* octree_node_b = b;
    return (octree_node_b->pixel_number - octree_node_a->pixel_number);
    
}



