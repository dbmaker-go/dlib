# C wrapper for face recognition

### build libdface.so release
```
g++ -std=c++11 -O3 dface.cpp ../dlib/all/source.cpp -I. -I..  -shared -fPIC -o libdface.so -ljpeg -lpthread -lpng -DDLIB_PNG_SUPPORT=1 -DDLIB_JPEG_SUPPORT=1 -DDLIB_NO_GUI_SUPPORT=1
```

### build libdface.so debug
```
g++ -std=c++11 -g dface.cpp ../dlib/all/source.cpp -I. -I..  -shared -fPIC -o libdfaced.so -ljpeg -lpthread -lpng -DDLIB_PNG_SUPPORT=1 -DDLIB_JPEG_SUPPORT=1 -DDLIB_NO_GUI_SUPPORT=1
```

### build c program
```
cc test.c -ldface -L. -o test
```

### program need modle files, download and unzip:
```
http://dlib.net/files/shape_predictor_5_face_landmarks.dat.bz2
http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2
http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2
```

### use cmake to build on both linux and windows
```
cd dface
mkdir build
cd build

// for linux or win32
cmake ..
cmake --build . --config Release

// for win64
cmake -G "Visual Studio 14 2015 Win64" -T host=x64 ..
cmake --build . --config Release
```

