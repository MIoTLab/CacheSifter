export CROSS_COMPILE=aarch64-linux-android-

#make ARCH=arm64 distclean
#rm -rf ../out

#mkdir ../out
#make ARCH=arm64 O=../out merge_hi3650_defconfig
#make ARCH=arm64 O=../out -j4
make ARCH=arm64 O=../out menuconfig
