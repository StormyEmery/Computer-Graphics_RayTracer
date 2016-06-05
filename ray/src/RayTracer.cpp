// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <thread>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

#define INDEX_AIR 1.0002772
// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

Vec3d RayTracer::trace(double x, double y)
{
  // Clear out the ray cache in the scene for debugging purposes,
  if (TraceUI::m_debug) scene->intersectCache.clear();
  ray r(Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY);
  scene->getCamera().rayThrough(x,y,r);
  Vec3d ret = traceRay(r, traceUI->getDepth());
  ret.clamp();
  return ret;
}

//perform antialiasing?
Vec3d RayTracer::tracePixel(int i, int j)
{
	Vec3d col(0,0,0);
	int numSamples = 1;
	double sample_size = 1.0;

	if( ! sceneLoaded() ) 
		return col;

	if(traceUI->antAlias()) {
		//just set variables for the different cases
		switch(traceUI->getSample()) {
			case 1:
					numSamples = 1;
					sample_size = 1.0;
					break;
			case 2:
					numSamples = 4;
					sample_size = 4.0;
					break;
			case 3:
					numSamples = 9;
					sample_size = 9.0;
					break;
			case 4: 
					numSamples = 16;
					sample_size = 16.0;
					break;
		}
	}

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);
	double step = 1.0/double((traceUI->getSample()));

	for(int k = 1; k <= numSamples; k++) {
		col += trace(x,y);
		if(k % traceUI->getSample() == 0) {
			x = double(i)/double(buffer_width);
			y = double(j+step)/double(buffer_height);
		}
		else
			x = double(i+step)/double(buffer_width);
	}

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	col = col/sample_size;

	pixel[0] = (int)( 255.0 * col[0]); //r
	pixel[1] = (int)( 255.0 * col[1]); //g
	pixel[2] = (int)( 255.0 * col[2]); //b

	return col;
}

bool notTIR(double n, Vec3d Norm, Vec3d dir) {
	bool ret = true;
	double term = 1 - ((n*n) * (1-pow(Norm.dot(Norm,dir),2)));
	if(term < 0) {
		ret = false;
	}

	return ret;
}

//FIX
// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay(ray& r, int depth)
{
	isect i;
	Vec3d colorC;
	double n_i;
	double n_t;
	if(depth >= 0){
		if(scene->intersect(r, i)) {
		  Vec3d Q = r.at(i.t);
		  Vec3d N = i.N;
		  const Material& m = i.getMaterial();
		  colorC = m.shade(scene, r, i); //I from Phone Shading
		  Vec3d nrDir = -(r.getDirection());
		  Vec3d R = ((2.0 * nrDir.dot(nrDir,N))*N) - nrDir; //-ray direction
		  R.normalize();
		  r.d = R;
		  r.p = Q;
		  colorC = colorC + (m.kr(i)%traceRay(r,depth-1));

		  if(N.dot(N,nrDir) > 0.0) { //ray is entering object
		  	n_i = INDEX_AIR;
		  	n_t = m.index(i);
		  }
		  else { //ray is exiting?
		  	n_i = m.index(i);
		  	n_t = INDEX_AIR;
		  	N *= -1;
		  }

		  double n = n_i/n_t;

		  if(m.Trans() && notTIR(n, N, nrDir)) {
		   	double cosIncAngle = N.dot(N,nrDir); //Theta_i
		   	double term = 1 - ((n*n) * (1-(cosIncAngle*cosIncAngle)));
		   	double cosTransAngle = sqrt(term);
		   	Vec3d T = (((n*cosIncAngle) - cosTransAngle)*N) - (n*nrDir);
		   	T.normalize();
		   	r.d = T;
		   	r.p = Q;
		   	colorC = colorC + (m.kt(i)%traceRay(r,depth-1));
		  }
		} 
		else {
			// No intersection.  This ray travels to infinity, so we color
			// it according to the background color, which in this (simple) case
			// is just black.
			if(!traceUI->cm()) {
				colorC = Vec3d(0.0,0.0,0.0);
			}
			else{
				CubeMap* cm = getCubeMap();
				TextureMap* tx;
				Vec2d uv = Vec2d(0.0,0.0);

				//computes max of the absolute value of the ray direction components
				double majorAxis = fmax(fmax(abs(r.d[0]),abs(r.d[1])), abs(r.d[2]));

				//major axis is x
				if(abs(r.d[0]) == majorAxis) {
					if(r.d[0] >= 0) { //major axis is xpos
						tx = cm->getXpos();
						uv[0] = (((r.d[2])/majorAxis) + 1.0)/2.0;
						uv[1] = (((r.d[1])/majorAxis) + 1.0)/2.0;
						colorC = tx->getMappedValue(uv);
					}
					else { //major axis is xneg 
						tx = cm->getXneg();
						uv[0] = (-(r.d[2]/majorAxis) + 1.0)/2.0;
						uv[1] = ((r.d[1]/majorAxis) + 1.0)/2.0;
						colorC = tx->getMappedValue(uv);
					}
				}

				//major axis is y
				else if(abs(r.d[1]) == majorAxis) {
					if(r.d[1] >= 0) { //major axis is ypos
						tx = cm->getYpos();
						uv[0] = ((r.d[0]/majorAxis) + 1.0)/2.0;
						uv[1] = ((r.d[2]/majorAxis) + 1.0)/2.0;
						colorC = tx->getMappedValue(uv);
					}
					else { //major axis is yneg (could be wrong)
						tx = cm->getYneg();
						uv[0] = ((r.d[0]/majorAxis) + 1.0)/2.0;
						uv[1] = ((-r.d[2]/majorAxis) + 1.0)/2.0;
						colorC = tx->getMappedValue(uv);
					}
				}

				//major axis is z
				else if(abs(r.d[2]) == majorAxis) {
					if(r.d[2] < 0) { //major axis is zneg
						tx = cm->getZpos();
						uv[0] = ((r.d[0]/majorAxis) + 1.0)/2.0;
						uv[1] = ((r.d[1]/majorAxis) + 1.0)/2.0;
						colorC = tx->getMappedValue(uv);
					}
					else { //major axis is zpos 
						tx = cm->getZneg();
						uv[0] = ((-r.d[0]/majorAxis) + 1.0)/2.0;
						uv[1] = ((r.d[1]/majorAxis) + 1.0)/2.0;
						colorC = tx->getMappedValue(uv);
					}
				}
			}
		}
	}

	return colorC;
}

RayTracer::RayTracer()
	: scene(0), buffer(0), buffer_width(256), buffer_height(256), m_bBufferReady(false)
{}

RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn ) {
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos ) path = ".";
	else path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if( !sceneLoaded() ) return false;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;
		bufferSize = buffer_width * buffer_height * 3;
		delete[] buffer;
		buffer = new unsigned char[bufferSize];
	}
	memset(buffer, 0, w*h*3);
	m_bBufferReady = true;
}

