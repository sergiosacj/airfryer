# 1. Controle de temperatura

temperatura é consultada, o controle está no sistema do interno.
desligar significa resfriamento até temperatura ambiente.

## 1.1 Ler temperatura interna atual
  * ler da uart
## 1.2 Ler temperatura referencia atual
  * ler da uart
## 1.3 PID
  * algoritmo que gera o sinal de controle
## 1.4 sinal de controle (-100% até 100%)
  * negativo -> ventoinha (PWM GPIO)
  * positivo -> resistência (PWM GPIO)
  * enviar para a uart o sinal

# 2. Controle de tempo

tempo mínimo: 1 minuto
tempo máximo: X minuto
controle: botões de usuário
tempo mínimo de requisições na uart: 50ms a 100ms
tempo de pré aquecimento não conta

# 3. Display LCD (I2C)

temperatura interna
temperatura referencia
tempo
menu (manual | alimento)

# 4. Botões

Ligar/Desligar o Forno -> funcionamento da airfryer
Inicia aquecimento/Cancela processo -> funcionamento do sistema interno da airfryer
Tempo +|- -> chama a função pid\_atualiza\_referencia
Menu -> altera o alimento (altera valores pré definidos)

# Máquina de estados

Consultar UART - 500ms
Atualizar display LCD - 1s
Log CSV - 1s
