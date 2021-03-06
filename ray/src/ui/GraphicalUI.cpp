//
// GraphicalUI.cpp
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifndef COMMAND_LINE_ONLY

#include <FL/fl_ask.H>
#include "debuggingView.h"

#include "GraphicalUI.h"
#include "../RayTracer.h"

#define MAX_INTERVAL 500

#ifdef _WIN32
#define print sprintf_s
#else
#define print sprintf
#endif

using std::thread;

bool GraphicalUI::stopTrace = false;
bool GraphicalUI::doneTrace = true;
GraphicalUI* GraphicalUI::pUI = NULL;
char* GraphicalUI::traceWindowLabel = "Stormy Emery's Ray Tracer";
bool TraceUI::m_debug = false;

//------------------------------------- Help Functions --------------------------------------------
GraphicalUI* GraphicalUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ((GraphicalUI*)(o->parent()->user_data()));
}

//--------------------------------- Callback Functions --------------------------------------------

void GraphicalUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	static char* lastFile = 0;
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			print(buf, "Ray <%s>", newfile);
			stopTracing();	// terminate the previous rendering
		} else print(buf, "Ray <Not Loaded>");

		pUI->m_mainWindow->label(buf);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();

		if( lastFile != 0 && strcmp(newfile, lastFile) != 0 )
			pUI->m_debuggingWindow->m_debuggingView->resetCamera();

		pUI->m_debuggingWindow->redraw();
	}
}

void GraphicalUI::cb_load_cubeMap(Fl_Menu_* o, void* v)
{
	pUI = whoami(o);

	pUI->m_cubeMapChooser->setCaller(pUI);
	pUI->m_cubeMapChooser->show();
	
}


void GraphicalUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	pUI = whoami(o);

	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void GraphicalUI::cb_exit(Fl_Menu_* o, void* v)
{
	pUI = whoami(o);

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
	pUI->m_cubeMapChooser->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_exit2(Fl_Widget* o, void* v) 
{
	pUI = (GraphicalUI *)(o->user_data());

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
	pUI->m_cubeMapChooser->hide();
	TraceUI::m_debug = false;
}

void GraphicalUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project for CS384g.");
}

void GraphicalUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());

	// terminate the rendering so we don't get crashes
	stopTracing();

	pUI->m_nSize=int(((Fl_Slider *)o)->value());
	int width = (int)(pUI->getSize());
	int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow(width, height);
}

void GraphicalUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_refreshSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->refreshInterval=clock_t(((Fl_Slider *)o)->value()) ;
}

void GraphicalUI::cb_thread(Fl_Widget* o, void* v) 
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_nThreads=int(((Fl_Slider *)o)->value());
}

void GraphicalUI::cb_aaSlides(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_nSample=int(((Fl_Slider *)o)->value());
	// ((GraphicalUI*)(o->user_data()))->m_nSample=int( ((Fl_Slider *)o)->value() );
}

void GraphicalUI::cb_filterSlides(Fl_Widget* o, void* v) {
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_nFilterWidth=int(((Fl_Slider *)o)->value());
}

void GraphicalUI::cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_displayDebuggingInfo = (((Fl_Check_Button*)o)->value() == 1);
	if (pUI->m_displayDebuggingInfo)
	  {
	    pUI->m_debuggingWindow->show();
	    pUI->m_debug = true;
	  }
	else
	  {
	    pUI->m_debuggingWindow->hide();
	    pUI->m_debug = false;
	  }
}

void GraphicalUI::cb_ssCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_smoothshade = (((Fl_Check_Button*)o)->value() == 1);
	if(pUI->m_smoothshade)
	  {
		pUI->m_smoothshade = true;
	  }
	else
	  {
	  	pUI->m_smoothshade = false;
	  }
}

void GraphicalUI::cb_shCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_shadows = (((Fl_Check_Button*)o)->value() == 1);
	if(pUI->m_shadows)
	  {
		pUI->m_shadows = true;
	  }
	else
	  {
	  	pUI->m_shadows = false;
	  }
}

void GraphicalUI::cb_aaCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_antialiasing = (((Fl_Check_Button*)o)->value() == 1);
	if(pUI->m_antialiasing)
	  {
		pUI->m_antialiasing = true;
	  }
	else
	  {
	  	pUI->m_antialiasing = false;
	  }
}

void GraphicalUI::cb_cmCheckButton(Fl_Widget* o, void* v)
{
	pUI=(GraphicalUI*)(o->user_data());
	pUI->m_usingCubeMap = (((Fl_Check_Button*)o)->value() == 1);
	if(pUI->m_usingCubeMap)
	  {
		pUI->m_usingCubeMap = true;
	  }
	else
	  {
	  	pUI->m_usingCubeMap = false;
	  }
}

void GraphicalUI::helperTrace(int start, int end, int y) {
	stopTrace = false;
	for(int x=start; x < end; x++) {
		if (stopTrace) break;
		pUI->raytracer->tracePixel(x,y);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();
	}
}

