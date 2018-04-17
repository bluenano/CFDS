#!/bin/bash
SITE=
PROJECT=

sudo rm -r $SITE/test/api
mkdir $SITE/test/
mkdir $SITE/test/api
mkdir $SITE/test/api/config
mkdir $SITE/test/api/scripts
mkdir $SITE/test/api/video
mkdir $SITE/test/api/shared
mkdir $SITE/uploads
sudo chgrp -R _www $SITE/uploads
sudo chmod u=rwx,g=rwx $SITE/uploads

# clear the directories, mkdirs will fail if
# initial structure already exists
rm -r $SITE/test/*.html
rm -r $SITE/test/*.php
rm -r $SITE/test/*.js
rm -r $SITE/test/*.css

cp $PROJECT/Front-End/*.html $SITE/test
cp $PROJECT/Front-End/*.js $SITE/test
cp $PROJECT/Front-End/*.css $SITE/test

cp $PROJECT/api/config/*.php $SITE/test/api/config
#cp ../backend/api/config/config.php ~/Sites
cp $PROJECT/api/video/*.php $SITE/test/api/video
cp $PROJECT/api/scripts/*.php $SITE/test/api/scripts
cp $PROJECT/api/shared/*.php $SITE/test/api/shared
mv $SITE/test/index.html $SITE/test/login.html
cp $SITE/database.ini $SITE/test/api/config
mv $SITE/test/api/config/config.php $SITE

chmod -R g=rwx $SITE/test
