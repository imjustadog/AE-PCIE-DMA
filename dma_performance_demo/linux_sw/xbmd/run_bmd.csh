make

rm -rf /dev/xbmd
mknod /dev/xbmd c 241 1
chown root /dev/xbmd
chmod 0644 /dev/xbmd
ls -al /dev/xbmd

./load_driver

