/****************************************************************************
 *
 * $Id$
 *
 * Copyright (C) 1998-2010 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Make the complete tracking of an object by using its CAD model
 *
 * Authors:
 * Nicolas Melchior
 * Romain Tallonneau
 * Eric Marchand
 *
 *****************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*!
 \file vpMbtHiddenFace.cpp
 \brief Make the complete tracking of an object by using its CAD model.
*/

#include <visp/vpMbtHiddenFace.h>

/*!
  Basic constructor.
*/
vpMbtPolygon::vpMbtPolygon()
{
  nbpt = 0 ;
  p = NULL ;
  isappearing = false;
  negative = 0;
  angle_1 = -1e6;
}

/*!
  Basic destructor.
*/
vpMbtPolygon::~vpMbtPolygon()
{
//  cout << "Deleting Polygon "  << index  <<  endl ;
  if (p!=NULL)
  {
    delete[] p;
    p = NULL;
  }
}

/*!
  Set the number of points which are the corners of the polygon.
  
  \param nb : The number of corners.
*/
void
vpMbtPolygon::setNbPoint(int nb)
{
  nbpt = nb ;
  if (p != NULL)
    delete[] p;
  p = new vpPoint[nb] ;
}

/*!
  Add a corner point to the list of polygon's corners.
  
  \param n : The index of the corner.
  \param P : The point to add.
*/
void
vpMbtPolygon::addPoint(int n, vpPoint &P)
{
  //if( p!NULL && n < nbpt )
    p[n] = P ;
}

/*!
  Project the 3D corner points into the image thanks to the pose of the camera.
  
  \param cMo : The pose of the camera.
*/
void
vpMbtPolygon::changeFrame(vpHomogeneousMatrix &cMo)
{
  for (int i = 0 ; i < nbpt ; i++)
  {
    p[i].changeFrame(cMo) ;
    p[i].projection() ;
  }
}

/*!
  Check if the polygon is visible in the image. To do that, the polygon is projected into the image thanks to the camera pose.
  
  \param cMo : The pose of the camera.
  
  \return Return true if the polygon is visible.
*/
bool
vpMbtPolygon::isVisible(vpHomogeneousMatrix &cMo)
{
  changeFrame(cMo) ;
  
  if(nbpt <= 2){
    /* a line is allways visible */
    isvisible = true;
    isappearing = false;
    return  true ;
  }

  vpColVector e1(3) ;
  vpColVector e2(3) ;
  vpColVector facenormal(3) ;

  e1[0] = p[1].get_X() - p[0].get_X() ;
  e1[1] = p[1].get_Y() - p[0].get_Y() ;
  e1[2] = p[1].get_Z() - p[0].get_Z() ;

  e2[0] = p[2].get_X() - p[1].get_X() ;
  e2[1] = p[2].get_Y() - p[1].get_Y() ;
  e2[2] = p[2].get_Z() - p[1].get_Z() ;

  facenormal = vpColVector::crossProd(e1,e2) ;

  double angle = p[0].get_X()*facenormal[0] +  p[0].get_Y()*facenormal[1]  +  p[0].get_Z()*facenormal[2]  ;
  
  double diff = angle - angle_1;
  if (diff < 0) negative++;
  else negative =0;

  if(angle < -0.00001 )
  {
    isvisible = true;
    isappearing = false;
    return  true ;
  }
  else
  {
    if (angle < 0.0000001 )//&& negative >=1)
      isappearing = true;
    else
      isappearing = false;
    isvisible = false ;
    return false ;
  }
}



