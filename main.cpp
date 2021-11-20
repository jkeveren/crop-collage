#include <iostream>
#include <filesystem>
#include <sys/xattr.h>
#include <algorithm>
#include <random>
#include <string.h>
#include <Magick++.h>
#include <iterator>

using namespace std;
using namespace Magick;

struct ImageCrop {
	string fileName{};
	string cropGeometry{};
	bool used{};
};

int main(int argc, char *argv[]) {
	InitializeMagick("");

	if (argc != 7) {
		cerr << "Usage: crop-collage <search-directory> <xattr-name> <grid-width> <grid-height> <out-image-width> <out-file>\n";
		return 1;
	}

	string baseDirName = argv[1];
	char *xattrName = argv[2];
	string gridWidthString = argv[3];
	string gridHeightString = argv[4];
	string outImageWidthString = argv[5];
	string outFileName = argv[6];

	int gridWidth;
	int gridHeight;
	try {
		gridWidth = stoi(gridWidthString);
		gridHeight = stoi(gridHeightString);
	} catch (...) {
		cerr << "The grid-width and/or grid-height arguments supplied are not numbers.";
	}
	int gridSize = gridWidth * gridHeight;

	int outImageWidth;
	try {
		outImageWidth = stoi(outImageWidthString);
	} catch (...) {
		cerr << "The out-image-width argument supplied is not a number.";
	}
	int tileWidth = outImageWidth / gridWidth;
	
	cout << "Looking for files in \"" << baseDirName << "\" with attribute \"" << xattrName << "\". Creating a \"" << gridWidth << "x" << gridHeight << "\" grid and saving to \"" << outFileName << "\"\n";

	vector<ImageCrop> candidates;
	int bufferLength = 100;
	for (auto& p: std::filesystem::recursive_directory_iterator(baseDirName)) {
		string fileName = p.path();

		char cValue[bufferLength];
		ssize_t size = getxattr(fileName.c_str(), xattrName, cValue, bufferLength);
		if (size == -1) {
			continue;
		}
		cout << "Found: " << fileName << endl;
		string cropGeometry(cValue);
		cropGeometry = cropGeometry.substr(0, size);
		
		ImageCrop candidate{ fileName, cropGeometry, false };
		candidates.push_back(candidate);
	}

	vector<Image> loaded;
	Image image;
	int candidateCount = candidates.size();
	int usedCount = 0;
	for (int index = 0; usedCount < gridSize && usedCount < candidateCount; index = rand() % candidateCount) {
		ImageCrop *imageCrop = &(candidates.at(index));

		if (imageCrop->used) {
			continue;
		}
		imageCrop->used = true;
		usedCount++;
		
		cout << "Loading: " << imageCrop->fileName << endl;
		
		image.read(imageCrop->fileName);
		image.crop(Geometry(imageCrop->cropGeometry));
		image.scale(Geometry(tileWidth, 0));
		loaded.push_back(image);
	}

	cout << "Creating collage..." << endl;
	
	vector<Image> montage;
	
	Montage montageOptions;
	montageOptions.geometry(Geometry(0,0));
	montageOptions.tile(Geometry(gridWidth, gridHeight));
	montageOptions.backgroundColor(ColorGray(0.5));
	
	montageImages(&montage, loaded.begin(), loaded.end(), montageOptions);
	
	writeImages(montage.begin(), montage.end(), outFileName);

	cout << "Done!" << endl;

	return 0;
}


