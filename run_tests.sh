#!/bin/sh

./install.sh
./test/driver/htr.py test ./test/ ./_test/ _install/helium/helium -t parse-fail run-fail run-pass
