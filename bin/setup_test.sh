#!/bin/bash
SITE=
PROJECT=

sudo rm -r $SITE/test
mkdir $SITE/test/
mkdir $SITE/test/api
mkdir $SITE/test/api/config
mkdir $SITE/test/api/scripts
mkdir $SITE/test/api/video
mkdir $SITE/test/api/shared
mkdir $SITE/uploads
sudo chgrp -R _www $SITE/uploads
sudo chmod u=rwx,g=rwx $SITE/uploads
sudo chmod g=rwx $SITE/test/api/scripts

cp $PROJECT/src/frontend/*.html $SITE/test
cp $PROJECT/src/frontend/*.js $SITE/test
cp $PROJECT/src/frontend/*.css $SITE/test

cp $PROJECT/src/api/config/*.php $SITE/test/api/config
cp $PROJECT/src/api/video/*.php $SITE/test/api/video
cp $PROJECT/src/api/scripts/*.php $SITE/test/api/scripts
cp $PROJECT/src/api/shared/*.php $SITE/test/api/shared

cp $PROJECT/bin/vidproc.out $SITE/test/api/scripts
cp $PROJECT/data/shape_predictor_68_face_landmarks.dat $SITE/test/api/scripts

mv $SITE/test/index.html $SITE/test/login.html
cp $SITE/database.ini $SITE/test/api/config
mv $SITE/test/api/config/config.php $SITE
