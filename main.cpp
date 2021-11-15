#include <iostream>
#include <Magick++.h>

using namespace std;
using namespace Magick;

int main(int argc, char **argv) {
	InitializeMagick(*argv);
	
	list<Image> images;
	Image image;
	image.read("images/2.png");
	image.scale(Geometry(430, 0));
	images.push_back(image);
	image.read("images/test.png");
	image.scale(Geometry(430, 0));
	images.push_back(image);

	list<Image> montage;
	
	Montage montageOptions;
	montageOptions.geometry(Geometry(0,0));
	montageOptions.tile("9x9");
	montageOptions.backgroundColor(ColorGray(0.5));
	
	montageImages(&montage, images.begin(), images.end(), montageOptions);
	writeImages(montage.begin(), montage.end(), "out.jpg");

	return 0;
}