void GraphicalUI::cb_render(Fl_Widget* o, void* v) {

	char buffer[256];

	pUI = (GraphicalUI*)(o->user_data());
	doneTrace = stopTrace = false;
	if (pUI->raytracer->sceneLoaded())
	  {
		int width = pUI->getSize();
		int height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		int origPixels = width * height;
		pUI->m_traceGlWindow->resizeWindow(width, height);
		pUI->m_traceGlWindow->show();
		pUI->raytracer->traceSetup(width, height);

		// Save the window label
                const char *old_label = pUI->m_traceGlWindow->label();

		clock_t now, prev;
		now = prev = clock();
		clock_t intervalMS = pUI->refreshInterval * 100;
		int step = pUI->getThreadNum();
		int size = width/step;
		int start = 0;
		int end = size;
		thread t[step];

		for (int y = 0; y < height; y++)
		  {
				// check for input and refresh view every so often while tracing
				now = clock();
				if ((now - prev)/CLOCKS_PER_SEC * 1000 >= intervalMS)
				  {
				    prev = now;
				    sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
				    pUI->m_traceGlWindow->label(buffer);
				    pUI->m_traceGlWindow->refresh();
				    Fl::check();
				    if (Fl::damage()) { Fl::flush(); }
				  }

				// look for input and refresh window
				for(int i = 0; i < step; i++) {
					if (stopTrace) break;
					t[i] = thread(helperTrace,start,end,y);
					if(end == width) {
						start = 0;
						end = size;
					}
					else {
						start = end;
						end += size;
					}
				}

				//joins threads if they are joinable
				//fixed bug where program crashed
				//if the stop button was clicked
				//before image was fully rendered
				for(int i = 0; i < step; i++) {
					if(t[i].joinable())
						t[i].join();
				}

				if (stopTrace) break;
		  }
		doneTrace = true;
		stopTrace = false;
		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);
		pUI->m_traceGlWindow->refresh();
	  }
}

void GraphicalUI::cb_stop(Fl_Widget* o, void* v)
{
	pUI = (GraphicalUI*)(o->user_data());
	stopTracing();
}

int GraphicalUI::run()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

	m_mainWindow->show();

	return Fl::run();
}

void GraphicalUI::alert( const string& msg )
{
	fl_alert( "%s", msg.c_str() );
}

