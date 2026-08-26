# sonoff
# Avahi?
https://www.sigmdel.ca/michel/ha/sonoff/sonoff_mini_en.html
avahi-browse -t _ewelink._tcp --resolve
# API
curl -v http://192.168.1.115:8081/zeroconf/info -H "Content-Type: application/json" -d '{"deviceid":"1000c8797e","data":{}}'


# LITE!
curl -F "firmware=@.pio/build/dev/firmware.bin" "http://192.168.1.225/api/ota_flash?tamanho=$(stat -c%s .pio/build/dev/firmware.bin)"
curl -F "firmware=@.pio/build/sonoff-mini/firmware.bin" "http://192.168.1.94/api/ota_flash?tamanho=$(stat -c%s .pio/build/sonoff-mini/firmware.bin)"