== Solução EKAZA3 sem boot ==
Resumo — BK7238 / EKAZA3 funcionando com LibreTiny
Conseguimos executar o firmware mínimo no BK7238 (Tuya T1-U), mantendo o bootloader original!
O problema não estava na gravação nem, aparentemente, na criptografia. Identificamos problemas na inicialização do LibreTiny 1.13.0 e aplicamos três alterações, sendo a última a que fez a aplicação funcionar.

1. Ambiente
   Componente Versão
   Hardware EKAZA3 — Tuya T1-U
   SoC BK7238
   Bootloader original BK7238 2.0.0 (T1)
   Flash 2 MiB
   LibreTiny 1.13.0
   Beken BDK 3.0.78
   Framework Arduino
   PlatformIO board t1-u
   O problema original era que, após a mensagem jump to:0x10000, a aplicação ficava completamente silenciosa e o dispositivo reiniciava aproximadamente 30 segundos depois.
2. Investigação realizada
   Antes de alterar o LibreTiny, comprovamos:

- O bootloader estava íntegro: primeiros 0x11000 bytes idênticos ao original.
- Os 2.048 blocos de CRC16 examinados estavam corretos.
- A imagem lógica completa, com 196.772 bytes, correspondia exatamente ao ELF (0 diferenças).
- O firmware original e o firmware LibreTiny continham instruções ARM legíveis e vetores válidos em 0x10000.
- O startup ARM, a inicialização das stacks e a transição para Thumb não apresentavam inconsistências evidentes.
  Conclusão: a hipótese de criptografia incompatível ficou muito enfraquecida. Não foi necessário modificar eFuse, coeficientes criptográficos nem bootloader.

3. As três alterações que fizemos
   Correção 1 — Forçar a linkagem de lt_init_family()
   Descobrimos que lt_main() deveria chamar lt_init_family(), mas a função estava declarada como weak e não era incluída no ELF final.
   Antes:
   34ac2: b.n 34ac6
   34ac6: b.n 34aca

Depois, adicionamos ao platformio.ini:
build_flags =
-Wl,--undefined=lt_init_family

O novo ELF passou a conter:
34ad6: bl 1582c <lt_init_family>

Essa função é responsável pela inicialização básica do hardware da família Beken, incluindo a UART.
Correção 2 — Corrigir o índice da UART
Arquivo:
~/.platformio/platforms/libretiny/cores/beken-72xx/base/api/lt_init.c
Encontramos um erro na seleção das portas UART.
O código utilizava:
uart_print_port = LT_UART_DEFAULT_PORT - 1;

Mas comparava o índice interno com os números físicos das portas.
Aplicamos este patch:
void lt_init_family() {
uart_print_port = LT_UART_DEFAULT_PORT - 1;

- if (uart_print_port == 1)

* if (uart_print_port == 0)
  uart1_init();

- else if (uart_print_port == 2)

* else if (uart_print_port == 1)
  uart2_init();
  }

Após recompilar, confirmamos pelo disassembly:
15830: movs r2, #1
15832: str r2, [r3, #0]
15834: bl 19d78 <uart2_init>

A função passou a inicializar corretamente a UART2.
Entretanto, essas duas correções ainda não fizeram a aplicação iniciar.
Correção 3 — Inicialização antecipada da UART em entry_main()
Esta foi a alteração decisiva no teste.
Arquivo:
~/.platformio/platforms/libretiny/cores/beken-72xx/base/fixups/arch_main.c
Descobrimos que o BDK original executava uart_fast_init() logo na entrada da aplicação, mas o startup adaptado pelo LibreTiny omitia essa chamada.
Adicionamos:
#include "start_type_pub.h"
+#include "uart_pub.h"

- +extern void uart_fast_init(void);

E no início da função:
void entry_main(void) {

- uart_print_port = UART2_PORT;
- uart_fast_init();
- bk_send_string(UART2_PORT, "DEBUG A\n");
-     // compatibility with BK7231S_1.0.5
  #if CFG_SUPPORT_BOOTLOADER
  entry_set_world_flag();
  #endif

Também inserimos marcadores DEBUG B, DEBUG C, DEBUG D, DEBUG E e DEBUG F entre as principais funções de inicialização.
A função uart_fast_init() executa três operações importantes:

1. Habilita o clock da UART.
2. Configura os GPIOs para a função UART.
3. Inicializa os registradores do periférico com uart_hw_init().
   Após essa alteração, o firmware mínimo finalmente funcionou!
4. O que provavelmente estava acontecendo
   Nossa hipótese técnica mais forte é que a inicialização da UART estava incompleta ou ocorrendo tarde demais.
   O lt_log() envia os caracteres usando:
   bk_send_byte((port & 0xFF) - 1, c);

E o disassembly de bk_send_byte() revelou uma espera bloqueante:
while (!(REG_READ(0x00802208) & 0x00100000)) {
// Espera a UART2 ficar pronta
}

Essa espera não possui timeout.
Uma possibilidade é que o firmware ficasse preso nesse laço quando tentava produzir seu primeiro log. O watchdog ou outro mecanismo de reset então reiniciaria o chip.
Contudo, não demonstramos que a CPU efetivamente parava nesse endereço. Também não isolamos as três mudanças para determinar quais são estritamente necessárias. O que está confirmado é que a combinação final funciona e que a última alteração mudou o resultado. 5. Estado final
Item Resultado
Bootloader Tuya original Preservado
Gravação de firmware Correta
Integridade ELF/flash Confirmada
Criptografia incompatível Sem evidência
lt_init_family() Linkada
Seleção UART1/UART2 Corrigida
uart_fast_init() antecipada Adicionada
Firmware mínimo Funcionando
Não alteramos a eFuse, o bootloader, a versão do BDK ou a implementação de lt_init_variant(). 6. Recomendações para consolidar a correção
Antes de transportar as alterações para o eTomada Lite completo, recomendo duas providências.
Primeiro: preservar os patches dos arquivos lt_init.c e arch_main.c, porque uma atualização do PlatformIO ou LibreTiny pode sobrescrevê-los.
Segundo: fazer um teste isolado para confirmar qual alteração é indispensável. Eu começaria removendo apenas os marcadores DEBUG, mantendo uart_fast_init() e as duas correções anteriores. Depois, se continuar funcionando, poderemos avaliar se uart_fast_init() por si só já resolveria o problema.

Conclusão
O problema do EKAZA3 não parece ser criptografia de flash, como suspeitávamos inicialmente, mas sim uma incompatibilidade entre o fluxo de inicialização do LibreTiny 1.13.0 e o BK7238 com bootloader Tuya T1 2.0.0.
A mudança que finalmente permitiu iniciar o firmware foi restaurar a inicialização antecipada da UART, utilizando uart_fast_init() como faz o BDK original.
Agora temos uma base funcional para prosseguir com o eTomada Lite no BK7238, preservando o bootloader original do dispositivo.
