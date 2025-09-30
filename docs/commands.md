# Sistema de Comandos - Documentação

## Arquivo Principal
`src/commands/commands.c` - Implementação dos comandos do terminal

## Arquitetura

### Processamento
- Entrada processada em `process_command()`
- Comparação de strings com `strcmp()`
- Chamada de função específica para cada comando

### Comandos Implementados

#### help
- **Função**: `cmd_help()`
- **Descrição**: Exibe lista de comandos disponíveis
- **Uso**: `help`

#### clear
- **Função**: `cmd_clear()`
- **Descrição**: Limpa completamente a tela
- **Implementação**: Chama `terminal_init()`

#### about
- **Função**: `cmd_about()`
- **Descrição**: Informações sobre o NanoOS
- **Conteúdo**: Versão, recursos implementados

#### uptime
- **Função**: `cmd_uptime()`
- **Descrição**: Tempo de execução do sistema
- **Dados**: Ticks do timer e conversão para segundos
- **Dependências**: `timer_ticks`, `uint_to_str()`

#### license
- **Função**: `cmd_license()`
- **Descrição**: Informações de licença e desenvolvedores
- **Conteúdo**: Licença MIT, links dos desenvolvedores

#### echo
- **Função**: `cmd_echo()`
- **Descrição**: Repete texto fornecido
- **Uso**: `echo [texto]`
- **Implementação**: Extração de substring após "echo "

### Tratamento de Erros
- Comandos não reconhecidos exibem mensagem de erro
- Sugestão para usar `help`
- Validação de entrada nula

### Dependências Externas
- `terminal_print()` - Output para terminal
- `strlen()`, `strcmp()` - Manipulação de strings
- `timer_ticks` - Variável global do timer
- `uint_to_str()` - Conversão numérica