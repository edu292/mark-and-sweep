# 🧹 Garbage Collector Mark and Sweep em C

Este projeto implementa, do zero, um **Garbage Collector (GC) Mark and Sweep**, inspirado nos mecanismos de gerenciamento automático de memória presentes em linguagens como Java, Python e Go. A proposta é **explorar os bastidores dessas linguagens**, entendendo como objetos são alocados, rastreados e eventualmente descartados da memória.

---

## 🚀 O que o projeto faz

- 📦 Aloca dinamicamente objetos como inteiros, strings, arrays e floats
- 🔁 Gerencia escopos e ciclos de vida com frames
- 🧹 Executa a coleta de lixo com algoritmo Mark and Sweep completo
- 🔗 Lida com **ciclos de referência** e grafos de objetos interdependentes
- ✅ Inclui uma suíte de testes automatizados utilizado **MUnit** para abrangir diversos cenários de uso

---

## 🧠 Por que esse projeto importa?

A maior parte das linguagens modernas esconde o gerenciamento de memória do desenvolvedor. Mas entender como isso funciona por baixo dos panos é um diferencial enorme ao escrever código com perfromance, robustez e escalabilidade. Este projeto permite:

- 📚 **Desmistificar** o funcionamento de GCs modernos
- 🔍 Simular como linguagens determinam o que ainda está vivo ou pode ser coletado
- 🧠 Ver, na prática, como um runtime rastreia acessos a objetos dinamicamente
- 🔧 Criar as bases para escrever a própria linguagem, VM ou interpretador

---

## 🧠 Como Funciona o Mark and Sweep
O coletor de lixo Mark and Sweep opera em duas fases principais:

### 🔹 1. Mark (Marcar)
O processo começa pelos frames e percorre todos os objetos referênciados por eles, marcando-os. Esse rastreamento segue referências de forma recursiva, o que significa que estruturas compostas, como arrays, também são exploradas:
- Um array marcado leva ao rastreamento de todos os objetos internos.
- No caso de referênciação cíclica, os objetos não serão marcados pois não são acessíveis a partir do frame
- O processo é similar a uma busca em profundidade (DFS) sobre o grafo de objetos.

### 🔹 2. Sweep (Varrer)
Na fase seguinte, o coletor varre todos os objetos alocados:
- Se estiverem marcados, são "desmarcados" e preservados.
- Se não estiverem marcados, são considerados lixo e liberados da memória.

### 🕹️ Exemplo de uso

```c
vm_t *vm = vm_new();
frame_t *frame = vm_new_frame(vm);

// Criação de objetos
object_t *n = new_int(vm, 100);
object_t *msg = new_string(vm, "Olá mundo");

// Referência os objetos no frame atual
frame_reference_object(frame, n);
frame_reference_object(frame, msg);

// Executa coleta: objetos referenciados são preservados
vm_collect_garbage(vm);

// Criando objetos sem referência ativa
new_string(vm, "Isso será coletado");

// Coleta remove automaticamente da heap
vm_collect_garbage(vm);
```

---

## 🧪 Testes e demonstrações   
Os testes cobrem: 
- Alocação e coleta básica
- Objetos encadeados (referência entre objetos)
- Arrays de objetos
- Referência cíclica (objeto → objeto → volta ao primeiro)
- Cenários com múltiplos frames e escopos aninhados

---

## 📂 Estrutura do Projeto

| Arquivo        | Descrição                                                              |
|----------------|------------------------------------------------------------------------|
| object.c/.h    | Define os tipos de objetos (int, float, string, array)                 |
| vm.c/.h        | Implementação da máquina virtual e do coletor mark and sweep           |
| stack.c/.h     | Implementação da estrutura de stack para uso no projeto                |
| tests.c        | Casos de teste automatizados para validar o comportamento do coletor   |
| CmakeLists.txt | Configurações de build                                                 |
| README.md      | Documentação do projeto                                                |   

---

## ⚙️ Como rodar

Este projeto usa **CMake** e inclui o **Munit** (framework de testes) como submódulo para validação do comportamento do coletor.

### 📥 Clonando o repositório

Clone com submódulos:

```bash
git clone --recursive https://github.com/edu292/mark_and_sweep.git
cd mas_gc
```

Já clonou sem `--recursive`?

```bash
git submodule update --init
```

### 🛠️ Compilando com CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 🧪 Executando os testes

```bash
./mas_gc
```
