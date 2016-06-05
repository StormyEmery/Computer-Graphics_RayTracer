#include <cmath>
#include <float.h>
#include <algorithm>
#include <assert.h>
#include "trimesh.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    if (!newFace->degen) faces.push_back( newFace );


    // Don't add faces to the scene's object list so we can cull by bounding box
    // scene->add(newFace);
    return true;
}

char* Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	double tmin = 0.0;
	double tmax = 0.0;
	typedef Faces::const_iterator iter;
	bool have_one = false;
	for( iter j = faces.begin(); j != faces.end(); ++j )
	  {
	    isect cur;
	    if( (*j)->intersectLocal( r, cur ) )
	      {
		if( !have_one || (cur.t < i.t) )
		  {
		    i = cur;
		    have_one = true;
		  }
	      }
	  }
	if( !have_one ) i.setT(1000.0);
	return have_one;
}

bool TrimeshFace::intersect(ray& r, isect& i) const {
  return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
    Vec3d n = Vec3d();
    Vec3d Q = Vec3d();
    Vec3d aQ = Vec3d();
    Vec3d bQ = Vec3d();
    Vec3d cQ = Vec3d();
    Vec3d bC = Vec3d();
    Vec2d uv = Vec2d();
    Vec3d dir = r.getDirection();
    Vec3d p = r.getPosition();
    double d, nP, nDir, t, aQN, bQN, cQN, denom, alpha, beta, gamma;
    int x = 0;
    int y = 1;
    int z = 2;
    Vec3d& a = parent->vertices[ids[0]];
    const Vec3d& b = parent->vertices[ids[1]];
    const Vec3d& c = parent->vertices[ids[2]];

    // YOUR CODE HERE
    
    //calculating cross product of B-A and C-A
    Vec3d b_a = b-a;
    Vec3d c_a = c-a;
    bC = bC.cross(b_a, c_a);

    //normalized for later use with shading
    //bC.normalize();

    n = bC;
    n.normalize();

    //dot product of n and dir
    //if this == 0, ray does not intersect plane
    nDir = n.dot(n,dir);
    if(nDir == 0) {
        return false;
    }

    //dot product of n and a
    d = n.dot(n,a);

    //dot product of n and p
    nP = n.dot(n,p);

    t = ((d - nP)/(nDir));

    if(t <= RAY_EPSILON)
        return false;

    //point of intersection
    Q = r.at(t);

    //triangle intersection; 3 cases
    //a-c cross q-c times n >= 0
    Vec3d a_c = a-c;
    Vec3d q_c = Q-c;
    aQ = aQ.cross(a_c, q_c);

    aQN = n.dot(n,aQ);

    if(aQN < 0) {
        return false;
    }

    //b-a cross q-a times n >= 0
    Vec3d q_a = Q-a;
    bQ = bQ.cross(b_a, q_a);

    bQN = n.dot(n,bQ);

    if(bQN < 0) {
        return false;
    }

    //c-b cross q-b times n >= 0
    Vec3d c_b = c-b;
    Vec3d q_b = Q-b;
    cQ = cQ.cross(c_b, q_b);

    cQN = n.dot(n,cQ);

    if (cQN < 0) {
        return false;
    }

    //if we reach this point, the point Q is inside the triangle,
    //so we need to compute barycentric coords and store all useful info.
    denom = n.dot(n,bC);
    alpha = cQN/denom;
    beta = aQN/denom;
    gamma = bQN/denom;
    uv[x] = alpha;
    uv[y] = beta;
    i.setUVCoordinates(uv); // I think these might be the wrong uv values
    i.setT(t);
    i.setBary(alpha, beta, gamma);
    i.setN(n);

    i.setMaterial(this->getMaterial());

    if(!parent->materials.empty()) {
            Material *aM = parent->materials[ids[0]];
            Material *bM = parent->materials[ids[1]];
            Material *cM = parent->materials[ids[2]];

            Material m = (alpha*(*aM));
            m += (beta*(*bM));
            m += (gamma*(*cM));
            i.setMaterial(m);
    }
    if(traceUI->smShadSw()){
        if(!parent->normals.empty() && parent->vertNorms) {
            n = (parent->normals[ids[0]]*alpha) + (parent->normals[ids[1]]*beta) + (parent->normals[ids[2]]*gamma);
            n.normalize();
            i.setN(n);
        }
    }

    return true;
}

void Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
		Vec3d faceNormal = (**fi).getNormal();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
    vertNorms = true;
}
