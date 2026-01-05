# Impdancer 

S2p View and Impedance matching tools

## Windows quick start

Download https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip and place inside src/thirdparty named as raylib-5.5

```console
mingw32-make.exe
build/impedancer path_to_dir_with_s2p_files
```

## Linux quick start
```console
wget https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz
mkdir -p src/thirdparty/raylib-5.5-linux
tar -xf raylib-5.5_linux_amd64.tar.gz -C src/thirdparty/raylib-5.5-linux --strip-components=1
```

To build and run the program:

```console
make
LD_LIBRARY_PATH=./src/thirdparty/raylib-5.5-linux/lib/ ./build/impedancer path_to_dir_with_s2p_files
```
