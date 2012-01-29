/**********************************************************************
 * File:        polyblk.c  (Formerly poly_block.c)
 * Description: Polygonal blocks
 * Author:					Sheelagh Lloyd?
 * Created:
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include    "mfcpch.h"
#include    <stdio.h>
#include          <ctype.h>
#include          <math.h>
#include          "elst.h"
#include    "polyblk.h"

#include          "hpddef.h"     //must be last (handpd.dll)

#define PBLOCK_LABEL_SIZE 150
#define INTERSECTING MAX_INT16

int lessthan(const void *first, const void *second);

POLY_BLOCK::POLY_BLOCK(ICOORDELT_LIST *points, POLY_TYPE t) {
  ICOORDELT_IT v = &vertices;

  vertices.clear ();
  v.move_to_first ();
  v.add_list_before (points);
  compute_bb();
  type = t;
}


/**********************************************************************
 * POLY_BLOCK::compute_bb
 *
 * Compute the bounding box from the outline points.
 **********************************************************************/

void POLY_BLOCK::compute_bb() {  //constructor
  ICOORD ibl, itr;               //integer bb
  ICOORD botleft;                //bounding box
  ICOORD topright;
  ICOORD pos;                    //current pos;
  ICOORDELT_IT pts = &vertices;  //iterator

  botleft = *pts.data ();
  topright = botleft;
  do {
    pos = *pts.data ();
    if (pos.x () < botleft.x ())
                                 //get bounding box
      botleft = ICOORD (pos.x (), botleft.y ());
    if (pos.y () < botleft.y ())
      botleft = ICOORD (botleft.x (), pos.y ());
    if (pos.x () > topright.x ())
      topright = ICOORD (pos.x (), topright.y ());
    if (pos.y () > topright.y ())
      topright = ICOORD (topright.x (), pos.y ());
    pts.forward ();
  }
  while (!pts.at_first ());
  ibl = ICOORD (botleft.x (), botleft.y ());
  itr = ICOORD (topright.x (), topright.y ());
  box = TBOX (ibl, itr);
}


/**********************************************************************
 * POLY_BLOCK::winding_number
 *
 * Return the winding number of the outline around the given point.
 **********************************************************************/

inT16 POLY_BLOCK::winding_number(                     //winding number
                                 const ICOORD &point  //point to wind around
                                ) {
  inT16 count;                   //winding count
  ICOORD pt;                     //current point
  ICOORD vec;                    //point to current point
  ICOORD vvec;                   //current point to next point
  inT32 cross;                   //cross product
  ICOORDELT_IT it = &vertices;   //iterator

  count = 0;
  do {
    pt = *it.data ();
    vec = pt - point;
    vvec = *it.data_relative (1) - pt;
                                 //crossing the line
    if (vec.y () <= 0 && vec.y () + vvec.y () > 0) {
      cross = vec * vvec;        //cross product
      if (cross > 0)
        count++;                 //crossing right half
      else if (cross == 0)
        return INTERSECTING;     //going through point
    }
    else if (vec.y () > 0 && vec.y () + vvec.y () <= 0) {
      cross = vec * vvec;
      if (cross < 0)
        count--;                 //crossing back
      else if (cross == 0)
        return INTERSECTING;     //illegal
    }
    else if (vec.y () == 0 && vec.x () == 0)
      return INTERSECTING;
    it.forward ();
  }
  while (!it.at_first ());
  return count;                  //winding number
}


/**********************************************************************
 * POLY_BLOCK::contains
 *
 * Is poly within poly
 **********************************************************************/

BOOL8 POLY_BLOCK::contains(  //other outline
                           POLY_BLOCK *other) {
  inT16 count;                   //winding count
  ICOORDELT_IT it = &vertices;   //iterator
  ICOORD vertex;

  if (!box.overlap (*(other->bounding_box ())))
    return FALSE;                //can't be contained

  /* check that no vertex of this is inside other */

  do {
    vertex = *it.data ();
                                 //get winding number
    count = other->winding_number (vertex);
    if (count != INTERSECTING)
      if (count != 0)
        return (FALSE);
    it.forward ();
  }
  while (!it.at_first ());

  /* check that all vertices of other are inside this */

                                 //switch lists
  it.set_to_list (other->points ());
  do {
    vertex = *it.data ();
                                 //try other way round
    count = winding_number (vertex);
    if (count != INTERSECTING)
      if (count == 0)
        return (FALSE);
    it.forward ();
  }
  while (!it.at_first ());
  return TRUE;
}


/**********************************************************************
 * POLY_BLOCK::rotate
 *
 * Rotate the POLY_BLOCK.
 **********************************************************************/

void POLY_BLOCK::rotate(                 //constructor
                        FCOORD rotation  //cos,sin of angle
                       ) {
  FCOORD pos;                    //current pos;
  ICOORDELT *pt;                 //current point
  ICOORDELT_IT pts = &vertices;  //iterator

  do {
    pt = pts.data ();
    pos.set_x (pt->x ());
    pos.set_y (pt->y ());
    pos.rotate (rotation);
    pt->set_x ((inT16) (floor (pos.x () + 0.5)));
    pt->set_y ((inT16) (floor (pos.y () + 0.5)));
    pts.forward ();
  }
  while (!pts.at_first ());
  compute_bb();
}


/**********************************************************************
 * POLY_BLOCK::move
 *
 * Move the POLY_BLOCK.
 **********************************************************************/

void POLY_BLOCK::move(              //constructor
                      ICOORD shift  //cos,sin of angle
                     ) {
  ICOORDELT *pt;                 //current point
  ICOORDELT_IT pts = &vertices;  //iterator

  do {
    pt = pts.data ();
    *pt += shift;
    pts.forward ();
  }
  while (!pts.at_first ());
  compute_bb();
}


