#!/bin/bash
# Post merge script for repository name words replacements
# Example (orion into btcu repo):
# -call git add remote orion https://github.com/bitcoin-ultimatum/orion.git
# -call ./contrib/scripts/pre-merge.sh
# -call git merge orion/master
# -call ./contrib/scripts/post-merge.sh btcu orion

sed -i '' 's/'"${1}"'/'"${2}"'/' README.md
sed -i '' 's/'"${1}"'/'"${2}"'/' install_ubuntu.sh
sed -i '' 's/'"${1}"'/'"${2}"'/' CONTRIBUTING.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/release-process.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/release-notes.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/dependencies.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/build-windows.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/build-unix.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/build-osx.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/gitian-building/gitian-setup-windows.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/gitian-building/gitian-setup-ubuntu.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/gitian-building/gitian-setup-mac.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/gitian-building/gitian-setup-fedora.md
sed -i '' 's/'"${1}"'/'"${2}"'/' doc/gitian-building/gitian-building-mac-os-sdk.md
sed -i '' 's/'"${1}"'/'"${2}"'/' contrib/gitian-build.py
sed -i '' 's/'"${1}"'/'"${2}"'/' contrib/rpm/btcu.spec
sed -i '' 's/'"${1}"'/'"${2}"'/' contrib/gitian-descriptors/gitian-win.yml
sed -i '' 's/'"${1}"'/'"${2}"'/' contrib/gitian-descriptors/gitian-osx.yml
sed -i '' 's/'"${1}"'/'"${2}"'/' contrib/gitian-descriptors/gitian-linux.yml
sed -i '' 's/'"${1}"'/'"${2}"'/' contrib/debian/control
sed -i '' 's/'"${1}"'/'"${2}"'/' build-aux/snap/snapcraft.yaml

cp tmpmerge/chainparams.cpp src/chainparams.cpp
cp tmpmerge/blockrewards.h src/blockrewards.h
rm -rf tmpmerge
