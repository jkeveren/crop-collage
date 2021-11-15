#include <iostream>
#include <filesystem>
#include <sys/xattr.h>
#include <algorithm>
#include <random>
#include <string.h>
#include <Magick++.h>

using namespace std;
using namespace Magick;

int main(int argc, char *argv[]) {
	InitializeMagick("");

	if (argc <= 3) {
		cerr << "Usage: crop-collage <base directory> <xattr name> <out file>\n";
		return 1;
	}

	string baseDirName = argv[1];
	char *xattrName = argv[2];
	string outFileName = argv[3];

	printf("Looking for files in \"%s\" with xattr \"%s\" adnd and saving to \"%s\"\n", baseDirName.c_str(), xattrName, outFileName.c_str());

	vector<Image> images;
	Image image;
	int bufferLength = 100;
	for (auto& p: std::filesystem::recursive_directory_iterator(baseDirName)) {
		string fileName = p.path();

		char cValue[bufferLength];
		ssize_t size = getxattr(fileName.c_str(), xattrName, cValue, bufferLength);
		if (size == -1) {
			continue;
		}
		cerr << "Processing: " << fileName << endl;
		string geometrySpec(cValue);
		geometrySpec = geometrySpec.substr(0, size);
		
		image.read(fileName);
		image.crop(Geometry(geometrySpec));
		image.scale(Geometry(430, 0));
		images.push_back(image);
	}

	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine randomEngine(seed);
	    
	shuffle(begin(images), end(images), randomEngine);
	
	vector<Image> montage;
	
	Montage montageOptions;
	montageOptions.geometry(Geometry(0,0));
	montageOptions.tile("9x9");
	montageOptions.backgroundColor(ColorGray(0.5));
	
	montageImages(&montage, images.begin(), images.end(), montageOptions);
	
	writeImages(montage.begin(), montage.end(), outFileName);

	return 0;
}


