
# Protocolo de Comunicação - Chat IRC Simples

Este documento define o protocolo de comunicação entre o **cliente** e o **servidor**, incluindo os comandos permitidos e suas respectivas funções.

---

## 📥 Comandos do Cliente

| Comando       | Descrição |
|---------------|-----------|
| `<NOME>`      | Envia o nome do usuário ao servidor para identificação. Exemplo: `Ana\n` |
| `<ALL>`       | Envia uma mensagem geral para todos os usuários conectados. Exemplo: `Olá pessoal!\n` |
| `<SAIR>`      | Encerra a conexão com o servidor de forma voluntária. |

---

## 📤 Respostas do Servidor

| Comando/Resposta | Descrição |
|------------------|-----------|
| `<ACK>`          | Nome aceito. O usuário está autorizado a interagir no chat. |
| `<NACK>`         | Nome recusado. Provavelmente já está em uso ou é inválido. |
| `timeout`        | O servidor desconecta automaticamente o cliente após **60 segundos de inatividade**. |

---

## 🔁 Fluxo de Comunicação

1. Cliente se conecta ao servidor.
2. Cliente envia `<NOME>` com o nome do usuário.
3. Servidor responde com `<ACK>` ou `<NACK>`.
4. Após `<ACK>`, o cliente pode:
   - Enviar mensagens com `<ALL>`.
   - Encerrar a sessão com `<SAIR>`.
5. Se o cliente ficar inativo por 60 segundos, será desconectado com `timeout`.

---

## 📝 Exemplo de Sessão

```plaintext
[Cliente] → Joao
[Servidor] ← <ACK>

[Cliente] → ALL Olá galera!
[Servidor] ← (mensagens encaminhadas aos demais clientes)

[Cliente] → SAIR
[Servidor] ← (encerra conexão)
```

---

## ⚠️ Observações

- O nome do usuário deve ser enviado como **primeira mensagem**.
- Não é permitido dois usuários com o mesmo nome.
- As mensagens são trocadas em **formato texto**.
