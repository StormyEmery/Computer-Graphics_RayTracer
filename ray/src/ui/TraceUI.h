//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

// who the hell cares if my identifiers are longer than 255 characters:
#pragma warning(disable : 4786)

#include <string>
#include <thread>

using std::string;
using std::thread;

class RayTracer;

class TraceUI {
public:
	TraceUI() : m_nDepth(0), m_nSize(512), m_nThreads(thread::hardware_concurrency()),m_nSample(1),m_displayDebuggingInfo(false),
                    m_shadows(true), m_smoothshade(true), raytracer(0),
                    m_nFilterWidth(1)
                    {
                    	// m_nThreads = thread::hardware_concurrency()-2;makmk
                    }

	virtual int	run() = 0;

	// Send an alert to the user in some manner
	virtual void alert(const string& msg) = 0;

	// setters
	virtual void setRayTracer( RayTracer* r ) { raytracer = r; }
	void setCubeMap(bool b) { m_gotCubeMap = b; }
	void useCubeMap(bool b) { m_usingCubeMap = b; }

	// accessors:
	int getSample() const { return m_nSample; }
	int	getSize() const { return m_nSize; }
	int	getDepth() const { return m_nDepth; }
	int		getFilterWidth() const { return m_nFilterWidth; }

	bool	cm() const{ return m_usingCubeMap; } 	
	bool	shadowSw() const { return m_shadows; }
	bool	smShadSw() const { return m_smoothshade; }
	bool 	antAlias() const { return m_antialiasing; }

	static bool m_debug;

protected:
	RayTracer*	raytracer;

	int	m_nSize;	// Size of the traced image
	int	m_nDepth;	// Max depth of recursion
	int m_nSample;  // Number of Saples for Antialiasing
	int m_nThreads; // Number of threads used

	// Determines whether or not to show debugging information
	// for individual rays.  Disabled by default for efficiency
	// reasons.
	bool m_displayDebuggingInfo;
	bool m_antialiasing; // turn on/off antialiasing?
	bool m_shadows;  // compute shadows?
	bool m_smoothshade;  // turn on/off smoothshading?
	bool		m_usingCubeMap;  // render with cubemap
	bool		m_gotCubeMap;  // cubemap defined
	int m_nFilterWidth;  // width of cubemap filter
};

#endif
