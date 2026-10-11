# sonoff
# Avahi?
https://www.sigmdel.ca/michel/ha/sonoff/sonoff_mini_en.html
avahi-browse -t _etomada._tcp --resolve
# API
curl -v http://192.168.1.115:8081/zeroconf/info -H "Content-Type: application/json" -d '{"deviceid":"1000c8797e","data":{}}'


# LITE!
curl -F "firmware=@.pio/build/dev/firmware.bin" "http://192.168.1.225/api/ota_flash?tamanho=$(stat -c%s .pio/build/dev/firmware.bin)"
curl -F "firmware=@.pio/build/sonoff-mini/firmware.bin" "http://192.168.1.94/api/ota_flash?tamanho=$(stat -c%s .pio/build/sonoff-mini/firmware.bin)"


# COZY
IP=10.10.10.242; \
FW=.pio/build/cozylife/firmware.uf2; \
SIZE=$(stat -c%s "$FW"); \
echo "UPLOAD de $FW [$SIZE bytes] para $IP"; \
curl -v --fail-with-body -F "firmware=@$FW;type=application/octet-stream" "http://$IP/api/ota?tamanho=$SIZE"



# 6 Teclas
ltchiptool flash write \
    -d /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5069RR4-if00-port0 \
    .pio/build/ekaza-6teclas/firmware.uf2
# Depois, gravar o bloco de 4K original com os dados de WiFi (mac, ...)
bk7231tools write_flash \
  -d /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5069RR4-if00-port0 \
  -s 0x1D0000 \
  files/EKAZA-6teclas-wifi-calibration-0x1D0000.bin

# Instalar o bk7231tools
sudo apt install python3-venv
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install bk7231tools

# restaurar firmware original
bk7231tools write_flash -d /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5069RR4-if00-port0 -s 0 -S 0 -l 0x200000 --bootloader ekaza-original.bin

# Ouvir a porta serial
stty -F /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5069RR4-if00-port0 115200 raw -echo -echoe -echok
cat /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5069RR4-if00-port0
