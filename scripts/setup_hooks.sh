#!/bin/bash

set -eux 
git submodule init
git submodule update
cp .githooks/* .git/hooks/
chmod +x .git/hooks/*


#git config core.hooksPath .githooks