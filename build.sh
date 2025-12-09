kn# Compilar rapidamente os 3 executaveis

unlink cli_*
unlink canal

gcc cliente.c -o cliente
gcc controlador.c -o controlador
# gcc veiculo.c -o veiculo