SHELL := /bin/bash
CXX=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
CXXMM=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang

OUTPUTPATH=build/
INTERMPATH=build/interm/

PROGRAM=sokoban
BUNDLE=$(OUTPUTPATH)/$(PROGRAM).app

OBJECTS=main.o g_Game.o coord.o game.o graphics.o board.o entity.o ui.o generator.o solver.o network.o
OBJECTSMM=fileio.o

LDFLAGS=-arch x86_64 -lstdc++ -F/Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_net -framework Cocoa -framework OpenGL -framework GLUT -framework IOKit -L/usr/local/lib -lcurl -L/Users/jcampbell/Projects/Build/lib_x64/ -lboost_filesystem -lboost_system -lboost_random -headerpad_max_install_names
CPPFLAGS=-arch x86_64 -I/Library/Frameworks/SDL2.framework/Headers -g -std=c++11 -Wno-write-strings

all: $(BUNDLE) $(BUNDLE)/Contents/MacOS/$(PROGRAM)

clean:
	rm -f $(OBJECTS)
	rm -fr $(BUNDLE)

#  This builds the bundle's directory structure.
$(BUNDLE):
	@echo "==== Building bundle directory structure ===="
	mkdir -p $(OUTPUTPATH)
	mkdir -p $(BUNDLE)/Contents
	mkdir -p $(BUNDLE)/Contents/MacOS
	mkdir -p $(BUNDLE)/Contents/Resources
	mkdir -p $(BUNDLE)/Contents/Frameworks
	cp -RH /Library/Frameworks/SDL2.framework $(BUNDLE)/Contents/Frameworks/
	cp -RH /Library/Frameworks/SDL2_image.framework $(BUNDLE)/Contents/Frameworks/
	cp -RH /Library/Frameworks/SDL2_net.framework $(BUNDLE)/Contents/Frameworks/
	cp /Users/jcampbell/Projects/Build/lib_x64/libboost_filesystem.dylib $(BUNDLE)/Contents/Frameworks/
	cp /Users/jcampbell/Projects/Build/lib_x64/libboost_system.dylib $(BUNDLE)/Contents/Frameworks/
	cp /Users/jcampbell/Projects/Build/lib_x64/libboost_random.dylib $(BUNDLE)/Contents/Frameworks/
	rm -fr $(BUNDLE)/Contents/Frameworks/SDL2.framework/Versions/A/Headers/
	rm -fr $(BUNDLE)/Contents/Frameworks/SDL2.framework/Headers
	cp -RH images32 $(BUNDLE)/Contents/Resources

#  This builds the executable right inside the bundle.
$(BUNDLE)/Contents/MacOS/$(PROGRAM): $(OBJECTS) $(OBJECTSMM)
	@echo "==== Building executable ===="
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++ -o $(BUNDLE)/Contents/MacOS/$(PROGRAM) $(OBJECTS) $(OBJECTSMM) $(LDFLAGS)
	install_name_tool -change libboost_filesystem.dylib @executable_path/../Frameworks/libboost_filesystem.dylib $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change libboost_random.dylib @executable_path/../Frameworks/libboost_random.dylib $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change libboost_system.dylib @executable_path/../Frameworks/libboost_system.dylib $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @executable_path/../Frameworks/SDL2.framework/Versions/A/SDL2 $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change @rpath/SDL2_image.framework/Versions/A/SDL2_image @executable_path/../Frameworks/SDL2_image.framework/Versions/A/SDL2_image $(BUNDLE)/Contents/MacOS/$(PROGRAM)
	install_name_tool -change libboost_system.dylib @executable_path/../Frameworks/libboost_system.dylib $(BUNDLE)/Contents/Frameworks/libboost_filesystem.dylib
	install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @executable_path/../Frameworks/SDL2.framework/Versions/A/SDL2 $(BUNDLE)/Contents/Frameworks/SDL2_image.framework/Versions/A/SDL2_image

$(OBJECTS): %.o: %.cpp
	$(CXX) -x c++ -c -o $@ $(CPPFLAGS) $<
	
$(OBJECTSMM): %.o: %.mm
	@echo "$(CXX) -c -o $@ $(CPPFLAGS) $<"
	$(CXXMM) -x objective-c++ -c -o $@ $(CPPFLAGS) $<
	