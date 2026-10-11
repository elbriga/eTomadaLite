#!/bin/bash

HOST=$1

uso() {
	echo "Uso: ./sobeFirmware.sh HOST"
	echo
	exit 1
}

[[ -z "$HOST" ]] && uso

FW=.pio/build/$HOST/firmware.uf2
[[ ! -f "$FW" ]] && FW=.pio/build/$HOST/firmware.bin

if [ ! -f "$FW" ]; then
	echo "Arquivo de firmware nao encontrado: $FW"
	echo "Compile o firmware antes de subir"
	exit 2
fi

TAM=$(stat -c%s "$FW")
SHA=$(sha256sum "$FW" | awk '{print $1}')

URL="http://$HOST/api/ota?tamanho=$TAM&sha=$SHA"

# LibreTiny (LN882H/BK7231N) utiliza firmware UF2
[[ "$FW" == *.uf2 ]] && echo ">> Modo LibreTiny UF2"
[[ "$FW" == *.uf2 ]] && MODOUF2=";type=application/octet-stream"

CMD="curl --fail-with-body -X POST -F \"firmware=@$FW$MODOUF2\" '$URL'"

echo "============================"
echo "Subindo FW: $FW"
echo "URL: $URL"
echo "============================"
echo $CMD
echo

curl --fail-with-body -X POST -F "firmware=@$FW$MODOUF2" "$URL"
