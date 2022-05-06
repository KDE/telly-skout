# telly-skout
A convergent TV guide based on Kirigami.

## Build
### Dependencies
#### Manjaro
```
sudo pacman -Syu cmake extra-cmake-modules gcc git make
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

## Develop
### Fetcher
A `Fetcher` retrieves the TV guide data from some source and stores it in the database. Currently, only the `TvSpielfilmFetcher` is used. If more `Fetchers` are added, functionality to select a `Fetcher` shall be provided.

All `Fetchers` must derive from `FetcherImpl`, write the retrieved data to the `Database` and emit the signals defined in `FetcherImpl`. `Fetchers`, which retrieve data from the network, may derive from `NetworkFetcher`.
