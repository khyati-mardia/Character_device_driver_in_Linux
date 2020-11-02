make clean
make all
sudo insmod rbt530_driver.ko
sudo insmod RBprobe.ko
sudo chmod 777 /dev/rbt530_dev1
sudo chmod 777 /dev/rbt530_dev2
sudo chmod 777 /dev/RBprobe

