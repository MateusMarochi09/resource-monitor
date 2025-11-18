# Validação de Memory Leaks com Valgrind

## 1. Execução do Programa

Para validar se o sistema possuía *memory leaks*, executei o monitoramento usando o Valgrind com as seguintes opções:

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./monitor

Ao iniciar, o programa exibiu o menu principal normalmente:

====================================
      SISTEMA DE MONITORAMENTO
====================================
1. Recource Profile
2. Namespace Analyser
3. Control Group Manager
0. Sair
------------------------------------
Escolha uma opção:

Durante os testes, naveguei pelo menu e interrompi manualmente a execução usando Ctrl + C (SIGINT) para forçar o relatório completo.

Output Relevante do Valgrind

A seguir, o trecho essencial que demonstra o estado da memória:

==62082== HEAP SUMMARY:
==62082==     in use at exit: 2,048 bytes in 2 blocks
==62082==   total heap usage: 68 allocs, 66 frees, 121,696 bytes allocated

==62082== LEAK SUMMARY:
==62082==    definitely lost: 0 bytes in 0 blocks
==62082==    indirectly lost: 0 bytes in 0 blocks
==62082==      possibly lost: 0 bytes in 0 blocks
==62082==    still reachable: 2,048 bytes in 2 blocks
==62082==         suppressed: 0 bytes in 0 blocks

==62082== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)


Análise dos Resultados

Nenhum memory leak foi detectado:

definitely lost: 0 bytes

indirectly lost: 0 bytes

possibly lost: 0 bytes

Os 2.048 bytes “still reachable” pertencem a buffers internos da biblioteca padrão (glibc), não caracterizando vazamento.

O programa finalizou sem erros relacionados à memória:

ERROR SUMMARY: 0 erros


Conclusão

A aplicação foi validada com o Valgrind e não apresenta memory leaks.
Todo o gerenciamento de memória implementado no projeto está correto e atende ao requisito "Sem memory leaks".
