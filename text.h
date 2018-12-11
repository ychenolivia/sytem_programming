/* tab:4
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
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
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep 9 22:08:16 2004
 * Filename:      text.h
 * History:
 *    SL    1    Thu Sep 9 22:08:16 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:40:11 2009
 *        Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT 16

#define row_num 16		//total row numbers
#define mask 0x80       
#define plane_num 4     //total plane numbers
#define colnumber 8     //total column numbers
#define bitnumber 8     //total bit number
#define and32      3     //logic-use number
#define total_planesize 5760  //total plane size
#define plane4    4      //total plane numbers
#define c3        0x3c   //offset
#define total320 320     //offset
#define start312 312     //the start column of strange spot
#define x05      0x05    //color
#define x18      18      //offset
#define x03      0x03    //offset
#define chara    "_"     //input _
#define emptyspace  ' '  //input " "

/* Standard VGA text font. */
extern unsigned char font_data[256][16];
unsigned char status_buff[total_planesize];
void write_room_name(const char* msg);
void write_status_message(const char* msg);
void clear_status_bar();
void write_user_input(const char* msg);

#endif /* TEXT_H */
