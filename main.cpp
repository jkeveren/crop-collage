#include <iostream>
#include <filesystem>
#include <random>
#include <sys/xattr.h>
#include <Magick++.h>

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

	srand (time(NULL));
	vector<Image> loaded;
	Image image;
	int candidateCount = candidates.size();
	int candidateIndex, gridIndex = 0;
	while (gridIndex < gridSize && gridIndex < candidateCount) {
		candidateIndex = rand() % candidateCount;
		ImageCrop *imageCrop = &(candidates.at(candidateIndex));

		if (imageCrop->used) {
			continue;
		}

		int rowIndex = floor(float(gridIndex) / float(gridWidth));
		int columnIndex = gridIndex - rowIndex * gridWidth;
		cout << "Processing tile " << gridIndex + 1 << " of " << gridSize << " (row: " << rowIndex + 1 << ", column: " << columnIndex + 1 << "): \"" << imageCrop->fileName << "\"" << endl;

		try {
			imageCrop->used = true;			
			image.read(imageCrop->fileName);
			image.crop(Geometry(imageCrop->cropGeometry));
			image.scale(Geometry(tileWidth, 0));
			loaded.push_back(image);
		} catch (...) {
			cerr << "Error reading loading or processing image \"" << imageCrop->fileName << "\"" << endl;
		}
		gridIndex++;
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


