DIR="$(dirname "$(realpath "$0")")"
termux-usb -r -e "$DIR/read_mouse_raw" /dev/bus/usb/001/002
