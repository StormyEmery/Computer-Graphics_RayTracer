#include <cmath>

#include "light.h"

using namespace std;

double DirectionalLight::distanceAttenuation(const Vec3d& P) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation(const ray& r, const Vec3d& p) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  Vec3d ret = Vec3d(1.0,1.0,1.0);
  isect i; 
  ray temp = r;
  temp.p = p;
  temp.d = getDirection(p);

  //since directional lights "posistion" is at infinity,
  //an intersected object will always be before it
  if(scene->intersect(temp,i)) {
          ret = Vec3d(0.0,0.0,0.0);

  }

  return ret;
}

Vec3d DirectionalLight::getColor() const
{
  return color;
}

Vec3d DirectionalLight::getDirection(const Vec3d& P) const
{
  // for directional light, direction doesn't depend on P
  return -orientation;
}

double PointLight::distanceAttenuation(const Vec3d& P) const
{

  // YOUR CODE HERE

  // You'll need to modify this method to attenuate the intensity 
  // of the light based on the distance between the source and the 
  // point P.  For now, we assume no attenuation and just return 1.0

  Vec3d diff = P - position;
  

  float d = diff.length();
  float atten_factors = constantTerm + (linearTerm*d) + (quadraticTerm*d*d);

  double f_d = min(1.0, 1.0/atten_factors);

  return f_d;
}

Vec3d PointLight::getColor() const
{
  return color;
}

Vec3d PointLight::getDirection(const Vec3d& P) const
{
  Vec3d ret = position - P;
  ret.normalize();
  return ret;
}


Vec3d PointLight::shadowAttenuation(const ray& r, const Vec3d& p) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  Vec3d ret = Vec3d(1.0,1.0,1.0);
  isect i; 
  ray temp = r;
  temp.p = p;
  temp.d = getDirection(p);

  if(scene->intersect(temp,i)) {
      Vec3d Q = temp.at(i.t); //intersection point
      //need to check to see if this point is before or after the lights source position
      float lightDist = (p - position).length();
      float isectDist = (p - Q).length();
      if(isectDist < lightDist) {
          ret = Vec3d(0.0,0.0, 0.0);

      }

  }

  return ret;
}
