# telly-skout
A convergent EPG based on Kirigami.

## Build
### Dependencies
#### Manjaro
```
sudo pacman -Syu git cmake gcc extra-cmake-modules
```

### Clone and Build
```
git clone https://github.com/plata/telly-skout.git
cd telly-skout
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Install
```
sudo make install
```

## Run
```
telly-skout
```
