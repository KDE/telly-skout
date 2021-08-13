# telly-scout
A convergent EPG based on Kirigami.

## Build
### Dependencies
#### Manjaro
```
sudo pacman -Syu git cmake gcc extra-cmake-modules
```

### Clone and Build
```
git clone https://github.com/plata/telly-scout.git
cd telly-scout
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Install
```
sudo make install
```

## Run
```
telly-scout
```
