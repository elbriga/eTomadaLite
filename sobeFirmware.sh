#!/bin/bash

FW=$1
HOST=$2

uso() {
	echo "Uso: ./sobeFirmware.sh ARQUIVO.bin HOST"
	echo
	exit 1
}

[[ -z "$FW" || -z "$HOST" ]] && uso
[[ ! -f "$FW" ]] && uso

TAM=$(stat -c%s $FW)
SHA=$(sha256sum $FW | awk '{print $1}')

URL="http://$HOST/api/ota?tamanho=$TAM&sha=$SHA"

# Firmware para o LN882 é UF2 > enviar esse header a mais
[[ "$FW" == *.uf2 ]] && echo ">> Modo LN882H"
[[ "$FW" == *.uf2 ]] && MODOUF2=";type=application/octet-stream"

CMD="curl -X POST -F \"firmware=@$FW$MODOUF2\" '$URL'"

echo "============================"
echo "Subindo FW: $FW"
echo "URL: $URL"
echo "============================"
echo $CMD
echo

curl -X POST -F "firmware=@$FW$MODOUF2" "$URL"
