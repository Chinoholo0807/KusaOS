gunzip 80m.img.gz
make image
if [ "`echo $?`" != "0" ]
then
	echo make image error!!!!!!
	exit 0
fi

cd command
make install

if [ "`echo $?`" != "0" ]
then
        echo make install error!!!!!!!
        exit 0
fi


cd ..
make image

if [ "`echo $?`" != "0" ]
then
        echo make image again error!!!!!!!
        exit 0
fi


bochs

