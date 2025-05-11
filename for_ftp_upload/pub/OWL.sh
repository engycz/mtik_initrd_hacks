ROOT=${1}/OWL
cd $ROOT

[ -f $ROOT/bin/busybox -a ! -f $ROOT/bin/sh  ] && {
  echo "Initializing busybox"
  $ROOT/bin/busybox chroot /system /ram/OWL/bin/busybox --install -s /ram/OWL/bin 2>> $ROOT/lll
}

export PATH="/ram/OWL/bin:$PATH"

echo "Launching telnetd"
$ROOT/bin/busybox chroot /system telnetd -p 22111 -F -l sh

echo "Konec"
