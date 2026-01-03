DIR="$(dirname "$(realpath "$0")")"
termux-usb -r -e "$DIR/read_mouse" /dev/bus/usb/001/002
