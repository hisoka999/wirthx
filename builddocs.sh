#!/bin/bash

pasdoc --include rtl/ \
      --output=docs \
       --visible-members=protected+ \
       `find rtl/ -iname '*.pas'` \
        --abbreviations=AUTHORS \
        '--marker=**' \
        --marker-optional \
        "--title=WirthX RTL Docs"