/*!
  Check if the polygon is visible in the image and if the angle between the normal 
  to the face and the normal to the camera plane is below the given threshold. 
  To do that, the polygon is projected into the image thanks to the camera pose.
  
  \param cMo : The pose of the camera.
  \param alpha : maximum angle to detect if the face is visible (in rad)
  
  \return Return true if the polygon is visible.
*/
bool 
vpMbtPolygon::isVisible(vpHomogeneousMatrix &cMo, const double alpha)
{
  changeFrame(cMo) ;
  
  if(nbpt <= 2){
    /* a line is allways visible */
    return  true ;
  }

  vpColVector e1(3) ;
  vpColVector e2(3) ;
  vpColVector facenormal(3) ;

  e1[0] = p[1].get_X() - p[0].get_X() ;
  e1[1] = p[1].get_Y() - p[0].get_Y() ;
  e1[2] = p[1].get_Z() - p[0].get_Z() ;

  e2[0] = p[2].get_X() - p[1].get_X() ;
  e2[1] = p[2].get_Y() - p[1].get_Y() ;
  e2[2] = p[2].get_Z() - p[1].get_Z() ;

  facenormal = vpColVector::crossProd(e1,e2) ;

  vpColVector n_plan(3);
  n_plan[0] = 0;
  n_plan[1] = 0;
  n_plan[2] = 1;
  
  vpColVector n_cam(3);
  n_cam = facenormal;
  
  double angle = p[0].get_X()*facenormal[0] +  p[0].get_Y()*facenormal[1]  +  p[0].get_Z()*facenormal[2]  ;
  
  double n_cam_dot_n_plan = vpColVector::dotProd (n_cam, n_plan);
  double cos_angle = n_cam_dot_n_plan * (1 / ( n_cam.euclideanNorm() * n_plan.euclideanNorm() ));
  double my_angle = acos(cos_angle);
  
  if(angle < 0 && ( my_angle > static_cast<double>(M_PI - alpha) || my_angle < static_cast<double>(-M_PI + alpha) ) ){
    return true;
  }  
  return false;
}


/*!
  Basic constructor.
*/
vpMbtHiddenFaces::vpMbtHiddenFaces()
{
}


/*!
  Basic destructor.
*/
vpMbtHiddenFaces::~vpMbtHiddenFaces()
{
//  cout << "Deleting Hidden Face struxture "<<endl  ;
  vpMbtPolygon *p ;

  Lpol.front() ;
  while (!Lpol.outside())
  {
    p = Lpol.value() ;
    if (p!=NULL) 
    {
      delete p ;
    }
    p = NULL ;

    Lpol.next() ;
  }

  Lpol.kill() ;
}

/*!
  Add a polygon to the list of polygons.
  
  \param p : The polygon to add.
*/
void
vpMbtHiddenFaces::addPolygon(vpMbtPolygon *p)
{
  vpMbtPolygon *p_new = new vpMbtPolygon;
  p_new->index = p->index;
  p_new->setNbPoint(p->nbpt);
  p_new->isvisible = p->isvisible;
  for(int i = 0; i < p->nbpt; i++)
    p_new->p[i]= p->p[i];
  Lpol += p_new ;
}


/*!
  Compute the number of visible polygons.
  
  \param cMo : The pose of the camera
  
  \return Return the number of visible polygons
*/
int
vpMbtHiddenFaces::setVisible(vpHomogeneousMatrix &cMo)
{
  int nbvisiblepolygone = 0 ;
  vpMbtPolygon *p ;

  Lpol.front();
  while (!Lpol.outside())
  {
    p = Lpol.value();

    if (p->isVisible(cMo) ==true) nbvisiblepolygone++;
    Lpol.next() ;
  }
  return nbvisiblepolygone ;
}

/*!
  Check if the polygon with the index \f$ index \f$ is visible or not.
  
  \param index : The index of one polygon in the list of polygons.
  
  \return Return true if the polygon is visible.
*/
bool
vpMbtHiddenFaces::isVisible(int index)
{
  vpMbtPolygon *p ;
  Lpol.front() ;
  while (!Lpol.outside())
  {
    p = Lpol.value() ;
    if (p->getIndex() == index) return p->isVisible() ;
    Lpol.next() ;
  }
  return false ;
}

/*!
  Check if the polygon with the index \f$ index \f$ will appear soon.
  
  \param index : The index of one polygon in the list of polygons.
  
  \return Return true if the polygon will appear soon.
*/
bool
vpMbtHiddenFaces::isAppearing(int index)
{
  vpMbtPolygon *p ;
  Lpol.front() ;
  while (!Lpol.outside())
  {
    p = Lpol.value() ;
    if (p->getIndex() == index) return p->isAppearing() ;
    Lpol.next() ;
  }
  return false ;
}

/*!
  Reset the Hidden faces (remove the list of vpMbtPolygon)
*/
void
vpMbtHiddenFaces::reset()
{
  vpMbtPolygon *p ;

  Lpol.front() ;
  while (!Lpol.outside())
  {
    p = Lpol.value() ;
    if (p!=NULL) 
    {
      delete p ;
    }
    p = NULL ;

    Lpol.next() ;
  }

  Lpol.kill() ;  
}

#endif
