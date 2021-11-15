#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/xattr.h>
#include <string.h>
#include <Magick++.h>

using namespace std;
using namespace Magick;

int main(int argc, char *argv[]) {
	InitializeMagick("");

	if (argc <= 2) {
		cerr << "Usage: crop-collage <base-dir> <xattr name>\n";
		return 1;
	}

	string baseDirName = argv[1];
	string xattrName = argv[2];

	printf("Looking for files in \"%s\" with xattr \"%s\"\n", baseDirName.c_str(), xattrName.c_str());

	list<Image> images;
	Image image;
	int bufferLength = 100;
	for (auto& p: std::filesystem::recursive_directory_iterator(baseDirName)) {
		string fileName = p.path();

		char cValue[bufferLength];
		ssize_t size = getxattr(fileName.c_str(), xattrName.c_str(), cValue, bufferLength);
		if (size == -1) {
			continue;
		}
		string geometrySpec(cValue);
		geometrySpec = geometrySpec.substr(0, size);
		cout << fileName << endl;
		
		image.read(fileName);
		image.crop(Geometry(geometrySpec));
		image.scale(Geometry(430, 0));
		images.push_back(image);
	}
	
	list<Image> montage;
	
	Montage montageOptions;
	montageOptions.geometry(Geometry(0,0));
	montageOptions.tile("9x9");
	montageOptions.backgroundColor(ColorGray(0.5));
	
	montageImages(&montage, images.begin(), images.end(), montageOptions);
	writeImages(montage.begin(), montage.end(), "out.jpg");

	return 0;
}


