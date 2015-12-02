#!/bin/bash

if [ ! -d /$HOME/.pip/ ]; then
  mkdir /$HOME/.pip/
fi
cat > /$HOME/.pip/pip.conf << EOF
[global]
index-url = http://10.160.60.9/pip/simple/
trusted-host=10.160.60.9
EOF

cat > /$HOME/.pydistutils.cfg << EOF
[easy_install]
index_url = http://10.160.60.9/pip/simple/
EOF
