#!/bin/sh

./install.sh
./test/driver/htr.py test ./test/ ./_test/ _install/helium/helium -t run-fail