#ifndef GRAPHICS_DISABLED
void POLY_BLOCK::plot(ScrollView* window, ScrollView::Color colour, inT32 num) {
  ICOORDELT_IT v = &vertices;

  window->Pen(colour);

  v.move_to_first ();

  if (num > 0) {
    window->Pen(colour);
    window->TextAttributes("Times", 80, false, false, false);
    char temp_buff[34];
    #ifdef __UNIX__
    sprintf(temp_buff, INT32FORMAT, num);
    #else
    ltoa (num, temp_buff, 10);
    #endif
    window->Text(v.data ()->x (), v.data ()->y (), temp_buff);
  }

  window->SetCursor(v.data ()->x (), v.data ()->y ());
  for (v.mark_cycle_pt (); !v.cycled_list (); v.forward ()) {
    window->DrawTo(v.data ()->x (), v.data ()->y ());
   }
  v.move_to_first ();
   window->DrawTo(v.data ()->x (), v.data ()->y ());
}


void POLY_BLOCK::fill(ScrollView* window, ScrollView::Color colour) {
  inT16 y;
  inT16 width;
  PB_LINE_IT *lines;
  ICOORDELT_LIST *segments;
  ICOORDELT_IT s_it;

  lines = new PB_LINE_IT (this);
  window->Pen(colour);

  for (y = this->bounding_box ()->bottom ();
  y <= this->bounding_box ()->top (); y++) {
    segments = lines->get_line (y);
    if (!segments->empty ()) {
      s_it.set_to_list (segments);
      for (s_it.mark_cycle_pt (); !s_it.cycled_list (); s_it.forward ()) {
        // Note different use of ICOORDELT, x coord is x coord of pixel
        // at the start of line segment, y coord is length of line segment
        // Last pixel is start pixel + length.
        width = s_it.data ()->y ();
        window->SetCursor(s_it.data ()->x (), y);
        window->DrawTo(s_it.data ()->x () + (float) width, y);
      }
    }
  }
}
#endif


BOOL8 POLY_BLOCK::overlap(  // do polys overlap
                          POLY_BLOCK *other) {
  inT16 count;                   //winding count
  ICOORDELT_IT it = &vertices;   //iterator
  ICOORD vertex;

  if (!box.overlap (*(other->bounding_box ())))
    return FALSE;                //can't be any overlap

  /* see if a vertex of this is inside other */

  do {
    vertex = *it.data ();
                                 //get winding number
    count = other->winding_number (vertex);
    if (count != INTERSECTING)
      if (count != 0)
        return (TRUE);
    it.forward ();
  }
  while (!it.at_first ());

  /* see if a vertex of other is inside this */

                                 //switch lists
  it.set_to_list (other->points ());
  do {
    vertex = *it.data ();
                                 //try other way round
    count = winding_number (vertex);
    if (count != INTERSECTING)
      if (count != 0)
        return (TRUE);
    it.forward ();
  }
  while (!it.at_first ());
  return FALSE;
}


ICOORDELT_LIST *PB_LINE_IT::get_line(inT16 y) {
  ICOORDELT_IT v, r;
  ICOORDELT_LIST *result;
  ICOORDELT *x, *current, *previous;
  float fy, fx;

  fy = (float) (y + 0.5);
  result = new ICOORDELT_LIST ();
  r.set_to_list (result);
  v.set_to_list (block->points ());

  for (v.mark_cycle_pt (); !v.cycled_list (); v.forward ()) {
    if (((v.data_relative (-1)->y () > y) && (v.data ()->y () <= y))
    || ((v.data_relative (-1)->y () <= y) && (v.data ()->y () > y))) {
      previous = v.data_relative (-1);
      current = v.data ();
      fx = (float) (0.5 + previous->x () +
        (current->x () - previous->x ()) * (fy -
        previous->y ()) /
        (current->y () - previous->y ()));
      x = new ICOORDELT ((inT16) fx, 0);
      r.add_to_end (x);
    }
  }

  if (!r.empty ()) {
    r.sort (lessthan);
    for (r.mark_cycle_pt (); !r.cycled_list (); r.forward ())
      x = r.data ();
    for (r.mark_cycle_pt (); !r.cycled_list (); r.forward ()) {
      r.data ()->set_y (r.data_relative (1)->x () - r.data ()->x ());
      r.forward ();
      delete (r.extract ());
    }
  }

  return result;
}


int lessthan(const void *first, const void *second) {
  ICOORDELT *p1 = (*(ICOORDELT **) first);
  ICOORDELT *p2 = (*(ICOORDELT **) second);

  if (p1->x () < p2->x ())
    return (-1);
  else if (p1->x () > p2->x ())
    return (1);
  else
    return (0);
}


/**********************************************************************
 * POLY_BLOCK::serialise_asc
 *
 * Converto to ascii file.
 **********************************************************************/

void POLY_BLOCK::serialise_asc(         //convert to ascii
                               FILE *f  //file to use
                              ) {
  vertices.serialise_asc (f);
  box.serialise_asc (f);
  serialise_INT32(f, type);
}


/**********************************************************************
 * POLY_BLOCK::de_serialise_asc
 *
 * Converto from ascii file.
 **********************************************************************/

void POLY_BLOCK::de_serialise_asc(         //convert from ascii
                                  FILE *f  //file to use
                                 ) {
  vertices.de_serialise_asc (f);
  box.de_serialise_asc (f);
  type = (POLY_TYPE) de_serialise_INT32 (f);
}