void GraphicalUI::setRayTracer(RayTracer *tracer)
{
	TraceUI::setRayTracer(tracer);
	m_traceGlWindow->setRayTracer(tracer);
	m_debuggingWindow->m_debuggingView->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item GraphicalUI::menuitems[] = {
	{ "&File", 0, 0, 0, FL_SUBMENU },
	{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)GraphicalUI::cb_load_scene },
	{ "&Load Cubemap...", FL_ALT + 'c', (Fl_Callback *)GraphicalUI::cb_load_cubeMap },
	{ "&Save Image...", FL_ALT + 's', (Fl_Callback *)GraphicalUI::cb_save_image },
	{ "&Exit", FL_ALT + 'e', (Fl_Callback *)GraphicalUI::cb_exit },
	{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
	{ "&About",	FL_ALT + 'a', (Fl_Callback *)GraphicalUI::cb_about },
	{ 0 },

	{ 0 }
};

void GraphicalUI::stopTracing()
{
	stopTrace = true;
}

GraphicalUI::GraphicalUI() : refreshInterval(10) {
	// init.
	m_mainWindow = new Fl_Window(100, 40, 450, 459, "Ray <Not Loaded>");
	m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
	// install menu bar
	m_menubar = new Fl_Menu_Bar(0, 0, 440, 25);
	m_menubar->menu(menuitems);

	// set up "render" button
	m_renderButton = new Fl_Button(360, 37, 70, 25, "&Render");
	m_renderButton->user_data((void*)(this));
	m_renderButton->callback(cb_render);

	// set up "stop" button
	m_stopButton = new Fl_Button(360, 65, 70, 25, "&Stop");
	m_stopButton->user_data((void*)(this));
	m_stopButton->callback(cb_stop);

	// install depth slider
	m_depthSlider = new Fl_Value_Slider(10, 40, 180, 20, "Recursion Depth");
	m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_depthSlider->type(FL_HOR_NICE_SLIDER);
	m_depthSlider->labelfont(FL_COURIER);
	m_depthSlider->labelsize(12);
	m_depthSlider->minimum(0);
	m_depthSlider->maximum(10);
	m_depthSlider->step(1);
	m_depthSlider->value(m_nDepth);
	m_depthSlider->align(FL_ALIGN_RIGHT);
	m_depthSlider->callback(cb_depthSlides);

	// install size slider
	m_sizeSlider = new Fl_Value_Slider(10, 65, 180, 20, "Screen Size");
	m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_sizeSlider->type(FL_HOR_NICE_SLIDER);
	m_sizeSlider->labelfont(FL_COURIER);
	m_sizeSlider->labelsize(12);
	m_sizeSlider->minimum(64);
	m_sizeSlider->maximum(1024);
	m_sizeSlider->step(2);
	m_sizeSlider->value(m_nSize);
	m_sizeSlider->align(FL_ALIGN_RIGHT);
	m_sizeSlider->callback(cb_sizeSlides);

	// install refresh interval slider
	m_refreshSlider = new Fl_Value_Slider(10, 90, 180, 20, "Screen Refresh Interval (0.1 sec)");
	m_refreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_refreshSlider->type(FL_HOR_NICE_SLIDER);
	m_refreshSlider->labelfont(FL_COURIER);
	m_refreshSlider->labelsize(12);
	m_refreshSlider->minimum(1);
	m_refreshSlider->maximum(300);
	m_refreshSlider->step(1);
	m_refreshSlider->value(refreshInterval);
	m_refreshSlider->align(FL_ALIGN_RIGHT);
	m_refreshSlider->callback(cb_refreshSlides);

	//install thread slider
	m_threadsSlider = new Fl_Value_Slider(10, 115, 180, 20, "Threads");
	m_threadsSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_threadsSlider->type(FL_HOR_NICE_SLIDER);
	m_threadsSlider->labelfont(FL_COURIER);
	m_threadsSlider->labelsize(12);
	m_threadsSlider->minimum(1);
	m_threadsSlider->maximum(32);
	m_threadsSlider->step(1);
	m_threadsSlider->value(m_nThreads);
	m_threadsSlider->align(FL_ALIGN_RIGHT);
	m_threadsSlider->callback(cb_thread);

	//install antialiasing checkbox and slider
	m_aaCheckButton = new Fl_Check_Button(10, 175, 140, 20, "Antialias");
	m_aaCheckButton->user_data((void*)(this));
	m_aaCheckButton->callback(cb_aaCheckButton);
	m_aaCheckButton->value(m_antialiasing);

	//intall antialiasing slider
	m_aaSamplesSlider = new Fl_Value_Slider(100, 175, 180, 20, "Pixel Samples: 1=1, 2=4, \n3=9, 4=16");
	m_aaSamplesSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_aaSamplesSlider->type(FL_HOR_NICE_SLIDER);
	m_aaSamplesSlider->labelfont(FL_COURIER);
	m_aaSamplesSlider->labelsize(10);
	m_aaSamplesSlider->minimum(1);
	m_aaSamplesSlider->maximum(4);
	m_aaSamplesSlider->step(1);
	m_aaSamplesSlider->value(m_nSample);
	m_aaSamplesSlider->align(FL_ALIGN_RIGHT);
	m_aaSamplesSlider->callback(cb_aaSlides);

	//install cubeMapping checkbox 
	m_cubeMapCheckButton = new Fl_Check_Button(10, 225, 140, 20, "CubeMap");
	m_cubeMapCheckButton->user_data((void*)(this));
	m_cubeMapCheckButton->callback(cb_cmCheckButton);
	m_cubeMapCheckButton->value(m_usingCubeMap);
	m_cubeMapCheckButton->deactivate();

	//intall cubeMapping slider
	m_filterSlider = new Fl_Value_Slider(100, 225, 180, 20, "Filter Width");
	m_filterSlider->user_data((void*)(this));	// record self to be used by static callback functions
	m_filterSlider->type(FL_HOR_NICE_SLIDER);
	m_filterSlider->labelfont(FL_COURIER);
	m_filterSlider->labelsize(12);
	m_filterSlider->minimum(1);
	m_filterSlider->maximum(17);
	m_filterSlider->step(1);
	m_filterSlider->value(m_nFilterWidth);
	m_filterSlider->align(FL_ALIGN_RIGHT);
	m_filterSlider->callback(cb_filterSlides);
	m_filterSlider->deactivate();

	//install smoothshading button
	m_ssCheckButton = new Fl_Check_Button(10, 400, 140, 20, "Smoothshade");
	m_ssCheckButton->user_data((void*)(this));
	m_ssCheckButton->callback(cb_ssCheckButton);
	m_ssCheckButton->value(m_smoothshade);

	//install shadow button
	m_shCheckButton = new Fl_Check_Button(150, 400, 140, 20, "Shadows");
	m_shCheckButton->user_data((void*)(this));
	m_shCheckButton->callback(cb_shCheckButton);
	m_shCheckButton->value(m_shadows);

	// set up debugging display checkbox
	m_debuggingDisplayCheckButton = new Fl_Check_Button(10, 429, 140, 20, "Debugging display");
	m_debuggingDisplayCheckButton->user_data((void*)(this));
	m_debuggingDisplayCheckButton->callback(cb_debuggingDisplayCheckButton);
	m_debuggingDisplayCheckButton->value(m_displayDebuggingInfo);

	m_mainWindow->callback(cb_exit2);
	m_mainWindow->when(FL_HIDE);
	m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, traceWindowLabel);
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);


	m_cubeMapChooser = new CubeMapChooser();



	// debugging view
	m_debuggingWindow = new DebuggingWindow();
}

#endif
