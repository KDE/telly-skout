# telly-skout
A convergent TV guide based on Kirigami.

<a href='https://flathub.org/apps/details/org.kde.telly-skout'><img width='190px' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.png'/></a>

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

### Flatpak
```
flatpak install org.kde.Sdk/x86_64/5.15-21.08
flatpak install org.kde.Platform/x86_64/5.15-21.08
flatpak-builder --user --install --force-clean build org.kde.telly-skout.yml
flatpak run org.kde.telly-skout
```

## Run
```
telly-skout
```

## Develop
### Fetcher
A `Fetcher` retrieves the TV guide data from some source and stores it in the database. The used `Fetcher` can be changed in the settings.

All `Fetchers` must derive from `FetcherImpl` and trigger the callbacks. `Fetchers`, which retrieve data from the network, may derive from `NetworkFetcher`.

To use the new `Fetcher`, it must be added to `TellySkoutSettings.kcfg`, `SettingsPage.qml` and the `Fetcher` constructor in `fetcher.cpp`.
