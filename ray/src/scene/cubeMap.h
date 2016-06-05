#ifndef __CubeMap_HEADER__
#define __CubeMap_HEADER__

#include "material.h"


class CubeMap {


	TextureMap*  xpos;
	TextureMap*  xneg;
	TextureMap*  ypos;
	TextureMap*  yneg;
	TextureMap*  zpos;
	TextureMap*  zneg;


public:
	CubeMap(){}
	~CubeMap(){}

	void setXposMap(TextureMap* Xpos){ xpos = Xpos;};
	void setXnegMap(TextureMap* Xneg){ xneg = Xneg;};
	void setYposMap(TextureMap* Ypos){ ypos = Ypos;};
	void setYnegMap(TextureMap* Yneg){ yneg = Yneg;};
	void setZposMap(TextureMap* Zpos){ zpos = Zpos;};
	void setZnegMap(TextureMap* Zneg){ zneg = Zneg;};

	TextureMap* getXpos(){ return xpos;}
	TextureMap* getXneg(){ return xneg;}
	TextureMap* getYpos(){ return ypos;}
	TextureMap* getYneg(){ return yneg;}
	TextureMap* getZpos(){ return zpos;}
	TextureMap* getZneg(){ return zneg;}
};


#endif