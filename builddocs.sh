#!/bin/bash

pasdoc --include rtl/ \
      --output=docs \
      --format simplexml\
      --define UNIX\
       --visible-members=protected+ \
       `find rtl/ -iname '*.pas'` \
        --abbreviations=AUTHORS \
        '--marker=**' \
        --marker-optional \
        "--title=WirthX RTL Docs"


for xmlfile in `find docs/ -iname '*.xml'`;
do
  name=${xmlfile%.xml}
  xsltproc -o $name.markdown docs/markdown.xsl $xmlfile
done