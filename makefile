crop-collage: main.cpp
	g++ main.cpp `GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs` -o crop-collage
